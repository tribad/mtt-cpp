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
#include "csignalclass.h"
#include "ccxxclass.h"

#include "cstruct.h"

#include "mmodel.h"
#include "cmodel.h"

#include "main.h"

extern long  simversion;

#if 0
void CSignalClass::CollectNeededModelHeader(std::shared_ptr<MElement> e) {
    auto  c = std::dynamic_pointer_cast<MClass>(e);

    if (donelist.find(e->name)==donelist.end()) {
        donelist.insert(e->name);

        for (auto & i : c->Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*i);

            if (((a->Aggregation == aNone) || (a->Aggregation == aComposition)) && a->Classifier) {
                eElementType et=a->Classifier->type;

                if ((et != eElementType::SimObject) && (et != eElementType::ExternClass)) {
                    CollectNeededModelHeader(a->Classifier);
                }
            }
        }
        for (auto & i : c->OtherEnd) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);

            if ((a->Aggregation != aShared) && a->Classifier && a->isNavigable()) {
                eElementType et=a->Classifier->type;

                if ((et != eElementType::SimObject) && (et != eElementType::ExternClass)) {
                    CollectNeededModelHeader(a->Classifier);
                }
            }
        }
        if (e.get() != this) {
            neededmodelheader.add(e);
        }
    }
}

#endif

void CSignalClass::SetFromTags(const std::string& name, const std::string&value)
{
    if (name == "BaseName") {
        basename=value;
    } if (name== "Direction") {
        direction = value;
    }
}

std::string CSignalClass::FQN() const {
    return name;
}

void CSignalClass::Prepare(void) {
    PrepareBase();
    if (HasStereotype("protobuf")) {
        m_encoding = SignalEncoding::protobuf;
        has_src.clear();
        has_hdr.clear();
    } else if ((HasStereotype("json")) || (HasStereotype("simsignal"))) {
        m_encoding = SignalEncoding::json;
    } else if (HasStereotype("tlv")) {
        m_encoding = SignalEncoding::tlv;
    } else {
        // 
        //  take the default value none
    }

    if (basename.empty())  {
        basename = name;
    }
    lower_basename=helper::tolower(basename);
    upper_basename=helper::toupper(basename);
    msgtype = "MSG_TYPE_UNSPECIFIED";
    if (!direction.empty()) {
        msgtype = "MSG_TYPE_" + helper::toupper(direction);
    }
}

void CSignalClass::Dump(std::shared_ptr<MModel> model) {

    CollectNeededModelHeader(sharedthis<MElement>(), neededmodelheader);

    switch (m_encoding) {
    case SignalEncoding::json:
        DumpJSON(model);
        break;
    case SignalEncoding::protobuf:
        DumpProtobuf(model);
        break;
    case SignalEncoding::tlv:
        DumpTLV(model);
        break;
    default:
        DumpJSON(model);
        break;
    }
}

void CSignalClass::DumpProtobufAttributes(std::ofstream& a_pbfile, int a_indentation) {
    auto alla = GetAttributes();
    std::string filler(a_indentation, ' ');

    for (auto & a : alla) {
        if (a->Classifier) {
            auto cb = a->Classifier->sharedthis<CClassBase>();
            a_pbfile << filler << cb->mTypeTree.mName << " " << a->name << " = " << m_attribute_index++ << ";\n";
        } else {
            a_pbfile << filler << a->ClassifierName << " " << a->name << " = " << m_attribute_index++ << ";\n";
        }
    }
}

void CSignalClass::DumpProtobuf(std::shared_ptr<MModel> model) {
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);
    DumpBase(cmodel, name);
    std::ofstream pbfile;
    OpenStream(pbfile, name + ".proto");
    pbfile << "syntax = \"proto3\";\n\n";
    pbfile << "message " << name << " {\n";
    DumpProtobufAttributes(pbfile, IndentSize);
    pbfile << "}\n\n";
    CloseStreams();
}

