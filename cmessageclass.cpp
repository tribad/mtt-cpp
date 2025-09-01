//
// Copyright 2015 Hans-Juergen Lange <hjl@simulated-universe.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in the
// Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include <iomanip>
#include "helper.h"
#include "crc64.h"
#include "tAttributePack.h"
#include "mattribute.h"
#include "cattribute.h"
#include "massociationend.h"
#include "cassociationend.h"
#include "cclassbase.h"
#include "ccxxclass.h"
#include "cmessageclass.h"
#include "cstruct.h"
#include "mmodel.h"
#include "cmodel.h"

extern long simversion;

CMessageClass::CMessageClass()
{
    basename = name;
}

CMessageClass::CMessageClass(const std::string& aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {
    has_hdr = ".h";
    type = eElementType::SimMessageClass;
}


void CMessageClass::SetFromTags(const std::string& name, const std::string&value)
{
    if (name == "BaseName") {
        basename=value;
    } if (name== "Direction") {
        direction = value;
    }
}

std::string CMessageClass::FQN() const {
    return mTypeTree.getFQN();
}

#if 0
void CMessageClass::CollectNeededModelHeader(std::shared_ptr<MElement> e) {
    auto  c = std::dynamic_pointer_cast<MClass>(e);

    if (donelist.find(e->name)==donelist.end()) {
        donelist.insert(e->name);

        for (auto & i : c->Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*i);

            if (((a->Aggregation == aNone) || (a->Aggregation == aComposition)) && a->Classifier) {
                eElementType et=a->Classifier->type;

                if ((et != eElementType::SimObject)) {
                    CollectNeededModelHeader(a->Classifier);
                }
            }
        }
        for (auto & i : c->OtherEnd) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);

            if ((a->Aggregation != aShared) && a->Classifier && (a->isNavigable())) {
                eElementType et=a->Classifier->type;

                if ((et != eElementType::SimObject)) {
                    CollectNeededModelHeader(a->Classifier);
                }
            }
        }
        if (e.get() != this) {
            neededmodelheader.add(e);
        }
        auto ti=e->tags.find("ExtraInclude");

        if (ti != e->tags.end()) {
            optionalmodelheader.add(e);
        }
        //
        //  If we are on the way out the tree we may add the dependencies.
        if (e.get() == this) {
            for ( auto & i : Supplier) {
                if ((i.getElement()->type==eElementType::CxxClass) || (i.getElement()->type==eElementType::CClass) || (i.getElement()->type==eElementType::Struct) || 
                    (i.getElement()->type==eElementType::Enumeration) || (i.getElement()->type==eElementType::Union) ) {
                    CollectNeededModelHeader(i.getElement());
                }
            }
        }
    }
}
#endif

void CMessageClass::Prepare(void) {
    PrepareBase();
    //
    //  Take the class name if no base name is set.
    if (basename.empty()) {
        basename = name;
    }
    //
    //  Create the Basename and the lowercase version.
    lower_basename=helper::tolower(basename);
    upper_basename=helper::toupper(basename);
    msgtype = "MSG_TYPE_UNSPECIFIED";
    if (!direction.empty()) {
        msgtype = "MSG_TYPE_" + helper::toupper(direction);
    }
}

