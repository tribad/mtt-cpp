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

#include <string>

#include "helper.h"

#include "melement.h"
#include "cparameter.h"
#include "moperation.h"
#include "coperation.h"
#include "mattribute.h"
#include "cattribute.h"

#include "massociation.h"
#include "cassociation.h"

#include "massociationend.h"
#include "cassociationend.h"

#include "mdependency.h"
#include "cdependency.h"

#include "cclassbase.h"
#include "ccclass.h"

#include "cmodel.h"

std::string CCClass::FQN() const {
    return name;
}

void CCClass::Prepare(void) {
    PrepareBase();
    CollectAttributes();

    donelist.clear();
}
void CCClass::SetFromTags(const std::string& name, const std::string&value)
{
    if (name == "ExtraInclude") {
        extraheader = value;
    } else {

    }
}

void CCClass::CollectExtraHeader(std::shared_ptr<MElement> e) {
    (void)e;
}


void CCClass::CollectFromBase(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto nc = aNode.mClassifier;

    (void)aHeaderList;
    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(*nc);
        //
        // Check the node classifier.
        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (nc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            CollectNeededModelHeader(nc, neededmodelheader);
        }
        for (auto & p : aNode.mParameter) {
            CollectFromBase(p, neededmodelheader);
        }
    }
}
void CCClass::CollectFromAttribute(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(*nc);
        //
        // Check the node classifier.
        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (nc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            //
            //  Composition.
            if (aNode.mExtension == TypeExtension::None) {
                CollectNeededModelHeader(nc, aHeaderList);
            } else {
                CollectNeededModelHeader(nc, aHeaderList);
            }
        }

        for (auto & p : aNode.mParameter) {
            CollectFromAttribute(p, aHeaderList);
        }
    }
}
void CCClass::CollectFromParameter(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto         nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(*nc);

        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (nc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            CollectNeededModelHeader(nc, neededmodelheader);
        }
        for (auto & p : aNode.mParameter) {
            CollectFromParameter(p, aHeaderList);
        }
    }
}

void CCClass::CollectFromSharedAssoc(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto         nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(*nc);

        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (nc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else if (nc->type == eElementType::SimObject) {
            auto st = MClass::mByFQN.find("tSimObj");

            if (st != MClass::mByFQN.end()) {
                CollectNeededModelHeader(st->second, aHeaderList);
            }
        } else {
            CollectNeededModelHeader(nc, optionalmodelheader);
        }
        for (auto & p : aNode.mParameter) {
            CollectFromSharedAssoc(p, aHeaderList);
        }
    }
}

void CCClass::CollectFromCompositeAssoc(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto         nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(*nc);

        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (nc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            CollectNeededModelHeader(nc, aHeaderList);
        }
        for (auto & p : aNode.mParameter) {
            CollectFromCompositeAssoc(p, aHeaderList);
        }
    }
}


