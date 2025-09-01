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
#include <fstream>
#include "helper.h"
#include "mattribute.h"
#include "cattribute.h"
#include "mmessage.h"
#include "mjsonmessage.h"
#include "cjsonmessage.h"
#include "mlifeline.h"
#include "clifeline.h"
#include "mclass.h"
#include "cclassbase.h"
#include "ccxxclass.h"
#include "cstruct.h"
#include "massociationend.h"
#include "cassociationend.h"
#include "massociation.h"
#include "cassociation.h"
#include "cmessageclass.h"
#include "csignalclass.h"

std::string CJSONMessage::FQN() const {
    return name;
}

void CJSONMessage::FillClass(void) {
    auto ci=CMessageClass::MessageByName.find(name);

    if (ci!=CMessageClass::MessageByName.end()) {
        Class = ci->second;
    } else {
        auto si = CSignalClass::SignalByName.find(name);

        if (si != CSignalClass::SignalByName.end()) {
            Class  = si->second;
        }
    }
}

std::shared_ptr<MClass> CJSONMessage::GetClass() {
    FillClass();
    return Class;
}

void CJSONMessage::Prepare(void) {
    if (source) {
        std::dynamic_pointer_cast<CLifeLine>(*source)->SetOutgoing(MElement::Instances[id]);
    }
    if (target) {
        std::dynamic_pointer_cast<CLifeLine>(*target)->SetIncoming(MElement::Instances[id]);
    }
}

void CJSONMessage::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}

void CJSONMessage::DumpIncoming(std::ofstream& ifc) {
    std::string tname;
    FillClass();
    if (Class) {
        tname = Class->name;
    } else {
        tname = name;
    }
    if (mtype == mSyncCall) {
        ifc << "// **************************************************************************\n";
        ifc << "//\n";
        ifc << "//  Method-Name   : msg_from_json_" << helper::tolower(name) << "()\n";
        ifc << "//\n";
        ifc << "//  Generated source code.\n";
        ifc << "//\n";
        ifc << "// **************************************************************************\n";
        ifc << "static tMsg* msg_from_json_" << helper::tolower(name) << "(tJSON*  json) {\n";
        ifc << "    " << tname << "* newsig=(" << tname << "*)create_msg(" << std::string("IDM_")+helper::toupper(name) << ");\n";
    } else if ((mtype == mAsyncCall) || (mtype == mAsyncSignal)) {
        ifc << "// **************************************************************************\n";
        ifc << "//\n";
        ifc << "//  Method-Name   : sig_from_json_" << helper::tolower(name) << "()\n";
        ifc << "//\n";
        ifc << "//  Generated source code.\n";
        ifc << "//\n";
        ifc << "// **************************************************************************\n";
        ifc << "static tSig* sig_from_json_" << helper::tolower(name) << "(tJSON*  json) {\n";
        ifc << "    " << tname << "* newsig=(" << tname << "*)create_sig(" << std::string("IDS_")+helper::toupper(name) << ");\n";
    }
    ifc << "    tJSON *j;\n";
    ifc << "\n";
    ifc << "    j=find(json, \"Destination\");\n";
    ifc << "    if (j != 0) {\n";
    ifc << "        newsig->dst    = tCommTarget(to_objectid_t(j), nullptr);\n";
    ifc << "    }\n";
    if (Class) {
        auto al = std::dynamic_pointer_cast<CClassBase>(Class)->GetAttributes();
        for (auto & ai : al) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ai);

            if (a->visibility == vPublic) {
                ifc << "    j=find(json, \"" << a->name <<"\");\n";
                ifc << "    if (j!=0) {\n";
                if (!a->isMultiple) {
                    if (a->Classifier) {
                        ifc << "        newsig->" << a->name << " = to_" << a->Classifier->name << "(j);\n";
                    } else {
                        ifc << "        newsig->" << a->name << " = to_" << a->ClassifierName << "(j);\n";
                    }
                } else {
                    DumpJSONIncomingArray(ifc, a, 8);
                }
                ifc << "    }\n";
            }
        }
    }
    if (mtype == mSyncCall) {
        ifc << "    return (newsig);\n";
    } else if ((mtype == mAsyncCall) || (mtype == mAsyncSignal)) {
        ifc << "    return (newsig);\n";
    }
    ifc << "}\n";
}

void CJSONMessage::DumpJSONIncomingArray(std::ofstream &ifc, std::shared_ptr<CAttribute> a, int space) {
    std::string filler;
    filler.assign(space, ' ');

    ifc << filler << "tJSONArray *array = (tJSONArray*)j;\n";
    ifc << filler << "std::vector<tJSON*>::iterator ai;\n";
    ifc << filler << "for (ai = array->values.begin(); ai != array->values.end(); ++ai) {\n"
                     "}\n";
}