void CSignalClass::DumpJSON(std::shared_ptr<MModel> model) {
    std::list< tAttributePack >           alist;
    std::list< tAttributePack >::iterator ilist;
    size_t                                typemax = 0;
    size_t                                namemax = 0;
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);
    DumpBase(cmodel, name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, name);

    hdr << "\n#include <simobj.h>\n";
    hdr << "#include <helper.h>\n";

    donelist.clear();
    for (auto & i : optionalmodelheader) {
        std::string header = i->tags.find("ExtraInclude")->second;
        std::string extra;
        size_t      start       = 0;
        size_t      end         = 0;

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

    DumpNeededIncludes(hdr, std::dynamic_pointer_cast<CClassBase>(MClass::Instances[id]), donelist, oplist);
    Crc64 crc;
    hdr << "\n#define " << std::string("IDS_")+upper_basename << " 0x" << std::hex << std::setfill('0') << std::setw(16) << crc.calc(std::string("IDS_")+upper_basename) << "\n";
    hdr << "//\n";
    hdr << "//                   S i g n a l c l a s s     d e c l a r a t i o n\n";
    hdr << "struct " << name << " : public tMsg {\n";
    hdr << "    " << name << "() { id = " << std::string("IDS_")+upper_basename <<"; type = " << msgtype << ";}\n";
    hdr << "    " << name << "(const tCommTarget& aDst, const tCommTarget& aSrc) {\n"
                             "        id = " << std::string("IDS_")+upper_basename <<"; \n"
                             "        type = " << msgtype << ";\n"
                             "        src = aSrc;\n"
                             "        dst = aDst;\n"
                             "    }\n";

    hdr << "    " << name << "(tSimObj* aDst, tSimObj* aSrc) {\n"
                             "        id          = " << std::string("IDS_")+upper_basename <<"; \n"
                             "        type        = " << msgtype << ";\n"
                             "        if (aSrc != nullptr) {\n"
                             "            src    = tReference(aSrc->objid, aSrc);\n"
                             "        } else {\n"
                             "            src    = std::monostate();\n"
                             "        }\n"
                             "        if (aDst != nullptr) {\n"
                             "            dst    = tReference(aDst->objid, aDst);\n"
                             "        } else {\n"
                             "            dst    = std::monostate();\n"
                             "        }\n"
                             "    }\n";

    hdr << "    //\n"
           "    //  This is the de-serializer code used for creating messages from a JSON tree.\n";
    hdr << "    " << name << "(tJSON* json) {\n"
           "        id          = " << std::string("IDS_")+upper_basename <<"; \n"
           "        type        = " << msgtype << ";\n"
           "        if (json != 0) {\n";
    fromJSONBuddy(hdr);
    hdr << "        }\n"
           "    }\n";

    hdr << "    virtual ~" << name << "() {}\n";
    hdr << "    //\n"
           "    //  This is the serializer code used for sending out messages in JSON format.\n";
    hdr << "    virtual std::ostream& json(std::ostream& output) {\n";
    toJSONBuddy(hdr);
    hdr << "\n        return output;\n"
           "    }\n";

    for (auto & i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string fqn=a->FQN();

        if (fqn == "__simobject__") {
            fqn = (a->Aggregation == aShared)?"tSimObj*":"objectid_t";
        } else {
        }
        if (a->Multiplicity == "1") {
            alist.emplace_back( fqn, a->name, a->defaultValue);
        } else if (a->Multiplicity == "0..1") {
            alist.emplace_back( fqn+"*", a->name, a->defaultValue);
        } else if (a->Multiplicity == "0..*") {
            if (a->QualifierType.empty()) {
                alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
            } else {
                alist.emplace_back( std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
            }
        } else if (a->Multiplicity == "1..*") {
            if (a->QualifierType.empty()) {
                alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
            } else {
                alist.emplace_back( std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
            }
        } else if (a->Multiplicity == "*") {
            if (a->QualifierType.empty()) {
                alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
            } else {
                alist.emplace_back( std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
            }
        } else if (a->Multiplicity.empty()) {
            alist.emplace_back( fqn, a->name, a->defaultValue);
        } else {
            alist.emplace_back( fqn, a->name+"["+a->Multiplicity+"]");
        }
    }
    //
    //  Processing the aggregations/compositions that are navigable.
    for (auto & i : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);

        if (a->isNavigable()) {
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
                    if (a->QualifierType.empty()) {
                        alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back( std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity == "1..*") {
                    if (a->QualifierType.empty()) {
                        alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back( std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity == "*") {
                    if (a->QualifierType.empty()) {
                        alist.emplace_back( std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back( std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity.empty()) {
                    alist.emplace_back( fqn, a->name, a->defaultValue);
                } else {
                    alist.emplace_back( fqn, a->name+"["+a->Multiplicity+"]");
                }
            }
        }
    }
    for (ilist = alist.begin(); ilist != alist.end(); ++ilist) {
        if (ilist->mType.size() > typemax) {
            typemax = ilist->mType.size();
        }
        if (ilist->mName.size() > namemax) {
            namemax = ilist->mName.size();
        }
    }
    for (ilist = alist.begin(); ilist != alist.end(); ++ilist) {
        std::string filler;

        filler.assign(typemax - ilist->mType.size() + 1, ' ');

        hdr << "    " << ilist->mType << filler << ilist->mName;
        if (!ilist->mDefault.empty()) {
            std::string dfiller;

            dfiller.assign(namemax - ilist->mName.size() + 1, ' ');

            hdr << dfiller << " = " << ilist->mDefault;
        }
        hdr << ";\n";
    }
    hdr << "};\n";
    hdr << "\n\n"
           "using " << name << "Ptr = std::shared_ptr<" << name << ">;\n\n";
    DumpGuardTail(hdr, name);

    CloseStreams();
}
void CSignalClass::DumpJSONIncomingDeclaration(std::ofstream& ifc) {
    ifc << "static tSig* sig_from_json_" << helper::tolower(basename) << "(tJSON*  json);\n";
}

void CSignalClass::fromJSONBuddy(std::ofstream& ifc) {
    ifc << "            tJSON *j;\n";
    ifc << "\n";
    ifc << "            j=find(json, \"Destination\");\n";
    ifc << "            if (j != 0) {\n";
    ifc << "                dst  = tReference(((tJSONValue*)j)->value, nullptr);\n";
    ifc << "            } else {\n";
    ifc << "                dst  = tReference(0, nullptr);\n";
    ifc << "            }\n";

    auto al = GetAttributes();
    for (auto & ai : al) {
        auto a = std::dynamic_pointer_cast<CAttribute>(ai);

        if (a->visibility == vPublic) {
            ifc << "            j = find(json, \"" << a->name <<"\");\n";
            ifc << "            if (j != nullptr) {\n";
            if (a->Classifier) {
                if (!a->isMultiple) {
                    if (a->Classifier->type == eElementType::Struct) {
                        //ifc << "                " << a->name << " = " << a->Classifier->name << " {};\n";
                    } else {
                        ifc << "                " << a->name << " = to_" << a->Classifier->name << "(j);\n";
                    }
                } else {
                    DumpJSONIncomingArray(ifc, a, "", 16);
                }
            } else {
                //
                //  Here it is some cardinal type.
                if (!a->isMultiple) {
                    ifc << "                " << a->name << " = to_" << a->ClassifierName << "(j);\n";
                } else {
                    DumpJSONIncomingArray(ifc, a, "", 16);
                }
            }
            if (a->Classifier || a->defaultValue.empty()) {
                ifc << "            }\n";
            } else {
                if ((!a->isMultiple) && (a->Classifier->type != eElementType::Struct)) {
                    ifc << "            } else {\n";
                    ifc << "                " << a->name << " = 0;\n";
                    ifc << "            }\n";
                }
            }
        }
    }
    for (auto & ai : OtherEnd) {
        if (ai->visibility == vPublic) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ai);
            //
            //  Check for single attribute.
            if (a->Multiplicity.empty() || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
//                    DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".", false, 4);
                } else {
//                    DumpJSONValue(ifc, a, prefix, false, 4);
                }
            } else {
                DumpJSONIncomingArray(ifc, a, "", 12);
            }
        }
    }
}

void CSignalClass::DumpJSONIncoming(std::ofstream& ifc) {
    ifc << "// **************************************************************************\n";
    ifc << "//\n";
    ifc << "//  Method-Name   : sig_from_json_" << helper::tolower(basename) << "()\n";
    ifc << "//\n";
    ifc << "//  Generated source code.\n";
    ifc << "//\n";
    ifc << "// **************************************************************************\n";
    ifc << "static tSig* sig_from_json_" << helper::tolower(basename) << "(tJSON*  json) {\n";
    ifc << "    " << name << "* newsig = new " << name << ";\n";

    ifc << "    tJSON *j;\n";
    ifc << "\n";
    ifc << "    j=find(json, \"Destination\");\n";
    ifc << "    if (j != 0) {\n";
    ifc << "        newsig->dst  = tReference(to_objectid_t(j), nullptr);\n";
    ifc << "    } else {\n";
    ifc << "    }\n";
    auto al = GetAttributes();

    for (auto & ai : al) {
        auto a = std::dynamic_pointer_cast<CAttribute>(ai);

        if (a->visibility == vPublic) {
            ifc << "    j=find(json, \"" << a->name <<"\");\n";
            ifc << "    if (j!=0) {\n";
            if (a->Classifier) {
                if (!a->isMultiple) {
                    if (a->Classifier->type == eElementType::Struct) {
                        ifc << "        newsig->" << a->name << "= " << a->Classifier->name << " {};\n";
                    } else {
                        ifc << "        newsig->" << a->name << "=to_" << a->Classifier->name << "(j);\n";
                    }
                } else {
                    DumpJSONIncomingArray(ifc, a, "newsig->", 8);
                }
            } else {
                //
                //  Here it is some cardinal type.
                if (!a->isMultiple) {
                    ifc << "        newsig->" << a->name << "=to_" << a->ClassifierName << "(j);\n";
                } else {
                    DumpJSONIncomingArray(ifc, a, "newsig->", 8);
                }
            }
            if (a->Classifier || (a->defaultValue.empty())) {
                ifc << "    }\n";
            } else {
                if ((!a->isMultiple) && (a->Classifier->type != eElementType::Struct)) {
                    ifc << "    } else {\n";
                    ifc << "        newsig->" << a->name << " = 0;\n";
                    ifc << "    }\n";
                }
            }
        }
    }
    ifc << "    return ((tMsg*)(newsig));\n";
    ifc << "}\n";
}

void CSignalClass::DumpJSONIncomingArray(std::ofstream &ifc, std::shared_ptr<CAttribute> a, std::string prefix, int space) {
    std::string filler;
    filler.assign(space, ' ');

    ifc << filler << "tJSONArray*                   array = (tJSONArray*)j;\n";
    ifc << filler << "std::vector<tJSON*>::iterator ai;\n";
    if (!a->isCollection) {
        ifc << filler << "int                           ac=0;\n";
    }
    ifc << filler << "\n";
    ifc << filler << "for (ai = array->values.begin(); ai != array->values.end(); ++ai) {\n";

    if (a->Classifier) {
        if (a->Classifier->type == eElementType::Struct) {

        } else {
            //
            //  We expect an array of single values.
            ifc << filler << "    if ((*ai)->type == eValue) {\n";
            if (a->isCollection) {
                ifc << filler << "        " << prefix << a->name << ".push_back(to_" << a->Classifier->name << "(*ai));\n";
            } else {
                ifc << filler << "        " << prefix << a->name << "[ac++] = to_" << a->Classifier->name << "(*ai);\n";
            }
            ifc << filler << "    }\n";
        }
    }

    ifc << filler << "}\n";


}

void CSignalClass::DumpJSONIncomingArray(std::ofstream &ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, int space) {
    std::string filler;
    filler.assign(space, ' ');

    ifc << filler << "tJSONArray*                   array = (tJSONArray*)find(json, \"" << a->name << "\");\n";
    ifc << filler << "if (array != nullptr) {\n";
    if (!a->isCollection) {
        ifc << filler << "    int                           ac=0;\n";
    }
    ifc << filler << "\n";
    ifc << filler << "    for (auto ai : array->values) {\n";

    if (a->Classifier) {
        if (a->Classifier->type == eElementType::Struct) {
            DumpJSONIncomingStruct(ifc, a->Classifier, prefix+a->name, space+4);
            ifc << filler << "        " << a->name << ".push_back(v);" << "\n";

        } else {
            //
            //  We expect an array of single values.
            ifc << filler << "    //\n";
            ifc << filler << "    //  Sanity check here\n";
            ifc << filler << "    if ((*ai)->type == eValue) {\n";
            if (a->isCollection) {
                ifc << filler << "        " << prefix << a->name << ".push_back(to_" << a->Classifier->name << "(*ai));\n";
            } else {
                ifc << filler << "        " << prefix << a->name << "[ac++] = to_" << a->Classifier->name << "(*ai);\n";
            }
            ifc << filler << "        }\n";
        }
    }
    ifc << filler << "    }\n";
    ifc << filler << "}\n";
}

void CSignalClass::DumpJSONIncomingStruct(std::ofstream &ifc, std::shared_ptr<MElement> aStruct, std::string prefix, int space) {
    std::string filler;
    filler.assign(space+4, ' ');

    ifc << filler << aStruct->name << " v;\n";
    ifc << filler << "tJSON* j;\n";

    DumpJSONIncomingValues(ifc, std::dynamic_pointer_cast<CClassBase>(aStruct), "v.", "ai",  space);
}

void CSignalClass::DumpJSONIncomingValues(std::ofstream &ifc, std::shared_ptr<CClassBase> aClass, std::string prefix, std::string jsonvar, int space) {
    std::string filler;
    filler.assign(space, ' ');

    auto al = aClass->GetAttributes();

    for (auto & ai : al) {
        auto a = std::dynamic_pointer_cast<CAttribute>(ai);

        if (a->visibility == vPublic) {
            ifc << filler << "    j = find(" << jsonvar << ", \"" << a->name <<"\");\n";
            ifc << filler << "    if (j != nullptr) {\n";
            if (a->Classifier) {
                if (!a->isMultiple) {
                    if (a->Classifier->type == eElementType::Struct) {
                        //ifc << "                " << a->name << " = " << a->Classifier->name << " {};\n";
                    } else {
                        ifc << filler << "        " << prefix << a->name << " = to_" << a->Classifier->name << "(j);\n";
                    }
                } else {
                    DumpJSONIncomingArray(ifc, a, "", 16);
                }
            } else {
                //
                //  Here it is some cardinal type.
                if (!a->isMultiple) {
                    ifc << filler << "        " << prefix << a->name << " = to_" << a->ClassifierName << "(j);\n";
                } else {
                    DumpJSONIncomingArray(ifc, a, prefix, 16);
                }
            }
            if (a->Classifier || (a->defaultValue.empty())) {
                ifc << filler << "    }\n";
            } else {
                if ((!a->isMultiple) && (a->Classifier->type != eElementType::Struct)) {
                    ifc << filler << "    } else {\n";
                    ifc << filler << "        " << prefix << a->name << " = 0;\n";
                    ifc << filler << "    }\n";
                }
            }
        }
    }
    for (auto & ai : aClass->OtherEnd) {
        if (ai->visibility == vPublic) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ai);
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if ((a->Classifier) && (a->Classifier->type==eElementType::Struct)) {
//                    DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".", false, 4);
                } else {
//                    DumpJSONValue(ifc, a, prefix, false, 4);
                }
            } else {
                DumpJSONIncomingArray(ifc, a, prefix, 12);
            }
        }
    }
}

void CSignalClass::DumpJSONArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first, int space) {
    std::string filler;
    std::string runner;

    runner=(char)('a'+space/4);

    filler.assign(space, ' ');
    if (!first) {
        ifc << filler << "    output <<  \", \\\"" << a->name << "\\\": [ \";\n";
    } else {
        ifc << filler << "    output <<  \"\\\"" << a->name << "\\\": [ \";\n";
    }
    if (a->Multiplicity=="*") {
        ifc << filler << "    for (std::vector<" << a->FQN() << ">::iterator " << runner << " = " << prefix << a->name << ".begin(); "<< runner <<" != "<< prefix << a->name << ".end(); ++"<< runner <<") {\n";
    } else {
        ifc << filler << "    " << a->FQN() << " *" << runner << ";\n";
        ifc << filler << "    for (" << runner << "= (" << prefix << a->name << "); "<< "("<< runner << "-(" << prefix << a->name << ")" <<") < "<< a->Multiplicity << "; ++"<< runner <<") {\n";
    }
    if (a->Classifier->type == eElementType::Struct) {
        ifc << filler << "        if (" << runner << " == " << prefix << a->name << ".begin()) {\n";
        ifc << filler << "            output <<  \"{\\n\";\n";
        ifc << filler << "        } else {\n";
        ifc << filler << "            output <<  \", {\\n\";\n";
        ifc << filler << "        }\n";
        DumpJSONStruct(ifc, a->Classifier, "", runner+"->", false, space);
    } else {
        if (a->Multiplicity == "*") {
            ifc << filler << "        if (" << runner << " != " << prefix << a->name << ".begin()) {\n";
        } else {
            ifc << filler << "        if (" << runner << " != " << "(" << prefix << a->name << ")) {\n";
        }
        ifc << filler << "            output <<  \", \";\n";
        ifc << filler << "        }\n";
        if (a->ClassifierName == "string") {
                ifc << filler << "        output <<  \"\\\"\" << " << "(*" << runner << ")" << " << \"\\\"\";\n";
        } else {
            if ((a->ClassifierName == "uint64_t") || (a->ClassifierName == "objectid_t")) {
                    ifc << filler << "        output << (int64_t)" << "(*" << runner << ")" << ";\n";
            } else {
                    ifc << filler << "        output << (*" << runner << ")" << ";\n";
            }
        }
    }

    ifc << filler << "    }\n";
    ifc << filler << "    output <<  \"]\\n\";\n";
}