void CMessageClass::Dump(std::shared_ptr<MModel> model) {
    std::list< tAttributePack >           alist;
    std::list< tAttributePack >::iterator ilist;
    size_t                                typemax = 0u;
    size_t                                namemax = 0u;

    auto cmodel = std::dynamic_pointer_cast<CModel>(model);
    //
    //  Collect all attributges and assocication ends so that the following functions are getting all needed information.
    CollectAttributes();
    //
    //  Collect the header files that we need to compile that module.
    CollectNeededModelHeader(sharedthis<MElement>(), neededmodelheader);

    DumpBase(cmodel, name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, name);
    hdr << "#include <simobj.h>\n"
           "#include <helper.h>\n"
           "#include <msg.h>\n"
           ;

    donelist.clear();
    //
    //  Add hardcoded headers and the message header file itself.
    donelist.insert("simobj");
    donelist.insert("helper");
    donelist.insert(name + ".h");
    //
    //  This is for the system header files that come from any other objects.
    //  Can be grow significantly.
    for (auto const& i : optionalmodelheader) {
        std::string header = i->tags.find("ExtraInclude")->second;
        std::string extra;
        size_t      start  = 0;
        size_t      end    = 0;

        do {
            end=header.find_first_of(' ', start);
            extra=header.substr(start, end-start);
            /*
             * trim front
             */
            while (extra[0]==' ') extra.erase(0, 1);
            /*
             * if proto has a size after trimming we can add it to the
             * list of protos.
             */
            if (!extra.empty()) {
                if (donelist.find(extra) == donelist.end()) {
                    hdr << "#include <" << extra << ">\n";
                    donelist.insert(extra);
                }
            }
            /*
             * if we are not at end of search we skip a character.
             * At end we do nothing and let the loop condition stop
             * the loop.
             */
            if (end == std::string::npos) {
                start = end;
            } else {
                start = end + 1;
            }
        } while (end!=std::string::npos);
    }

    //
    //  Insert the needed headers here.
    //  Because we have the implementation of the converver methods in the header
    //  we need headers for included structs here as well.
    std::set<std::shared_ptr<MElement>> oplist;

    DumpNeededIncludes(hdr, shared_this(), donelist, oplist);
    Crc64 crc;

    hdr << "\n#define " << std::string("IDM_")+upper_basename << " 0x" << std::hex << std::setfill('0') << std::setw(16) << crc.calc(std::string("IDM_")+upper_basename) << "\n";
    hdr << "//\n";
    hdr << "//                   M e s s a g e c l a s s     d e c l a r a t i o n\n";
    hdr << "struct " << name << " : public tMsg {\n";
    hdr << "    " << name << "() : tMsg (" << std::string("IDM_")+upper_basename <<", " << msgtype << ") {}\n";
    hdr << "    " << name << "(const tCommTarget& aDst, const tCommTarget& aSrc) : tMsg (" << std::string("IDM_")+upper_basename << ", " << msgtype << ") {\n"
                             "        src = aSrc;\n"
                             "        dst = aDst;\n"
                             "    }\n";
    hdr << "    " << name << "(uint64_t aDst, uint64_t aSrc) : tMsg (" << std::string("IDM_")+upper_basename << ", " << msgtype << ") {\n"
                            "        src    = tReference(aSrc, nullptr);\n"
                            "        dst    = tReference(aDst, nullptr);\n"
                            "    }\n";
    hdr << "    //\n"
           "    //  This is the de-serializer code used for creating messages from a JSON tree.\n"
           "    " << name << "(tJSON* json) {\n"
           "        id          = " << std::string("IDM_")+upper_basename <<"; \n"
           "        type        = " << msgtype << ";\n"
           "        if (json != nullptr) {\n";
    fromJSONBuddy(hdr);
    hdr << "        }\n"
           "    }\n";

    //
    //  This generates the virtual destructor.
    hdr << "    virtual ~" << name << "() = default;\n\n";
    //
    //  Create the size method.
    hdr << "    virtual size_t size() {\n"
           "        return 0;\n"
           "    }\n";
    hdr << "    //\n"
           "    //  This is the serializer code used for sending out messages in JSON format.\n";
    hdr << "    virtual std::ostream& pack_json(std::ostream& output) {\n";
    toJSONBuddy(hdr, "output");
    hdr << "\n        return output;\n"
           "    }\n";

    hdr << "    virtual std::ostringstream& pack_json() {\n"
           "        oss.clear();\n"
           "        oss.str() = \"\";\n"
           "        oss << '{';\n";
    toJSONBuddy(hdr, "oss");
    hdr << "        oss << '}';\n";
    hdr << "\n        return oss;\n"
           "    }\n";


    for (auto & a : Attribute) {
        auto aa = std::dynamic_pointer_cast<CAttribute>(*a);
        std::string fqn=a->FQN();

        if (fqn == "__simobject__") {
            fqn = (aa->Aggregation == aShared)?"tSimObj*":"objectid_t";
        } else {
        }
        if (aa->Multiplicity == "1") {
            alist.emplace_back( fqn, a->name, aa->defaultValue);
        } else if (aa->Multiplicity == "0..1") {
            alist.emplace_back( fqn+"*", a->name, aa->defaultValue);
        } else if (aa->Multiplicity == "0..*") {
            if (aa->QualifierType.empty()) {
                alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
            } else {
                alist.emplace_back( std::string("std::map< ") + aa->QualifierType + ", "+fqn+" >", a->name);
            }
        } else if (aa->Multiplicity == "1..*") {
            if (aa->QualifierType.empty()) {
                alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
            } else {
                alist.emplace_back( std::string("std::map< ") + aa->QualifierType + ", "+fqn+" >", a->name);
            }
        } else if (aa->Multiplicity == "*") {
            if (aa->QualifierType.empty()) {
                alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
            } else {
                alist.emplace_back( std::string("std::map< ") + aa->QualifierType + ", "+fqn+" >", a->name);
            }
        } else if (aa->Multiplicity.empty()) {
            alist.emplace_back( fqn, a->name, aa->defaultValue);
        } else {
            alist.emplace_back( fqn, a->name+"["+aa->Multiplicity+"]");
        }
    }
    //
    //  Processing the aggregations/compositions that are navigable.
    for (auto & ar : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ar);

        if (a->isNavigable()) {
            auto aa = std::dynamic_pointer_cast<CAssociationEnd>(a);
            std::string fqn=a->FQN();

            for (ilist=alist.begin(); ilist!=alist.end(); ++ilist) {
                if (ilist->mName == a->name) {
                    break;
                }
            }
            if (ilist == alist.end()) {
                if (fqn == "__simobject__") {
                    fqn = (a->Aggregation == aShared)?"tSimObj*":"objectid_t";
                } else {
                }
                if (a->Multiplicity == "1") {
                    alist.emplace_back( fqn, a->name, a->defaultValue);
                } else if (a->Multiplicity == "0..1") {
                    alist.emplace_back( fqn+"*", a->name, a->defaultValue);
                } else if (a->Multiplicity == "0..*") {
                    if (aa->QualifierType.empty()) {
                        alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back( std::string("std::map< ") + aa->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity == "1..*") {
                    if (aa->QualifierType.empty()) {
                        alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back( std::string("std::map< ") + aa->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity == "*") {
                    if (aa->QualifierType.empty()) {
                        alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back( std::string("std::map< ") + aa->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity.empty()) {
                    alist.emplace_back( fqn, a->name, a->defaultValue);
                } else {
                    alist.emplace_back( fqn, a->name+"["+a->Multiplicity+"]");
                }
            }
        }
    }
    for (auto & ilist : alist) {
        if (ilist.mType.size() > typemax) {
            typemax = ilist.mType.size();
        }
        if (ilist.mName.size() > namemax) {
            namemax = ilist.mName.size();
        }
    }
    for (auto & ilist : alist) {
        std::string filler;

        filler.assign(typemax - ilist.mType.size() + 1, ' ');

        hdr << "    " << ilist.mType << filler << ilist.mName;
        if (!ilist.mDefault.empty()) {
            std::string dfiller;

            dfiller.assign(namemax - ilist.mName.size() + 1, ' ');

            hdr << dfiller << " = " << ilist.mDefault;
        } else {

        }
        hdr << ";\n";
    }
    hdr << "};\n";
    hdr << "\n\n"
           "using " << name << "Ptr = std::shared_ptr<" << name << ">;\n\n";

    DumpGuardTail(hdr, name);
    CloseStreams();

}

void CMessageClass::fromJSONBuddy(std::ofstream& ifc) {
    ifc <<  "            tJSON *j;\n"
            "\n"
            "            j = find(json, \"Destination\");\n"
            "            if (j != nullptr) {\n"
            "                dst  = tReference(((tJSONValue*)(j))->value, nullptr);\n"
            "            } else {\n"
            "                dst  = tReference(0, nullptr);\n"
            "            }\n"
            "\n"
            "            j = find(json, \"SeqNumber\");\n"
            "            if (j != nullptr) {\n"
            "                seq  = ((tJSONValue*)j)->value;\n"
            "            }\n"
            "\n";

    for (auto & aa : allAttr) {
        if (aa->visibility == vPublic) {
            ifc <<  "            j = find(json, \"" << aa->name <<"\");\n"
                    "            if (j != nullptr) {\n";

            if (aa->Classifier) {
                if (!aa->isMultiple) {
                    if ((aa->Classifier->type == eElementType::Struct) || (aa->Classifier->type == eElementType::CxxClass)) {
                    } else {
                        ifc << "                " << aa->name << " = to_" << aa->Classifier->name << "(j);\n";
                    }
                } else {
                    //
                    //  Process array.
                    DumpFromJSONArray(ifc, "oss", aa, "", false, 4);
                }
            } else {
                ifc << "                " << aa->name << " = to_" << aa->ClassifierName << "(j);\n";
            }
            ifc << "            }";
            if (!aa->defaultValue.empty()) {
                ifc <<  " else {\n"
                        "                " << aa->name << " = " << aa->defaultValue << ";\n"
                        "              }";
            }
            ifc << std::endl;
        }
    }
    for (auto & ai : allEnds) {
        if (ai->visibility == vPublic) {
            ifc <<  "            j = find(json, \"" << ai->name <<"\");\n"
                                                                 "            if (j != nullptr) {\n";
            //
            //  Check for single attribute.
            if (!ai->isMultiple) {
                if ((ai->Classifier->type == eElementType::Struct) || (ai->Classifier->type == eElementType::CxxClass)) {
                } else {
                    ifc << "                " << ai->name << " = to_" << ai->Classifier->name << "(j);\n";
                }
            } else {
                //
                //  Array de-serializer
                DumpFromJSONArray(ifc, "oss", ai, "", false, 4);
            }
            ifc << "            }";
            if (!ai->defaultValue.empty()) {
                ifc <<  " else {\n"
                        "                " << ai->name << " = " << ai->defaultValue << ";\n"
                                                                                     "              }";
            }
            ifc << std::endl;
        }
    }
}

#if 0

void CMessageClass::DumpJSONIncoming(std::ofstream& ifc) {
    ifc <<  "// **************************************************************************\n"
            "//\n"
            "//  Method-Name   : msg_from_json_" << helper::tolower(basename) << "()\n"
            "//\n"
            "//  Generated source code.\n"
            "//\n"
            "// **************************************************************************\n"
            "static tMsg* msg_from_json_" << helper::tolower(basename) << "(tJSON*  json) {\n"
            "    " << name << "* newsig = new " << name << ";\n"

            "    tJSON *j;\n"
            "\n"
            "    j = find(json, \"Destination\");\n"
            "    if (j != nullptr) {\n"
            "        newsig->dst = tCommTarget(to_objectid_t(j), nullptr);\n"
            "    } else {\n"
            "        newsig->dst  = tCommTarget(0, nullptr);\n"
            "    }\n";
    std::list<MAttribute*> al = GetAttributes();
    for (std::list<MAttribute*>::iterator ai=al.begin(); ai!=al.end(); ++ai) {
        CAttribute* a=(CAttribute*)(*ai);
        if (a->visibility == vPublic) {
            ifc << "    j=find(json, \"" << a->name <<"\");\n";
            ifc << "    if (j!=0) {\n";
            if (a->Classifier != 0) {
                if (!a->isMultiple) {
                    if (a->Classifier->type == eElementType::Struct) {
                        ifc << "        newsig->" << a->name << "= " << a->Classifier->name << " {"
                                                                                               "};\n";
                    } else {
                        ifc << "        newsig->" << a->name << "=to_" << a->Classifier->name << "(j);\n";
                    }
                } else {
                    ifc << " //  this is a multi structure\n";
                }
            } else {
                ifc << "        newsig->" << a->name << "=to_" << a->ClassifierName << "(j);\n";
            }
            ifc << "    }";
            if (!a->defaultValue.empty()) {
                ifc << " else {\n";
                ifc << "        newsig->" << a->name << " = " << a->defaultValue << ";\n";
                ifc << "    }";
            }
            ifc << std::endl;
        }
    }
    ifc << "    return ((tMsg*)(newsig));\n";
    ifc << "}\n";

}
#endif

void CMessageClass::DumpJSONArray(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAttribute> a, const std::string& prefix, bool first, int space) {
    std::string filler;
    std::string runner;

    runner=(char)('a'+space/4);

    filler.assign(space, ' ');
    if (!first) {
        ifc << filler << "    " << a_stream  << "<< \", \\\"" << a->name << "\\\": [ \";\n";
    } else {
        ifc << filler << "    " << a_stream <<  "<< \"\\\"" << a->name << "\\\": [ \";\n";
    }
    if (a->Multiplicity=="*") {
        ifc << filler << "    for (std::vector<" << a->FQN() << ">::iterator " << runner << " = " << prefix << a->name << ".begin(); "<< runner <<" != "<< prefix << a->name << ".end(); ++"<< runner <<") {\n";
    } else {
        ifc << filler << "    " << a->FQN() << " *" << runner << ";\n"
            << filler << "    for (" << runner << "= (" << prefix << a->name << "); "<< "("<< runner << "-(" << prefix << a->name << ")" <<") < "<< a->Multiplicity << "; ++"<< runner <<") {\n";
    }
    if (a->Classifier->type == eElementType::Struct) {
        ifc << filler << "        if (" << runner << " == " << prefix << a->name << ".begin()) {\n"
            << filler << "            " << a_stream <<  " << \"{\\n\";\n"
            << filler << "        } else {\n"
            << filler << "            " << a_stream << " << \", {\\n\";\n"
            << filler << "        }\n";
        DumpJSONStruct(ifc, a_stream, a->Classifier, "", runner+"->", false, space);
    } else {
        if (a->Multiplicity == "*") {
            ifc << filler << "        if (" << runner << " != " << prefix << a->name << ".begin()) {\n";
        } else {
            ifc << filler << "        if (" << runner << " != " << "(" << prefix << a->name << ")) {\n";
        }
        ifc << filler << "             " << a_stream <<  " << \", \";\n"
            << filler << "        }\n";
        if (a->ClassifierName == "string") {
                ifc << filler << "        " << a_stream <<  " << \"\\\"\" << " << "(*" << runner << ")" << " << \"\\\"\";\n";
        } else {
            if ((a->ClassifierName == "uint64_t") || (a->ClassifierName == "objectid_t")) {
                    ifc << filler << "        " << a_stream << " << (int64_t)" << "(*" << runner << ")" << ";\n";
            } else {
                    ifc << filler << "        " << a_stream  << " << (*" << runner << ")" << ";\n";
            }
        }
    }

    ifc << filler << "    }\n"
        << filler << "    " << a_stream  <<  " << \"]\\n\";\n";
}

void CMessageClass::DumpJSONArray(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAssociationEnd> a, const std::string& prefix, bool first, int space) {
    std::string filler;
    std::string runner;

    runner=(char)('a'+space/4);

    filler.assign(space, ' ');
    if (!first) {
        ifc << filler << "    " << a_stream  <<  " << \", \\\"" << a->name << "\\\": [ \";\n";
    } else {
        ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\": [ \";\n";
    }
    ifc << filler << "    for (std::vector<" << a->FQN() << ">::iterator " << runner << " = " << prefix << a->name << ".begin(); "<< runner <<" != "<< prefix << a->name << ".end(); ++"<< runner <<") {\n";
    if (a->Classifier->type == eElementType::Struct) {
        ifc << filler << "        if (" << runner << " == " << prefix << a->name << ".begin()) {\n"
            << filler << "            " << a_stream <<  " << \"{\\n\";\n"
            << filler << "        } else {\n"
            << filler << "            " << a_stream <<  " << \", {\\n\";\n"
            << filler << "        }\n"
            << filler << "// struct\n";
        DumpJSONStruct(ifc, a_stream, a->Classifier, "", runner+"->", false, space);
    } else {
        ifc << filler << "        if (" << runner << " != " << prefix << a->name << ".begin()) {\n"
            << filler << "            " << a_stream <<  " << \", \";\n"
            << filler << "        }\n";
        if (a->Classifier->name == "string") {
                ifc << filler << "        " << a_stream  <<  " << \"\\\"\" << " << "(*" << runner << ")" << " << \"\\\"\";\n";
        } else {
            if ((a->Classifier->name == "uint64_t") || (a->Classifier->name == "objectid_t")) {
                    ifc << filler << "        " << a_stream  << " << (int64_t)" << "(*" << runner << ")" << ";\n";
            } else {
                    ifc << filler << "        " << a_stream  << " << (*" << runner << ")" << ";\n";
            }
        }
    }

    ifc << filler << "    }\n"
        << filler << "    " << a_stream <<  " << \"]\\n\";\n";
}


void CMessageClass::DumpJSONStruct(std::ofstream& ifc, const std::string& a_stream , std::shared_ptr<MElement> a, const std::string& sname, const std::string& prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->type == eElementType::Struct) {
        auto s = std::dynamic_pointer_cast<CStruct>(a);

        if (!sname.empty()) {
            if (!first) {
                ifc << filler << "    " << a_stream  <<  " << \", \\\"" << sname << "\\\": { \";\n";
            } else {
                ifc << filler << "    " << a_stream <<  " << \"\\\"" << sname << "\\\": { \";\n";
            }
        } else {
        }
        auto al = s->GetAttributes();
        for (auto & ai : al) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ai);
            //
            //  Check for single attribute.
            if (a->Multiplicity.empty() || (a->Multiplicity == "1")) {
                if (a->Classifier && ((a->Classifier->type==eElementType::Struct) || (a->Classifier->type == eElementType::CxxClass))) {
                    DumpJSONStruct(ifc, a_stream, a->Classifier, a->name, a->name+".", ai == *al.begin(), space+4);
                } else {
                    DumpJSONValue(ifc, a_stream, a, prefix, ai == *al.begin(), space+4);
                }
            } else {
                DumpJSONArray(ifc, a_stream, a, prefix, ai == *al.begin(), space+4);
            }
        }

        for (auto & ae: s->OtherEnd) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ae);
            if (a->isNavigable()) {
                //
                //  Check for single attribute.
                if (a->Multiplicity.empty() || (a->Multiplicity == "1")) {
                    if (a->Classifier && ((a->Classifier->type==eElementType::Struct) || (a->Classifier->type == eElementType::CxxClass))) {
                        DumpJSONStruct(ifc, a_stream, a->Classifier, a->name, prefix+a->name+".", ae == *(s->OtherEnd.begin()), space+4);
                    } else {
                        DumpJSONValue(ifc, a_stream, a, prefix, ae == *(s->OtherEnd.begin()), space+4);
                    }
                } else {
                    DumpJSONArray(ifc, a_stream, a, prefix, ae == *(s->OtherEnd.begin()), space+4);
                }
            }
        }

        if (!sname.empty()) {
            ifc << filler << "    " << a_stream  <<  " << \"}\\n\";\n";
        } else {
            ifc << filler << "        " << a_stream <<  " << \"}\\n\";\n";
        }
    }
}