//
//  The next optimization step ahead.
void CCClass::CollectNeededModelHeader(std::shared_ptr<MElement> e, HeaderList& aHeaderList) {
    //
    //  We should work only on class based elements.
    //  And the ones we have not processed so far.
    if ((e->IsClassBased()) && (!aHeaderList.isdone(e))) {
        //
        //  Whatever we do next we need to lockout the element we process through the donelist.
        aHeaderList.done(e);
        //
        //  As we work on class based elements only we can cast the element to the classbase
        auto c = std::dynamic_pointer_cast<CClassBase>(e);
        //
        //  Go to the base-classes
        //  Base classes only can be external or similar or are in the needed list.
        for (auto & bi : c->Base) {
            CollectFromBase(bi.getElement()->mTypeTree, aHeaderList);
        }
        //
        //  Go through all attributes.
        for (auto & a : c->allAttr) {
            //
            //  Different ways to handle the attribute type.
            //  If the attribute type is linked to some classifier we use it.
            if (a->Classifier && (a->Classifier->IsClassBased())) {
                auto ac = std::dynamic_pointer_cast<CClassBase>( *a->Classifier);
                //
                //  Create a list of types from the classifier.
                CollectFromAttribute(ac->mTypeTree, aHeaderList);
                if (a->isMultiple) {
                    if (a->Qualifier.empty()) {
                        auto v = MClass::mByFQN.find("std::vector");

                        if (v != MClass::mByFQN.end()) {
                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(v->second));
                        }
                    } else {
                        auto m = MClass::mByFQN.find("std::map");

                        if (m != MClass::mByFQN.end()) {
                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(m->second));
                        }
                    }
                }
            } else {
                TypeNode temp(TypeNode::parse(a->ClassifierName));

                temp.fill(mNameSpace.getString());
                CollectFromParameter(temp, aHeaderList);
            }
        }


        //
        //  Go through all association ends.
        for (auto & a : c->allEnds) {
            if (a->Classifier->IsClassBased()) {
                auto ac = std::dynamic_pointer_cast<CClassBase>(*a->Classifier);
                //
                //  The specific classes of an association.
                auto assoc   = std::dynamic_pointer_cast<CAssociation>(*a->parent);
                auto otherend        = a;
                auto thisend         = assoc->OtherEnd(a);
                //
                //  Sanity check of otherend and check if navigable.
                if (otherend && otherend->Classifier && (otherend->isNavigable())) {
                    //
                    //  Check if pointer or not.
                    if (thisend && (thisend->Aggregation == aShared)) {
                        CollectFromSharedAssoc(ac->mTypeTree, aHeaderList);
                    } else if (thisend && (thisend->Aggregation == aComposition)) {
                        CollectFromCompositeAssoc(ac->mTypeTree, aHeaderList);
                    }
                    if (!(otherend->Multiplicity.empty()) && (otherend->Multiplicity != "1")) {
                        if (otherend->mQualifier.empty()) {
                            auto v = MClass::mByFQN.find("std::vector");

                            if (v != MClass::mByFQN.end()) {
                                extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(v->second));
                            }
                        } else {
                            auto m = MClass::mByFQN.find("std::map");

                            if (m != MClass::mByFQN.end()) {
                                extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(m->second));
                            }
                        }
                    }
                }
            }
        }

        for (auto & oi : c->Operation) {
            auto operation = std::dynamic_pointer_cast<COperation>(*oi);
            //
            //  Go through the parameters and return values.
            for (auto & pi : operation->Parameter) {
                auto param = std::dynamic_pointer_cast<CParameter>(*pi);

                if (param->Classifier && (param->Classifier->IsClassBased())) {
                    //
                    //  Create a list of types from the classifier.
                    CollectFromParameter(std::dynamic_pointer_cast<CClassBase>(*param->Classifier)->mTypeTree, aHeaderList);
                } else {
                    TypeNode temp(TypeNode::parse(param->ClassifierName));

                    temp.fill(mNameSpace.getString());
                    CollectFromParameter(temp, aHeaderList);
                }
            }
        }
        //
        //  Here starts the exit sequence.
        //  We ad ourselfes into the needed list.
        aHeaderList.add(e);
        //
        // Collect the extraheaders. Ignore the extern classes that have inmodel headers.
        if (!(!c->IsExternInModel()) && (!c->GetExtraHeader().empty())) {
            extramodelheader.push_back(c);
        }
        //
        //  If we are on the way out the tree we may add the dependencies.
        if (e.get() == this) {
            //
            // Collect the extraheaders. Ignore the extern classes that have inmodel headers.
            if ((!IsExternInModel()) && (!GetExtraHeader().empty())) {
                extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(MElement::Instances[id]));
            }
            for (auto & i : Supplier) {
                if (i.getElement()->IsClassBased()) {
                    if ((i.getElement()->type==eElementType::ExternClass) || (i.getElement()->type==eElementType::QtClass)) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(i.getElement()));
                    } else {
                        CollectNeededModelHeader(i.getElement(), optionalmodelheader);
                    }
                }
            }
        }
    }
}

void CCClass::DumpOperationDecl(std::ostream& hdr) {
    bool dump=true;

    for (auto & mo : Operation) {
        if (mo->visibility == vPublic) {
            if (dump) {
                dump=false;
            }
            auto op = std::dynamic_pointer_cast<COperation>(*mo);

            std::string pdecl=op->GetParameterDecl(NameSpace());
            hdr << op->GetReturnType(NameSpace()) << " ";
            hdr << op->name << "(" <<  pdecl << ") ;\n";
        }
    }
}