void CSignalClass::DumpJSONArray(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first, int space) {
    std::string filler;
    std::string runner;

    runner=(char)('a'+space/4);

    filler.assign(space, ' ');
    if (!first) {
        ifc << filler << "    output <<  \", \\\"" << a->name << "\\\": [ \";\n";
    } else {
        ifc << filler << "    output <<  \"\\\"" << a->name << "\\\": [ \";\n";
    }
    ifc << filler << "    for (std::vector<" << a->FQN() << ">::iterator " << runner << " = " << prefix << a->name << ".begin(); "<< runner <<" != "<< prefix << a->name << ".end(); ++"<< runner <<") {\n";
    if (a->Classifier->type == eElementType::Struct) {
        ifc << filler << "        if (" << runner << " == " << prefix << a->name << ".begin()) {\n";
        ifc << filler << "            output <<  \"{\\n\";\n";
        ifc << filler << "        } else {\n";
        ifc << filler << "            output <<  \", {\\n\";\n";
        ifc << filler << "        }\n";
        ifc << filler << "// struct\n";
        DumpJSONStruct(ifc, a->Classifier, "", runner+"->", false, space);
    } else {
        ifc << filler << "        if (" << runner << " != " << prefix << a->name << ".begin()) {\n";
        ifc << filler << "            output <<  \", \";\n";
        ifc << filler << "        }\n";
        if (a->Classifier->name == "string") {
                ifc << filler << "        output <<  \"\\\"\" << " << "(*" << runner << ")" << " << \"\\\"\";\n";
        } else {
            if ((a->Classifier->name == "uint64_t") || (a->Classifier->name == "objectid_t")) {
                    ifc << filler << "        output << (int64_t)" << "(*" << runner << ")" << ";\n";
            } else {
                    ifc << filler << "        output << (*" << runner << ")" << ";\n";
            }
        }
    }

    ifc << filler << "    }\n";
    ifc << filler << "    output <<  \"]\\n\";\n";
}


