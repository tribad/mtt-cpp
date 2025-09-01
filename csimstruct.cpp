//
// Copyright 2017 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include <iostream>
#include "helper.h"
#include "mattribute.h"
#include "cattribute.h"
#include "massociationend.h"
#include "cassociationend.h"
#include "cclassbase.h"
#include "csimstruct.h"

#include "mmodel.h"
#include "cmodel.h"

void CSimStruct::SetFromTags(const std::string& name, const std::string&value)
{
}

std::string CSimStruct::FQN() const {
    return name;
}

void CSimStruct::Prepare(void) {
    PrepareBase();
    lower_name=helper::tolower(name);
    upper_name=helper::toupper(name);

}

void CSimStruct::Dump(std::shared_ptr<MModel> model) {
    std::list<std::pair<std::string, std::string> >           alist;
    std::list<std::pair<std::string, std::string> >::iterator ilist;
    size_t                                                    alistmax=0;

    DumpBase(std::dynamic_pointer_cast<CModel>(model), name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, name);

    for (auto & i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string fqn=a->FQN();

        if ((a->ClassifierName.empty()) && (a->visibility==vPublic)) {
            hdr << "#define " << a->name << " " << a->defaultValue << "\n";
        }
    }

    hdr << "//\n";
    hdr << "//                   S t r u c t    d e c l a r a t i o n\n";
    hdr << "typedef struct __" << name << " {\n";

    auto al=GetAttributes();

    for (auto & i : al) {
        auto a = std::dynamic_pointer_cast<CAttribute>(i);
        std::string fqn=a->FQN();

        if (!a->name.empty()) {
            if (!(a->ClassifierName.empty())) {
                if (fqn == "__simobject__") {
                    fqn = (a->Aggregation == aShared)?"tSimObj*":"tSimObjRef";
                } else {
                }
                if (a->isReadOnly) {
                    fqn = "const " + fqn;
                }
                if (a->Multiplicity == "1") {
                    alist.emplace_back(fqn, a->name);
                } else if (a->Multiplicity == "0..1") {
                    alist.emplace_back(fqn+"*", a->name);
                } else if (a->Multiplicity == "0..*") {
                    if (a->QualifierType.empty()) {
                        alist.emplace_back(std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back(std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity == "1..*") {
                    if (a->QualifierType.empty()) {
                        alist.emplace_back(std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back(std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity == "*") {
                    if (a->QualifierType.empty()) {
                        alist.emplace_back(std::string("std::vector< ")+fqn+" >", a->name);
                    } else {
                        alist.emplace_back(std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                    }
                } else if (a->Multiplicity.size() == 0) {
                    if (a->defaultValue.size() > 0) {
                        alist.emplace_back(fqn, a->name+" = " + a->defaultValue);
                    } else {
                        alist.emplace_back(fqn, a->name);
                    }
                } else {
                    alist.emplace_back(fqn, a->name+"["+a->Multiplicity+"]");
                }
            }
        }
    }
    //
    //  Processing the aggregations/compositions that are navigable.
    for (auto & i : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);

        if (a->isNavigable()) {
            std::string fqn=a->FQN();

            for (ilist=alist.begin(); ilist!=alist.end(); ++ilist) {
                if (ilist->second == a->name) {
                    break;
                }
            }
            if (ilist == alist.end()) {
                if (!a->name.empty()) {
                    if (fqn == "__simobject__") {
                        fqn = (a->Aggregation == aShared)?"tSimObj*":"tSimObjRef";
                    } else {
                    }
                    if (a->Multiplicity == "1") {
                        alist.emplace_back(fqn, a->name);
                    } else if (a->Multiplicity == "0..1") {
                        alist.emplace_back(fqn+"*", a->name);
                    } else if (a->Multiplicity == "0..*") {
                        if (a->QualifierType.empty()) {
                            alist.emplace_back(std::string("std::vector< ")+fqn+" >", a->name);
                        } else {
                            alist.emplace_back(std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                        }
                    } else if (a->Multiplicity == "1..*") {
                        if (a->QualifierType.empty()) {
                            alist.emplace_back(std::string("std::vector< ")+fqn+" >", a->name);
                        } else {
                            alist.emplace_back(std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                        }
                    } else if (a->Multiplicity == "*") {
                        if (a->QualifierType.empty()) {
                            alist.emplace_back(std::string("std::vector< ")+fqn+" >", a->name);
                        } else {
                            alist.emplace_back(std::string("std::map< ") + a->QualifierType + ", "+fqn+" >", a->name);
                        }
                    } else if (a->Multiplicity.size() == 0) {
                        alist.emplace_back(fqn, a->name);
                    } else {
                        alist.emplace_back(fqn+"["+a->Multiplicity+"]", a->name);
                    }
                }
            }
        }
    }
    for (ilist=alist.begin(); ilist != alist.end(); ++ilist) {
        if (ilist->first.size()>alistmax) {
            alistmax=ilist->first.size();
        }
    }
    for (ilist=alist.begin(); ilist != alist.end(); ++ilist) {
        std::string filler;
        filler.assign(alistmax-ilist->first.size()+1, ' ');
        hdr << "    " << ilist->first << filler << ilist->second << ";\n";
    }
    hdr << "} " << name << ";\n";


    DumpGuardTail(hdr, name);
    CloseStreams();
}