void CCClass::DumpAttributeDecl(std::ostream& hdr) {
    bool dump = true;

    for (auto & ma : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

        if ((ma->visibility == vPublic)  && (!a->isStatic)) {
            if (dump) {
                dump=false;
            }
            std::string cname;

            if (a->Classifier) {
                cname = a->Classifier->FQN();
            } else {
                cname = a->ClassifierName;
            }

            if (!cname.empty()) {
                hdr << "extern ";
                hdr << cname << " " << a->name;
                if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                    hdr << ";\n";
                } else {
                    if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {

                    } else {
                        hdr << "[" << a->Multiplicity << "];\n";
                    }

                }
            }
        }
    }
    //
    //  Go through the associations
    for (auto & me : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*me);
        //
        //  Only use public navigable ends.
        if ((me->visibility == vPublic) && a->isNavigable() ){
            if (dump) {
                dump=false;
            }
            hdr << "extern ";

            std::string cname=a->Classifier->name;
            auto assoc = std::dynamic_pointer_cast<CAssociation>(*a->parent);

            if (assoc->ends[0] == me) {
                if (std::dynamic_pointer_cast<CAssociationEnd>(*assoc->ends[1])->Aggregation == aShared) {
                    cname.push_back('*');
                }
            } else {
                if (std::dynamic_pointer_cast<CAssociationEnd>(*assoc->ends[0])->Aggregation == aShared) {
                    cname.push_back('*');
                }
            }

            if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                hdr << cname << " " << a->name << ";\n";
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    hdr << "std::vector<" << cname << "> " << a->name << ";\n";
                    //
                    //  We expect that the stl is defined somewhere and search the vector definition.
                    auto vec = MClass::mByFQN.find("std::vector");
                    if (vec != MClass::mByFQN.end()) {
                        optionalmodelheader.add(vec->second);
                    }
                } else {
                    hdr << cname << " " << a->name << "[" << a->Multiplicity << "];\n";
                }
            }
        }
    }
}
void CCClass::DumpAttributeDefinition(std::ostream& src, eVisibility vis) {
    bool dump = true;

    for (auto & ma : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

        if (ma->visibility == vis) {
            if (dump) {
                dump=false;
            }
            std::string cname;

            if (a->Classifier) {
                cname = a->Classifier->FQN();
            } else {
                cname = a->ClassifierName;
            }

            if (!cname.empty()) {
                if ((vis == vPrivate) || (a->isStatic)) {
                    src << "static ";
                }
                src << cname << " " << a->name;
                if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                    if (!a->defaultValue.empty()) {
                        src << " = " << a->defaultValue;
                    }
                    src << ";\n";
                } else {
                    if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {

                    } else {
                        src << "[" << a->Multiplicity << "];\n";
                    }

                }
            }
        }
    }

    for (auto & me : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*me);

        if (((*me)->visibility == vis) && a->isNavigable()){
            std::string cname=a->Classifier->name;

            if (dump) {
                dump=false;
            }
            if (vis == vPrivate) {
                src << "static ";
            }
            auto assoc = std::dynamic_pointer_cast<CAssociation>(*a->parent);

            if (assoc->ends[0] == me) {
                if (std::dynamic_pointer_cast<CAssociationEnd>(*assoc->ends[1])->Aggregation == aShared) {
                    cname.push_back('*');
                }
            } else {
                if (std::dynamic_pointer_cast<CAssociationEnd>(*assoc->ends[0])->Aggregation == aShared) {
                    cname.push_back('*');
                }
            }

            if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                src << cname << " " << a->name;
                if (!a->defaultValue.empty()) {
                    src << " = " << a->defaultValue;
                }
                src << ";\n";
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    src << "std::vector<" << cname << "> " << a->name << ";\n";
                } else {
                    src << cname << " " << a->name << "[" << a->Multiplicity << "];\n";
                }

            }
        }
    }
}