void CSignalClass::DumpJSONStruct(std::ofstream& ifc, std::shared_ptr<MElement> a, std::string sname, std::string prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->type == eElementType::Struct) {
        auto s = std::dynamic_pointer_cast<CStruct>(a);

        if (!sname.empty()) {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << sname << "\\\": { \";\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << sname << "\\\": { \";\n";
            }
        } else {
        }
        auto al = s->GetAttributes();
        for (auto ai : al) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ai);
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                    DumpJSONStruct(ifc, a->Classifier, a->name, a->name+".", ai == *al.begin(), space+4);
                } else {
                    DumpJSONValue(ifc, a, prefix, ai == *al.begin(), space+4);
                }
            } else {
                DumpJSONArray(ifc, a, prefix, ai == *al.begin(), space+4);
            }
        }

        for (auto & ae : s->OtherEnd) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ae);

            if (a->isNavigable()) {
                //
                //  Check for single attribute.
                if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                    if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                        DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".", ae == *s->OtherEnd.begin(), space+4);
                    } else {
                        DumpJSONValue(ifc, a, prefix, ae == *s->OtherEnd.begin(), space+4);
                    }
                } else {
                    DumpJSONArray(ifc, a, prefix, ae == *s->OtherEnd.begin(), space+4);
                }
            }
        }

        if (!sname.empty()) {
            ifc << filler << "    output <<  \"}\\n\";\n";
        } else {
            ifc << filler << "        output <<  \"}\\n\";\n";
        }
    }
}