void CJSONMessage::DumpArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first, int space) {
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
        DumpStruct(ifc, a->Classifier, "", runner+"->", false, space);
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

void CJSONMessage::DumpArray(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first, int space) {
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
        DumpStruct(ifc, a->Classifier, "", runner+"->", false, space);
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


void CJSONMessage::DumpStruct(std::ofstream& ifc, std::shared_ptr<MElement> a, std::string sname, std::string prefix, bool first, int space) {
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
        auto  al = s->GetAttributes();

        for (auto & ai : al) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ai);
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                    DumpStruct(ifc, a->Classifier, a->name, a->name+".", ai == *al.begin(), space+4);
                } else {
                    DumpValue(ifc, a, prefix, ai == *al.begin(), space+4);
                }
            } else {
                DumpArray(ifc, a, prefix, ai == *al.begin(), space+4);
            }
        }

        for (auto & ae : s->OtherEnd) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ae);

            if (a->isNavigable()) {
                //
                //  Check for single attribute.
                if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                    if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                        DumpStruct(ifc, a->Classifier, a->name, prefix+a->name+".", ae == *s->OtherEnd.begin(), space+4);
                    } else {
                        DumpValue(ifc, a, prefix, ae == *s->OtherEnd.begin(), space+4);
                    }
                } else {
                    DumpArray(ifc, a, prefix, ae == *s->OtherEnd.begin(), space+4);
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

void CJSONMessage::DumpValue(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first, int space) {
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

void CJSONMessage::DumpValue(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first, int space) {
    std::string filler;

    filler.assign(space, ' ');
    if (a->Classifier->name == "string") {
        if (!first) {
            ifc << filler << "    output <<  \", \\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        } else {
            ifc << filler << "    output <<  \"\\\"" << a->name << "\\\":\\\"\" << helper::escape(" << prefix << a->name << ") << \"\\\"\";\n";
        }
    } else {
        if ((a->Classifier->name == "uint64_t") || (a->Classifier->name == "objectid_t")) {
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


void CJSONMessage::DumpOutgoing(std::ofstream& ifc) {
    std::string prefix;
    std::string tname;

    FillClass();
    if (Class != 0) {
        tname = Class->name;
    } else {
        tname = name;
    }

    if ((mtype == mSyncCall) || (mtype == mReply)) {
        ifc << "// **************************************************************************\n";
        ifc << "//\n";
        ifc << "//  Method-Name   : msg_to_json_" << helper::tolower(name) << "()\n";
        ifc << "//\n";
        ifc << "//  Generated source code.\n";
        ifc << "//\n";
        ifc << "// **************************************************************************\n";
        ifc << "static std::ostream& msg_to_json_" << helper::tolower(name) << "(tMsg* aMsg, std::ostream& output) {\n";
        ifc << "    " << tname << "* msg=("<< tname << "*)aMsg;\n";
        prefix="msg->";
        ifc << "    output << \"\\\"MsgId\\\": \\\"" << name << "\\\"\";\n";
    } else if ((mtype == mAsyncCall) || (mtype == mAsyncSignal)) {
        ifc << "// **************************************************************************\n";
        ifc << "//\n";
        ifc << "//  Method-Name   : sig_to_json_" << helper::tolower(name) << "()\n";
        ifc << "//\n";
        ifc << "//  Generated source code.\n";
        ifc << "//\n";
        ifc << "// **************************************************************************\n";
        ifc << "static std::ostream& sig_to_json_" << helper::tolower(name) << "(tSig* aSig, std::ostream& output) {\n";
        ifc << "    " << tname << "* sig=("<< tname << "*)aSig;\n";
        prefix="sig->";
        ifc << "    output << \"\\\"SignalId\\\": \\\"" << name << "\\\"\";\n";
    }
    if (Class) {
        auto al = std::dynamic_pointer_cast<CClassBase>(Class)->GetAttributes();

        for (auto & ai : al) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ai);
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                    DumpStruct(ifc, a->Classifier, a->name, prefix+a->name+".");
                } else {
                    DumpValue(ifc, a, prefix);
                }
            } else {
                DumpArray(ifc, a, prefix);
            }
        }
        for (auto & ai : Class->OtherEnd) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(*ai);
            //
            //  Check for single attribute.
            if ((a->Multiplicity.empty()) || (a->Multiplicity == "1")) {
                if (a->Classifier && (a->Classifier->type==eElementType::Struct)) {
                    DumpStruct(ifc, a->Classifier, a->name, prefix+a->name+".");
                } else {
                    DumpValue(ifc, a, prefix);
                }
            } else {
                DumpArray(ifc, a, prefix);
            }
        }
    }

    ifc << "    return (output);\n";
    ifc << "}\n";
}