void CCClass::DumpOperationDefinition(std::ostream &src) {
    std::string rettype;

    for (auto & mo : Operation) {
        if (mo->visibility == vPrivate) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);
            //
            //  Dump any comment before the function prototype.
            mo->DumpComment(src, 0, 130);
            std::string pdef=op->GetParameterDefinition(NameSpace());

            rettype=op->GetReturnType(NameSpace());
            if (rettype.at(rettype.size()-1)== '&') {
                rettype=rettype.substr(0, rettype.size()-1);
            }
            src << "static " << rettype << " ";
            if (pdef == "void") {
                src << op->name << "() {\n";
            } else {
                src << op->name << "(" << pdef << ") {\n";
            }
            if ((op->GetReturnType(NameSpace()) != "void") && (op->GetReturnType(NameSpace()) == rettype)) {
                src << "    " << rettype << " " << op->GetReturnName();
                if (!op->GetReturnDefault().empty()) {
                    src << " = " << op->GetReturnDefault();
                }
                src << ";\n";
            }
            src << "// User-Defined-Code:" << op->id << "\n";
            src << "// End-Of-UDC:" << op->id << "\n";
            if ((op->GetReturnType(NameSpace()) != "void") && (op->GetReturnType(NameSpace()) == rettype)) {
                src << "    return  ("<< op->GetReturnName() << ");\n";
            }
            src << "}\n\n";
        }
    }
    for (auto & mo : Operation) {
        if ((mo->visibility == vProtected) || (mo->visibility == vPublic)) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);
            //
            //  Dump any comment before the function prototype.
            mo->DumpComment(src, 0, 130);
            std::string pdef=op->GetParameterDefinition(NameSpace());

            rettype=op->GetReturnType(NameSpace());
            if (rettype.at(rettype.size()-1)== '&') {
                rettype=rettype.substr(0, rettype.size()-1);
            }

            src << rettype << " ";

            if (pdef == "void") {
                src << op->name << "() {\n";
            } else {
                src << op->name << "(" << pdef << ") {\n";
            }
            if ((op->GetReturnType(NameSpace()) != "void") && (op->GetReturnType(NameSpace()) == rettype)) {
                src << "    " << rettype << " " << op->GetReturnName();
                if (!op->GetReturnDefault().empty()) {
                    src << " = " << op->GetReturnDefault();
                }
                src << ";\n";
            }
            src << "// User-Defined-Code:" << op->id << "\n";
            src << "// End-Of-UDC:" << op->id << "\n";
            if ((op->GetReturnType(NameSpace()) != "void") && (op->GetReturnType(NameSpace()) == rettype)) {
                src << "    return  ("<< op->GetReturnName() << ");\n";
            }
            src << "}\n\n";
        }
    }
}


void CCClass::Dump(std::shared_ptr<MModel> model) {
    std::set<std::shared_ptr<MElement>> oplist;
    bool                                dump;
    auto thisbase = std::dynamic_pointer_cast<CClassBase>(MClass::Instances[id]);
    //
    //  Collect all headers needed to dump the files.
    CollectNeededModelHeader(thisbase, neededmodelheader);

    DumpBase(std::dynamic_pointer_cast<CModel>(model), name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, name);
    DumpPublicMacros(hdr);
    DumpFileHeader(src, name, ".cpp");
    donelist.clear();
    //
    //  Create forwards from list of optional headers.
    dump = true;
    for (auto & i : optionalmodelheader) {
        //
        //  Forwards are only valid for classes.
        if (i->type == eElementType::CxxClass) {
            //
            //  We do not generate forwards for ourselfes.
            if (i.get() != this) {
                //
                //  Check if name is in done list. Only if not in the donelist.
                //  we create a forward.
                if (donelist.find(i->name) == donelist.end()) {
                    if (dump) {
                        dump = false;
                        hdr << "//\n"
                               "//  List of forwards needed in this module.\n";
                    }
                    //
                    //  Output the forward.
                    hdr << "class " << i->name << ";\n";
                    //
                    //  Add name to donelist to prevent us from doing things twice.
                    donelist.insert(i->name);
                }
            }
        }
    }
    DumpAttributeDecl(hdr);
    DumpOperationDecl(hdr);
    hdr << "\n";
    DumpGuardTail(hdr, name);

    donelist.clear();
    DumpExtraIncludes(src, donelist, oplist);


    donelist.clear();

    DumpNeededIncludes(src, thisbase, donelist, oplist);
    src << "// Optional\n";

    oplist.clear();
    DumpOptionalIncludes(src, thisbase, donelist, oplist);

    DumpPrivateMacros(src);

    DumpAttributeDefinition(src, vPrivate);
    DumpAttributeDefinition(src);
    DumpOperationDefinition(src);
    CloseStreams();
}