void CSignalClass::DumpJSONValue(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->ClassifierName == "string") {
        if (!first) {
            ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        } else {
            ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        }
    } else {
        if ((a->ClassifierName == "uint64_t") || (a->ClassifierName == "objectid_t")) {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            }
        } else  if (a->ClassifierName == "bool") {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            }
        } else if (a->ClassifierName == "uint8_t") {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            }
        } else {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            }
        }
    }
}

void CSignalClass::DumpJSONValue(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->Classifier->name == "string") {
        if (!first) {
            ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        } else {
            ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        }
    } else {
        if ((a->Classifier->name == "uint64_t") || (a->Classifier->name == "objectid_t") || (a->Classifier->type == eElementType::Enumeration)) {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << (int64_t)" << prefix << a->name << ";\n";
            }
        } else if (a->Classifier->name == "bool") {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << ((" << prefix << a->name << "==true)?(const char*)\"true\":(const char*)\"false\");\n";
            }
        } else if (a->Classifier->name == "uint8_t") {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << (uint16_t)(" << prefix << a->name << ");\n";
            }
        } else {
            if (!first) {
                ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            } else {
                ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\" << " << prefix << a->name << ";\n";
            }
        }
    }
}

void CSignalClass::DumpJSONOutgoingDeclaration(std::ofstream& ifc) {
    ifc << "static std::ostream& sig_to_json_" << helper::tolower(basename) << "(tSig* aSig, std::ostream& output);\n";
}