void CMessageClass::DumpJSONValue(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAttribute> a, const std::string& prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->ClassifierName == "string") {
        if (!first) {
            ifc << filler << "    " << a_stream  <<  " << \", \\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        } else {
            ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        }
    } else {
        if ((a->ClassifierName == "uint64_t") || (a->ClassifierName == "objectid_t")) {
            if (!first) {
                ifc << filler << "    " << a_stream <<  " << \", \\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            }
        } else  if (a->ClassifierName == "bool") {
            if (!first) {
                ifc << filler << "    " << a_stream <<  " << \", \\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            } else {
                ifc << filler << "    " << a_stream  <<  " << \"\\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            }
        } else if (a->ClassifierName == "uint8_t") {
            if (!first) {
                ifc << filler << "    " << a_stream  <<  " << \", \\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            } else {
                ifc << filler << "     " << a_stream <<  " << \"\\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            }
        } else {
            if (!first) {
                ifc << filler << "    " << a_stream <<  " << \", \\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            }
        }
    }
}

void CMessageClass::DumpJSONValue(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAssociationEnd> a, const std::string& prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->Classifier->name == "string") {
        if (!first) {
            ifc << filler << "    " << a_stream <<  " << \", \\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        } else {
            ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        }
    } else {
        if ((a->Classifier->name == "uint64_t") || (a->Classifier->name == "objectid_t") || (a->Classifier->type == eElementType::Enumeration)) {
            if (!first) {
                ifc << filler << "    " << a_stream  <<  " << \", \\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            }
        } else if (a->Classifier->name == "bool") {
            if (!first) {
                ifc << filler << "    " << a_stream <<  " << \", \\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            } else {
                ifc << filler << "    " << a_stream <<  " << \"\\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            }
        } else if (a->Classifier->name == "uint8_t") {
            if (!first) {
                ifc << filler << "    " << a_stream  <<  " << \", \\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            } else {
                ifc << filler << "    " << a_stream  <<  " << \"\\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            }
        } else {
            if (!first) {
                ifc << filler << "    " << a_stream <<  " << \", \\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    " << a_stream  <<  " << \"\\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            }
        }
    }
}

void CMessageClass::DumpJSONOutgoingDeclaration(std::ofstream& ifc) {
    ifc << "static std::ostream& msg_to_json_" << helper::tolower(basename) << "(tMsg* aMsg, std::ostream& output);\n";
}

void
CMessageClass::DumpFromJSONArray(std::ofstream &ifc, const std::string &a_stream, const std::shared_ptr<CAttribute> a,
                                 const std::string &prefix, bool first, int space) {
    std::string filler(space + 4, ' ');

    if (a) {
        if (a->Classifier) {
            auto ac = std::dynamic_pointer_cast<CClassBase>(*a->Classifier);

//            std::string searchvalue = prefix + "/" + ac->name;
//            ifc << "    j = find(\"" << searchvalue << "\");\n"
            ifc << filler << "        for (auto __b : ((tJSONArray*)(j))->values) {\n";
            //
            //  Check for complex datatypes like structs or classes.
            if (!((ac->type == eElementType::Struct) || (ac->type == eElementType::CxxClass))) {
                //
                //  Simple type
                ifc << filler << "            if (__b->type == eValue) {\n";
                ifc << filler << "                " << a->name << ".push_back(((tJSONValue*)__b)->value);\n";
                ifc << filler << "            }\n";
            }
            ifc << filler << "        }\n";
        }
    }
}

void
CMessageClass::DumpFromJSONArray(std::ofstream &ifc, const std::string &a_stream, const std::shared_ptr<CAssociationEnd> a,
                                 const std::string &prefix, bool first, int space) {

    std::string filler(space + 4, ' ');

    if (a) {
        if (a->Classifier) {
            auto ac = std::dynamic_pointer_cast<CClassBase>(*a->Classifier);

//            std::string searchvalue = prefix + "/" + ac->name;
//            ifc << "    j = find(\"" << searchvalue << "\");\n"
            ifc << filler << "        for (auto __b : ((tJSONArray*)(j))->values) {\n";
            //
            //  Check for complex datatypes like structs or classes.
            if (!((ac->type == eElementType::Struct) || (ac->type == eElementType::CxxClass))) {
                //
                //  Simple type
                ifc << filler << "            if (__b->type == eValue) {\n";
                ifc << filler << "                " << ac->name << ".push_back((tJSONValue*)__b)->value);\n";
                ifc << filler << "            }\n";
            }
            ifc << filler << "        }\n";
        }
    }

}

void CMessageClass::toJSONBuddy(std::ofstream & ifc, const std::string& a_stream) {
    std::string prefix = "this->";
    auto al = GetAttributes();

    ifc << "        " << a_stream << " << \"\\\"MsgId\\\": \\\"" << basename << "\\\",\";\n";

    ifc <<  "\n        if (std::holds_alternative<tConnection>(dst)) {\n"
            "            seq = std::get<tConnection>(dst).seqnumber;\n"
            "        }\n\n";

    ifc << "        " << a_stream << " << \"\\\"SeqNumber\\\": \" << seq ;\n";

    ifc <<  "        if (std::holds_alternative<tReference>(dst)) {\n"
            "            auto dstid = std::get<tReference>(dst).m_id;\n"
            "\n"
            "            if (dstid != 0) {\n"
            "                " << a_stream << " << \", \\\"Destination\\\": \\\"\" << dstid;\n"
            "            }\n"
            "        }\n";

    for (auto & ai: al) {
        if (ai->visibility == vPublic) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ai);
            //
            //  Check for single attribute.
            if (a->Multiplicity.empty() || (a->Multiplicity == "1")) {
                if (a->Classifier && ((a->Classifier->type==eElementType::Struct) || (a->Classifier->type == eElementType::CxxClass))) {
                    DumpJSONStruct(ifc, a_stream, a->Classifier, a->name, prefix+a->name+".", false, 4);
                } else {
                    DumpJSONValue(ifc, a_stream, a, prefix, false, 4);
                }
            } else {
                DumpJSONArray(ifc, a_stream, a, prefix, false, 4);
            }
        }
    }

    for (auto & ai : OtherEnd) {
        if (ai->visibility == vPublic) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ai);
            //
            //  Check for single attribute.
            if (a->Multiplicity.empty() || (a->Multiplicity == "1")) {
                if (a->Classifier && ((a->Classifier->type == eElementType::Struct) || (a->Classifier->type == eElementType::CxxClass))) {
                    DumpJSONStruct(ifc, a_stream, a->Classifier, a->name, prefix+a->name+".", false, 4);
                } else {
                    DumpJSONValue(ifc, a_stream, a, prefix, false, 4);
                }
            } else {
                DumpJSONArray(ifc, a_stream, a, prefix, false, 4);
            }
        }
    }
}