void CSignalClass::toJSONBuddy(std::ofstream & ifc) {
    std::string prefix;
    prefix="this->";
    ifc << "        output << \"\\\"SignalId\\\": \\\"" << basename << "\\\"\";\n";
    auto al = GetAttributes();
    for (auto & ai : al) {
        auto a = std::dynamic_pointer_cast<CAttribute>(ai);
        //
        //  Check for single attribute.
        if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
            if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".", false, 4);
            } else {
                DumpJSONValue(ifc, a, prefix, false, 4);
            }
        } else {
            DumpJSONArray(ifc, a, prefix, false, 4);
        }
    }
    for (auto & ai : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ai);
        //
        //  Check for single attribute.
        if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
            if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".", false, 4);
            } else {
                DumpJSONValue(ifc, a, prefix, false, 4);
            }
        } else {
            DumpJSONArray(ifc, a, prefix, false, 4);
        }
    }
}


void CSignalClass::DumpJSONOutgoing(std::ofstream& ifc) {
    std::string prefix;
    ifc << "// **************************************************************************\n";
    ifc << "//\n";
    ifc << "//  Method-Name   : sig_to_json_" << helper::tolower(basename) << "()\n";
    ifc << "//\n";
    ifc << "//  Generated source code.\n";
    ifc << "//\n";
    ifc << "// **************************************************************************\n";
    ifc << "static std::ostream& sig_to_json_" << helper::tolower(basename) << "(tSig* aSig, std::ostream& output) {\n";
    ifc << "    " << name << "* sig=("<< name << "*)aSig;\n";
    prefix="sig->";
    ifc << "    output << \"\\\"SignalId\\\": \\\"" << basename << "\\\"\";\n";
    auto al = GetAttributes();

    for (auto & ai: al) {
        auto a = std::dynamic_pointer_cast<CAttribute>(ai);

        if (a->visibility == vPublic) {
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                    DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".");
                } else {
                    DumpJSONValue(ifc, a, prefix);
                }
            } else {
                DumpJSONArray(ifc, a, prefix);
            }
        }
    }
    for (auto & ai : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ai);

        if (a->visibility == vPublic) {
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                    DumpJSONStruct(ifc, a->Classifier, a->name, prefix+a->name+".");
                } else {
                    DumpJSONValue(ifc, a, prefix);
                }
            } else {
                DumpJSONArray(ifc, a, prefix);
            }
        }
    }

    ifc << "    return (output);\n";
    ifc << "}\n";
}

void CSignalClass::DumpTLV(std::shared_ptr<MModel> model) {
}
