//
// Copyright 2024 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#include <sstream>
#include "path.h"
#include "moperation.h"
#include "coperation.h"
#include "mattribute.h"
#include "cattribute.h"
#include "cparameter.h"

#include "massociationend.h"
#include "cassociationend.h"

#include "massociation.h"
#include "cassociation.h"

#include "mconnector.h"
#include "cconnector.h"

#include "mdependency.h"
#include "cdependency.h"

#include "mgeneralization.h"
#include "cgeneralization.h"

#include "cenumeration.h"

#include "cclassbase.h"
#include "cmoduleclass.h"

#include "cprimitivetype.h"

#include "cmodel.h"

#include "namespacenode.h"

#include "msimmessage.h"
#include "csimmessage.h"
#include "cmessageclass.h"

#include "mjsonmessage.h"
#include "cjsonmessage.h"

#include "msimsignal.h"
#include "csimsignal.h"
#include "csignalclass.h"

#include "helper.h"

#include "medge.h"
#include "mpin.h"
#include "mactionnode.h"
#include "mactivity.h"
#include "cactivity.h"

#include "mmessage.h"
#include "cmessage.h"

#include "mlifeline.h"
#include "clifeline.h"

#include "minteraction.h"
#include "cinteraction.h"

#include "ccollaboration.h"

#include "cpackagebase.h"
#include "crequirement.h"

#include "main.h"

extern std::vector<std::pair<eVisibility, std::string> > vis;

#if 0

#define CTOR_MASK        0x01
#define COPY_CTOR_MASK   0x02
#define MOVE_CTOR_MASK   0x04
#define COPY_ASSIGN_MASK 0x08
#define MOVE_ASSIGN_MASK 0x10
#define DTOR_MASK        0x80

bool dumpHelp = false;

static const char* cardinaltypes[] = {"int",
                                      "long",
                                      "char",
                                      "void*",
                                      "unsigned",
                                      "unsigned long",
                                      "long long",
                                      "unsigned char",
                                      "float",
                                      "double",
                                      "long double",
                                      "short",
                                      "unsigned short",
                                      0
                                     };

std::vector<std::pair<eVisibility, std::string> > vis = {{vPublic, "public"},
                                                         {vProtected, "protected"},
                                                         {vPrivate, "private"}
};

const size_t kVisSize = 3U;

std::string CCxxClass::mapVisibility(eVisibility a_vis) {
    for (auto & v : vis) {
        if (v.first == a_vis) {
            return v.second;
        }
    }
    return std::string();
}

bool CCxxClass::IsCardinalType(const std::string& aName) {
    const char **s = cardinaltypes;

    while ((*s != nullptr) && (aName != *s)) {
        s++;
    }
    if (*s != nullptr) {
        return true;
    }
    return false;
}

#endif

std::string CModuleClass::FQN() const {
    return mTypeTree.getFQN();
}

void CModuleClass::Prepare(void) {
    if (!PrepDone) {
        mTypeTree = TypeNode::parse(name);
        PrepareBase();
        if (HasTaggedValue("Serialize")) {
            std::string ser = helper::tolower(GetTaggedValue("Serialize"));
            if (ser == "true") {
                mSerialize = true;
            }
            else {
            }
        }
        if (HasTaggedValue("ByteOrder")) {
            std::string bo = helper::tolower(GetTaggedValue("ByteOrder"));
            if (bo == "network") {
                mByteOrder = eByteOrder::Network;
            }
            else {
            }
        }
        //
        //  To have a more complete view on the attributes we create a list of attributes
        //  available in this class.
        CollectAttributes();
        donelist.clear();
        PrepDone = true;
        //
        //  Prepare attached collaborations.
        for (auto& col : mCollaboration) {
            if (col->IsClassBased()) {
                col->Prepare();
            }
        }
        //
        //  Need to sort operations and attributes.
        std::multimap<int64_t, std::shared_ptr<MOperation>> sorting;

        for (auto & o : Operation) {
            sorting.insert(std::pair<int64_t, std::shared_ptr<MOperation>>(o->mPosition, std::dynamic_pointer_cast<MOperation>(*o)));
        }
        //
        //  All operations have been moved to the temporary sorting map.
        Operation.clear();

        for (auto & o : sorting) {
            Operation.emplace_back(o.second->id);
        }
        std::multimap<int64_t, std::shared_ptr<MAttribute>> asorting;

        for (auto & a : Attribute) {
            asorting.insert(std::pair<int64_t, std::shared_ptr<MAttribute>>(a->mPosition, std::dynamic_pointer_cast<MAttribute>(*a)));
        }
        Attribute.clear();
        for (auto & a : asorting) {
            Attribute.emplace_back(a.second->id);
        }
        auto enclosed = getEnclosedClasses();
        for (auto& e : enclosed) {
            e->Prepare();
        }
    }
}

void CModuleClass::SetFromTags(const std::string& name, const std::string&value)
{
}

#if 0

void CCxxClass::FillInCollection(std::shared_ptr<MElement> e) {
    bool putExtra = false;

    if (e->type == eElementType::ExternClass) {
        mSelfContainedExtras.push_back(e);
        putExtra = true;
    } else if (e->type == eElementType::QtClass) {
        mSelfContainedQt.push_back(e);
    } else if (e->type == eElementType::PrimitiveType) {
        //std::cerr << "Primitive Type:" << e->name << std::endl;
        //
        // Three ways to handle a primitive type
        //
        //  1. It has a dependency to another classifier.
        //  2. It has a base classifier
        //  3. We find the real class by name
        //
        //  First simple approach. Always use the name to match.
        if (e->name.find_first_of(":.<>&*") != std::string::npos) {
            FillInCollection(e->name);
        }
    } else {
        mSelfContainedHeaders.push_back(e);
    }
    //
    // If we did not put the element into the extra list but it has an extra include defined we do it
    // here.
    if ((e->HasTaggedValue("ExtraInclude")) && (!putExtra)) {
        std::string extra = e->GetTaggedValue("ExtraInclude");

        if (!extra.empty()) {
            mSelfContainedExtras.push_back(e);
        }
    }
}

void CCxxClass::FillInCollection(const std::string &aTextType) {
    //
    //  Split the un-classified type into several subtypes. Catching templates as well.
    auto tlist=helper::typelist(aTextType);

    for (auto & tli : tlist) {
        //
        //  Check that the type is no pointer types.
        //  Pointer types are optional and can generate forwards.
        if (tli.second.find('*') == std::string::npos) {
            size_t fpos;
            //
            //  As we know from the previous check that the name has no star in it
            //  We only need to remove the &
            while ((fpos = tli.second.find_first_of("&")) != std::string::npos) tli.second.erase(fpos, 1);
            //
            //  "Not a pointer" handling
            auto ci = MClass::mByFQN.find(tli.second);

            if (ci != MClass::mByFQN.end()) {
                if (ci->second->IsClassBased()) {
                    FillInCollection(ci->second);
                }
            }
        } else {

        }
    }  //  end of for-loop over typenames.
}
//
//  Self contained header are not recursive.
void CCxxClass::CollectSelfContainedHeader() {
    //
    //  Go to the base-classes
    //  Base classes only can be external or similar or are in the needed list.
    for (auto & bi : Base) {
        FillInCollection(bi.getElement());
    }
    //
    //  Go along the attributes.
    for (auto & a : allAttr) {
        //
        //  Different ways to handle the attribute type.
        //  If the attribute type is linked to some classifier we use it.
        if (a->Classifier != nullptr) {
            //
            //  If the multiplicity is '*' we add the vector definition to the extramodelheaders.
            if (a->Multiplicity == "*") {
                std::string container = "std::vector";
                if (!a->QualifierType.empty()) {
                    container = "std::map";
                }
                auto ci = MClass::mByFQN.find(container);
                //
                //  Check if we found the default collection class.
                if (ci != MClass::mByFQN.end()) {
                    mSelfContainedExtras.push_back(ci->second);
                }
            }
            //
            //  We have a look on the aggregation type of the attribute.
            //  Shared attributes are realized through pointers. And pointers only need the class forward.
            if ((a->Aggregation == aNone) || (a->Aggregation == aComposition)) {
                FillInCollection(a->Classifier.ptr());
            } else {
            }
        } else {
        }
    }  //  end of foo-loop over attributes.
    //
    //  Go through all association ends.
    for (auto & a : allEnds) {
        //
        //  The specific classes of an association.
        std::shared_ptr<CAssociation>    assoc   = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr());
        std::shared_ptr<CAssociationEnd> otherend = a;
        std::shared_ptr<CAssociationEnd> thisend = std::dynamic_pointer_cast<CAssociationEnd>(assoc->OtherEnd(otherend));
        //
        //  Sanity check of otherend and check if navigable.
        if (otherend && otherend->Classifier && (otherend->isNavigable())) {
            //
            //  Check if pointer or not.
            if (thisend && (thisend->Aggregation == aShared)) {
            } else if ((thisend != 0) && (thisend->Aggregation == aComposition)) {
                FillInCollection(a->Classifier.ptr());
            }
            //
            //  As the otherend defines the properties of the attribute to generate we go-on
            //  with more specials.
            //
            //  Use the standard collection class if multiplicity is '*'
            if (otherend->Multiplicity == "*") {
                //
                //  For the container name to prevent us from do it differently in the various places.
                std::string container;
                //
                //  Only if we have not Qualifier defined we use vector as the collection class.
                if (!otherend->mQualifier.empty()) {
                    //
                    //  We need map as we have a qualifier defined.
                    container = "std::map";
                    //
                    // The qualifier type is not empty. So we need to split the text to the types involved.
                    if (!otherend->QualifierType.empty()) {
                        FillInCollection(otherend->QualifierType);
                    }
                } else {

                    if (assoc->HasTaggedValue("Container")) {
                        container = assoc->GetTaggedValue("Container");
                        if (container.empty()) {
                            container = "std::vector";
                        } else {
                        }
                    } else {
                        container ="std::vector";
                    }
                }
                auto ci = MClass::mByFQN.find(container);
                //
                //  Check if we found the default collection class.
                if (ci != MClass::mByFQN.end()) {
                    FillInCollection(ci->second);
                }
            }
        }
    }
    //
    //  Go through the operations.
    for (auto & oi : Operation) {
        //
        //  Go through the parameters and return values.
        for (auto & pi : std::dynamic_pointer_cast<COperation>(oi.ptr())->Parameter) {
            std::shared_ptr<CParameter> param = std::dynamic_pointer_cast<CParameter>(pi.ptr());

            if (param->Classifier) {
                //
                //  If we find any star at the classifier name we have a pointer type
                if (param->ClassifierName.find('*') == std::string::npos) {
                    FillInCollection(param->Classifier.ptr());
                }
            } else {
                FillInCollection(param->ClassifierName);
            }
        }
    }

}

void CCxxClass::CollectFromBase(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto nc = aNode.mClassifier;

    (void)aHeaderList;
    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(nc.ptr());
        //
        // Check the node classifier.
        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (ncc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            CollectNeededModelHeader(ncc, neededmodelheader);
        }
        for (auto & p : aNode.mParameter) {
            CollectFromBase(p, neededmodelheader);
        }
    }
}
void CCxxClass::CollectFromAttribute(const TypeNode& aNode, HeaderList& aHeaderList, bool aIsTemplateReferences) {
    auto nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(nc.ptr());
        //
        // Check the node classifier.
        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (nc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            //
            //  Composition.
            if (aNode.mExtension == TypeExtension::None) {
                CollectNeededModelHeader(ncc, aHeaderList);
            } else {
                CollectNeededModelHeader(ncc, optionalmodelheader);
            }
        }
    }
    for (auto & p : aNode.mParameter) {
        CollectFromAttribute(p, aHeaderList);
    }
}
void CCxxClass::CollectFromParameter(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto         nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(nc.ptr());

        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (ncc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            //CollectNeededModelHeader(ncc, neededmodelheader);
            if (aNode.mExtension == TypeExtension::None) {
                CollectNeededModelHeader(ncc, aHeaderList);
            } else {
                CollectNeededModelHeader(ncc, optionalmodelheader);
            }        
        }
    }
    for (auto & p : aNode.mParameter) {
        CollectFromParameter(p, aHeaderList);
    }
}

void CCxxClass::CollectFromSharedAssoc(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto         nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(nc.ptr());

        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (ncc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else if (nc->type == eElementType::SimObject) {
            auto st = MClass::mByFQN.find("tSimObj");

            if (st != MClass::mByFQN.end()) {
                CollectNeededModelHeader(st->second, aHeaderList);
            }
        } else {
            CollectNeededModelHeader(ncc, optionalmodelheader);
        }
    }
    for (auto & p : aNode.mParameter) {
        CollectFromSharedAssoc(p, aHeaderList);
    }
}

void CCxxClass::CollectFromCompositeAssoc(const TypeNode& aNode, HeaderList& aHeaderList) {
    auto         nc = aNode.mClassifier;

    if (nc) {
        auto ncc = std::dynamic_pointer_cast<CClassBase>(nc.ptr());

        if ((ncc->IsExternClass() && (!ncc->IsExternInModel())) || (ncc->type == eElementType::QtClass)) {
            extramodelheader.push_back(ncc);
        } else {
            CollectNeededModelHeader(ncc, aHeaderList);
        }
    }
    for (auto & p : aNode.mParameter) {
        CollectFromCompositeAssoc(p, aHeaderList);
    }
}

void CCxxClass::CollectFromTemplateBinding(std::shared_ptr<CClassBase> aClass, HeaderList &aHeaderList) {
    //
    //  Go through the generalizations and find template binding parameter.
    for (auto & g : aClass->Generalization) {
        auto generalization = std::dynamic_pointer_cast<MGeneralization>(*g);

        for (auto &bp : generalization->mTemplateParameter) {
            auto parameter = std::dynamic_pointer_cast<MParameter>(*(bp.second));
            //
            //  Different ways to get the classifier/name for EA and StarUML
            if (!parameter->mActual) {
                if (parameter->Classifier) {
                    auto c = std::dynamic_pointer_cast<CClassBase>(*parameter->Classifier);
                    CollectFromParameter(c->mTypeTree, aHeaderList);
                } else {
                    if (!parameter->ClassifierName.empty()) {
                        TypeNode temp(TypeNode::parse(parameter->ClassifierName));

                        temp.fill(mNameSpace.getString());
                        CollectFromParameter(temp, aHeaderList);
                    }
                }
            } else {
                if (parameter->mActual->IsClassBased()) {
                    auto cb = std::dynamic_pointer_cast<CClassBase>(*parameter->mActual);

                    CollectFromParameter(cb->mTypeTree, aHeaderList);
                } else {
                    // ignoring classbased without classifier set
                }
            }
        }
    }
}

//
//  The next optimization step ahead.
void CCxxClass::CollectNeededModelHeader(std::shared_ptr<MElement> e, HeaderList& aHeaderList) {
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
        // Collect dependencies from the template binding parameter.
        CollectFromTemplateBinding(c, aHeaderList);
        //
        //  Go to the base-classes
        //  Base classes only can be external or similar or are in the needed list.
        for (auto & bi : c->Base) {
            CollectFromBase(bi.getElement()->mTypeTree, aHeaderList);
        }
        //
        //  Check for alias/use dependency
        for (auto & i : c->Supplier) {
            if (i.getConnector()->HasStereotype("use")) {
                if ((i.getElement()->type == eElementType::ExternClass) || (i.getElement()->type == eElementType::QtClass)) {
                    extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(i.getElement()));
                } else {
                    CollectNeededModelHeader(i.getElement(), aHeaderList);
                }
            }
        }
        //
        //  Go to the enclosed classes. They are the same as the class itself.
        auto enclosed = c->getEnclosedClasses();
        for (auto& en : enclosed) {
            CollectNeededModelHeader(en, aHeaderList);
        }
        //
        //  Go through all attributes.
        for (auto & a : c->allAttr) {
            //
            //  Different ways to handle the attribute type.
            //  If the attribute type is linked to some classifier we use it.
            if ((a->Classifier != nullptr) && (a->Classifier->IsClassBased())) {
                auto ac = std::dynamic_pointer_cast<CClassBase>(*a->Classifier);
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
            if (a->Classifier && a->Classifier->IsClassBased()) {
                auto ac = std::dynamic_pointer_cast<CClassBase> (a->Classifier.ptr());
                //
                //  The specific classes of an association.
                std::shared_ptr<CAssociation>    assoc    = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr());
                std::shared_ptr<CAssociationEnd> otherend = a;
                std::shared_ptr<CAssociationEnd> thisend  = std::dynamic_pointer_cast<CAssociationEnd>(assoc->OtherEnd(otherend));
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
            //
            //  Go through the parameters and return values.
            for (auto & pi : std::dynamic_pointer_cast<COperation>(oi.ptr())->Parameter) {
                std::shared_ptr<CParameter> param = std::dynamic_pointer_cast<CParameter>(pi.ptr());

                auto pc = std::dynamic_pointer_cast<CClassBase>(param->Classifier.ptr());

                if (pc && (pc->IsClassBased())) {
                    //
                    //  Create a list of types from the classifier.
                    CollectFromParameter(pc->mTypeTree, aHeaderList);
                } else {
                    TypeNode temp(TypeNode::parse(param->ClassifierName));

                    temp.fill(mNameSpace.getString());
                    CollectFromParameter(temp, aHeaderList);
                }
            }
        }
        //
        //  Here starts the exit sequence.
        //  We add ourselfes into the needed list if the class is not enclosed.
        //  If the parent is a classbased element, we are enclosed and must not be added.
        aHeaderList.add(c->getContainerClass());
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
                extramodelheader.push_back(shared_this());
            }
            for (auto & i : Supplier) {
                if (i.getElement()->IsClassBased() && !(i.getConnector()->HasStereotype("use"))) {
                    if ((i.getElement()->type == eElementType::ExternClass) || (i.getElement()->type == eElementType::QtClass)) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(i.getElement()));
                    } else {
                        CollectNeededModelHeader(i.getElement(), optionalmodelheader);
                    }
                } else {
                }
            }
        }
    }
}

void CCxxClass::CollectForwards() {
    donelist.clear();
    //
    //  Go through all attributes.
    for (auto & a : allAttr) {
        //
        //  Different ways to handle the attribute type.
        //  If the attribute type is linked to some classifier we use it.
        if ((a->Classifier != nullptr) && (a->Classifier->IsClassBased())) {
            auto ac = std::dynamic_pointer_cast<CClassBase>(*a->Classifier);
            //
            //  Create a list of types from the classifier.
            CollectForwardRefs(ac);
        }  else {
            TypeNode tmp(TypeNode::parse(a->ClassifierName));

            tmp.fill(mNameSpace.getString());

            auto cil = CClassBase::getRefTypes(tmp);

            for (auto & cili : cil) {
                if (!(cili->IsExternClass()) && (cili->type != eElementType::QtClass) && (cili.get() != this) &&
                        (cili->mClassParameter.empty())) {
                    forwards.insert(cili);
                }
            }
        }
    }
    //
    //  Go through all association ends.
    for (auto & a : allEnds) {
        if (a && a->parent && (a->isNavigable())) {
            std::shared_ptr<CAssociation>    assoc   = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr());
            std::shared_ptr<CAssociationEnd> thisend = std::dynamic_pointer_cast<CAssociationEnd>(assoc->OtherEnd(a));

            if (thisend->Aggregation == enumAggregation::aShared) {
                auto ac = std::dynamic_pointer_cast<CClassBase>(a->Classifier.ptr());

                if (ac) {
                    CollectForwards(ac);
                }
            }
        }
    }

    for (auto & oi : Operation) {
        //
        //  Go through the parameters and return values.
        for (auto & pi : std::dynamic_pointer_cast<COperation>(oi.ptr())->Parameter) {
            auto param = std::dynamic_pointer_cast<CParameter>(pi.ptr());

            if (param->Classifier && (param->Classifier->IsClassBased())) {
                auto pc = std::dynamic_pointer_cast<CClassBase>(param->Classifier.ptr());

                CollectForwardRefs(pc);
            } else {
                TypeNode tmp(TypeNode::parse(param->ClassifierName));

                tmp.fill(mNameSpace.getString());

                auto cil = CClassBase::getRefTypes(tmp);

                for (auto & cili : cil) {
                    if (!(cili->IsExternClass()) && (cili->type != eElementType::QtClass) && (cili.get() != this)) {
                        forwards.insert(cili);
                    }
                }
            }
        }
    }
}
//
//  This operation dumps all operation declarations that have package scope.
//  These are operations that are like simple C-style functions .
//  They get defined outside a class.
void CCxxClass::DumpPackageOperationDecl(std::ostream& hdr) {
    bool                               dump = true;

    for (auto & mo : Operation) {
        std::shared_ptr<COperation> op = std::dynamic_pointer_cast<COperation>(mo.ptr());

        if ((mo->visibility == vPackage) && (!op->isStatic)) {
            std::string pdecl = op->GetParameterDecl(mNameSpace);
            if (dump) {
                dump = false;
                hdr << "//\n//  These are the operations defined with package scope.\n";
            }
            if (op->HasStereotype("CLinkage")) {
                hdr << "extern \"C\" ";
            }
            if ((op->name != name) && (op->name != std::string("~")+name)) {
                hdr << op->GetReturnType(mNameSpace) << " ";
                hdr << op->name << "(" <<  pdecl << ") ;\n";
            } else {
                if (pdecl == "void") {
                    hdr << op->name << "() ;\n";
                } else {
                    hdr << op->name << "(" <<  pdecl << ") ;\n";
                }
            }
        }
    }
}

void CCxxClass::DumpOperationDecl(std::ostream& hdr, int indent) {
    bool dump=true;
    std::string classfiller;
    classfiller.assign(indent, ' ');
    //
    //  First we dump all "normal" operations.
    for (auto & vi : vis) {
        dump=true;
        //
        //  This block adds the missing to match the rule of five.
        uint8_t oneoffive = getOneOfFive();
        //
        //  Rule of five only apply to classes not to structs.
        if ((vi.first == vPublic) && (type == eElementType::CxxClass)) {
            //
            //  if we have there is any virtual method defined we need a virtual destructor.

            if (((oneoffive & DTOR_MASK) == 0) && HasAnyVirtuals()) {
                //
                //  First we simulate the existence of a DTOR)
                oneoffive |= DTOR_MASK;
            }
            if ((oneoffive != 0U) && (gRuleOfFive)) {
                //
                //  Check the flags and create the missing.
                if ((oneoffive & CTOR_MASK) == 0) {
                    if (dump) {
                        hdr << classfiller << "public:\n";
                        dump = false;
                    }
                    hdr << classfiller << "    " << name << "() = default;\n";
                }
                if ((oneoffive & COPY_CTOR_MASK) == 0) {
                    if (dump) {
                        hdr << classfiller << "public:\n";
                        dump = false;
                    }
                    hdr << classfiller << "    " << name << "(";
                    if (!gConstInArgumentRight) {
                        hdr << "const " << name << "& aOther) = default;\n";
                    } else {
                        hdr << name << " const& aOther) = default;\n";
                    }
                }
                if ((oneoffive & MOVE_CTOR_MASK) == 0) {
                    if (dump) {
                        hdr << classfiller << "public:\n";
                        dump = false;
                    }
                    hdr << classfiller << "    " << name << "(" << name << "&& aOther) = default;\n";
                }
                if ((oneoffive & COPY_ASSIGN_MASK) == 0) {
                    if (dump) {
                        hdr << classfiller << "public:\n";
                        dump = false;
                    }
                    hdr << classfiller << "    " << name << "& operator=(";
                    if (!gConstInArgumentRight) {
                        hdr << "const " << name << "& aOther) = default;\n";
                    } else {
                        hdr << name << " const& aOther) = default;\n";
                    }
                }
                if ((oneoffive & MOVE_ASSIGN_MASK) == 0) {
                    if (dump) {
                        hdr << classfiller << "public:\n";
                        dump = false;
                    }
                    hdr << classfiller << "    " << name << "& operator=(" << name << "&& aOther) = default;\n";
                }
                //
                //  As the DTOR_MASK maybe set above we check the real state besides oneoffive flag.
                if (((oneoffive & DTOR_MASK) == 0) || ((HasAnyVirtuals() && !HasDTOR()))) {
                    if (dump) {
                        hdr << classfiller << "public:\n";
                        dump = false;
                    }
                    hdr << classfiller << "    ";
                    if (HasAnyVirtuals()) {
                        hdr << "virtual ";
                    }
                    hdr << "~" << name << "() = default;\n";
                }
            }
        }
        //
        //  Now the operations follow.
        for (auto & mo : Operation) {
            auto cmo = std::dynamic_pointer_cast<COperation>(*mo);

            if ((mo->visibility == vi.first) && (!cmo->qtSlot) && (!cmo->qtSignal)) {
                //
                // Add the visibility that is valid now but only for classes not for structs.
                if ((dump) && (type != eElementType::Struct)) {
                    dump=false;
                    hdr << classfiller << vi.second << ":\n";
                }
                if ((type == eElementType::Struct) && (mo->visibility != vPublic)) {
                    std::cerr << "Visibility other than public in structs is not supported. " << mTypeTree.getFQN() << "::" << mo->name << std::endl;
                }
                auto op = std::dynamic_pointer_cast<COperation>(*mo);
                std::string header = op->getHeader(indent);
                std::string pdecl  = op->GetParameterDecl(mNameSpace);
                std::string filler;
                filler.assign(indent + IndentSize, ' ');
                hdr << header << filler << "///" <<  std::endl;

                if (op->isTemplateOperation()) {
                    op->DumpTemplateOperationPrefix(hdr, false, indent + IndentSize);
                }


                hdr << filler;
                if ((op->isStatic) && (!mIsInterface)) {
                    hdr << "static ";
                }
                if ((mIsInterface && !op->isAbstract) || (op->isInline)) {
                    hdr << "inline ";
                    op->isInline = true;
                }

                if ((op->isAbstract) || ((op->name == "~"+name) && HasAnyVirtuals()) || (mIsInterface && (op->name != mTypeTree.mName))) {
                    hdr << "virtual ";
                }
                if ((op->name != mTypeTree.mName) && (op->name != std::string("~") + mTypeTree.mName) && (!op->IsCastOperator())) {
                    hdr << op->GetReturnType(mNameSpace) << " ";
                    hdr << op->name << "(" <<  pdecl << ") ";
                    if (op->isQuery) {
                        hdr << "const ";
                    }
                    if (mIsInterface && op->isAbstract) {
                        hdr << "= 0";
                    }
                    if (op->isAbstract && (op->HasStereotype("pure") || op->isPure)) {
                        hdr << "= 0";
                    }
                    if (op->HasStereotype("default")) {
                        hdr << "= default";
                    }
                    if (op->HasStereotype("delete")) {
                        hdr << "= delete";
                    }
                    hdr << ";\n";
                } else {
                    if (pdecl == "void") {
                        hdr << op->name << "() ";
                    } else {
                        hdr << op->name << "(" <<  pdecl << ") ";
                    }
                    if (op->isQuery) {
                        hdr << "const ";
                    }
                    if (op->IsCastOperator()) {
                        hdr << ";\n";
                    } else {
                        //
                        //  Constructor or destructor
                        if (op->HasStereotype("default")) {
                            hdr << "= default";
                        }
                        if (op->HasStereotype("delete")) {
                            hdr << "= delete";
                        }
                        hdr << ";\n";
                    }
                }
            }
        }
    }
    //
    //  Then there are dumped all slots.
    for (auto & vi : vis) {
        dump=true;
        for (auto & mo : Operation) {
            auto cmo = std::dynamic_pointer_cast<COperation>(*mo);

            if ((mo->visibility == vi.first) && (cmo->qtSlot) ) {
                if (dump) {
                    dump=false;
                    hdr << classfiller << vi.second << " slots:\n";
                }
                auto op = std::dynamic_pointer_cast<COperation>(mo.ptr());
                std::string pdecl=op->GetParameterDecl(mNameSpace);

                hdr << classfiller << "    ";
                if (op->isStatic) {
                    hdr << "static ";
                }
                if (op->isAbstract) {
                    hdr << "virtual ";
                }
                hdr << op->GetReturnType(mNameSpace) << " ";
                hdr << op->name << "(" <<  pdecl << ") ;\n";
            }
        }
    }
    //
    //  Then there are dumped all signals
    dump=true;
    for (auto & mo : Operation) {
        auto cmo = std::dynamic_pointer_cast<COperation>(mo.ptr());

        if (cmo->qtSignal) {
            if (dump) {
                dump=false;
                hdr << classfiller << "signals:\n";
            }
            auto op = std::dynamic_pointer_cast<COperation>(mo.ptr());
            std::string pdecl=op->GetParameterDecl(mNameSpace);

            hdr << classfiller << "    ";
            if (op->isStatic) {
                hdr << "static ";
            }
            if (op->isAbstract) {
                hdr << "virtual ";
            }
            hdr << op->GetReturnType(mNameSpace) << " ";
            hdr << op->name << "(" <<  pdecl << ") ;\n";
        }
    }
}

void CCxxClass::DumpAttributeDecl(std::ostream& hdr, int indent) {
    size_t i = 0U;
    bool dump = true;
    std::string classfiller;
    classfiller.assign(indent, ' ');
    //
    //  These are the attributes sorted by visibility.
    //  Package visibility is not considered here.
    //  We are collecting them first for proper indentation.

    std::list<std::pair<std::string, std::string> >  attrlist[kVisSize];

    for (auto & vi: vis) {
        for (auto &a: allAttr) {
            if (a->visibility == vi.first) {
                std::string cname;
                std::ostringstream aname;
                std::ostringstream tname;

                if ((a->Classifier != nullptr) && (a->Classifier->IsClassBased())) {
                    auto cb = std::dynamic_pointer_cast<CClassBase>(a->Classifier.ptr());

                    cname = cb->mTypeTree.getFQN();
                    if (a->Classifier->HasStereotype("QtDesigner")) {
                        cname.append("Ui::");
                    }
                } else {
                    TypeNode tmptree(TypeNode::parse(a->ClassifierName));

                    tmptree.fill(mNameSpace.getString());
                    cname = tmptree.getFQN();
                }
                if (!cname.empty()) {
                    if (a->isStatic) {
                        tname << "static ";
                    }
                    if (a->isReadOnly) {
                        tname << "const ";
                    }
                    if (a->Aggregation == eAggregation::aShared) {
                        tname << "volatile ";
                    }
                    if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                        tname << cname;
                        aname << a->name;
                        if (!a->defaultValue.empty() && (!a->isStatic) && (!gInitMemberDefaultInInitializerList)) {
                            aname << " = " << a->defaultValue;
                        }


                    } else {
                        if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                            if (!a->QualifierType.empty()) {
                                tname << "std::map<" << a->QualifierType << ", " << cname << ">";
                                extraheader += " map";
                            } else {
                                extraheader += " vector";
                                tname << "std::vector<" << cname << ">";
                            }
                            aname << a->name;
                        } else {
                            tname << cname;
                            aname << a->name << "[" << a->Multiplicity << "]";
                            if (!a->defaultValue.empty() && (!gInitMemberDefaultInInitializerList)) {
                                aname << " = " << a->defaultValue;
                            }
                        }
                    }
                    attrlist[i].emplace_back(tname.str(), aname.str());
                }
            }
        }

        for (auto &a: allEnds) {
            if (a->Classifier) {
                if ((a->visibility == vi.first) && (a->isNavigable()) && (!a->name.empty()) && (!a->mOwner)) {
                    std::shared_ptr<MAssociation> pa = std::dynamic_pointer_cast<MAssociation>(a->parent.ptr());
                    std::shared_ptr<MAssociationEnd> oe = pa->OtherEnd(a);               // own end

                    std::string cname = std::dynamic_pointer_cast<CClassBase>(*a->Classifier)->mTypeTree.getFQN();

                    cname = mNameSpace.diff(cname);

                    if (a->Classifier->HasStereotype("QtDesigner")) {
                        cname.append("Ui::");
                    }

                    if (oe->Aggregation == aShared) {
                        cname.push_back('*');
                    } else {
                        if ((oe->Aggregation == aNone) && (a->Aggregation != aNone)) {
                            cname.push_back('*');
                        }
                    }
                    std::ostringstream tname;
                    std::ostringstream aname;

                    if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                        tname << cname;
                        aname << a->name;
                        if (!a->defaultValue.empty() && (!gInitMemberDefaultInInitializerList)) {
                            aname << " = " << a->defaultValue;
                        }
                    } else {
                        if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                            if (!a->QualifierType.empty()) {
                                tname << "std::map<" << a->QualifierType << ", " << cname << ">";
                                extraheader += " map";
                            } else {
                                std::string container;

                                if (pa->HasTaggedValue("Container")) {
                                    container = pa->GetTaggedValue("Container");
                                    if (container.empty()) {
                                        container = "std::vector";
                                    } else {
                                    }
                                } else {
                                    container = "std::vector";
                                }
                                auto element = MClass::mByFQN.find(container);

                                if (element != MClass::mByFQN.end()) {
                                    tname << element->first << "<" << cname << ">";
                                    extraheader += " " + element->first;
                                } else {
                                    tname << "std::vector<" << cname << ">";
                                    extraheader += " vector";
                                }
                            }
                            aname << a->name;
                        } else {
                            tname << cname;
                            aname << a->name << "[" << a->Multiplicity << "]";
                            if (!a->defaultValue.empty() && (!gInitMemberDefaultInInitializerList)) {
                                aname << " = " << a->defaultValue;
                            }
                        }
                    }
                    attrlist[i].emplace_back(tname.str(), aname.str());
                }
            }
        }
        i++;
    }
    size_t maxtype = 0;

    for (i = 0; i<vis.size(); ++i) {
        for (auto & li : attrlist[i]) {
            if (li.first.size() > maxtype) {
                maxtype = li.first.size();
            }
        }
    }
    for (i = 0; i<vis.size(); ++i) {
        std::string filler;
        dump = true;
        for (auto &li: attrlist[i]) {
            if ((dump) && (type != eElementType::Struct)) {
                dump = false;
                hdr << classfiller << vis[i].second << ":\n";
            }
            if ((type == eElementType::Struct) && (i != 0)) {
                std::cerr << "Visibility other than public in structs is not supported. " << mTypeTree.getFQN() << "::" << li.second << std::endl;
            }

            filler.assign(maxtype - li.first.size() + 1, ' ');
            hdr << classfiller << "    " << li.first << filler << li.second << ";\n";
        }
    }
}

void CCxxClass::DumpPackageAttributeDecl(std::ostream& hdr) {
    std::list<std::pair<std::string, std::string> >  attrlist;

    for (auto & ma : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

        if ((ma->visibility == vPackage) && (!a->isStatic)) {
            std::string        cname;
            std::ostringstream aname;
            std::ostringstream tname;

            if (a->Classifier) {
                cname = a->Classifier->FQN();
            } else {
                cname = a->ClassifierName;
            }
            if (!cname.empty()) {
                if (a->Aggregation == eAggregation::aShared) {
                    tname << "volatile ";
                }
                if (a->isReadOnly) {
                    tname << "const ";
                }
                tname << cname;
                aname << a->name;
                if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                } else {
                    if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                        aname << "[]";
                    } else {
                        aname << "[" << a->Multiplicity << "]";
                    }

                }
            }
            attrlist.emplace_back(tname.str(), aname.str());
        }
    }
    for ( auto & me : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(me.ptr());

        if ((me->visibility == vPackage) && (a->isNavigable()) && (!me->name.empty()) && (!a->mOwner)) {
            std::string cname;
            if (a->Classifier->type == eElementType::SimObject) {
                cname = "tSimObj";
            } else {
                cname = a->Classifier->FQN();
            }
            //
            //  ends[0] in OtherEnd
            auto pe0 = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr())->ends[0];
            auto pe1 = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr())->ends[1];

            if (pe0 == me) {
                if (std::dynamic_pointer_cast<CAssociationEnd>(pe1.ptr())->Aggregation == aShared) {
                    cname.push_back('*');
                }
            } else {
                if (std::dynamic_pointer_cast<CAssociationEnd>(pe0.ptr())->Aggregation == aShared) {
                    cname.push_back('*');
                }
            }
            std::ostringstream tname;
            std::ostringstream aname;

            if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                tname << cname;
                aname << a->name;
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    tname << "std::vector<" << cname << ">";
                    aname << a->name;
                } else {
                    tname << cname;
                    aname << a->name << "[" << a->Multiplicity << "]";
                }

            }
            attrlist.emplace_back(tname.str(), aname.str());
        }
    }
    size_t maxtype=0;
    for (auto & li : attrlist) {
        if (li.first.size()>maxtype) {
            maxtype=li.first.size();
        }
    }
    std::string filler;
    for (auto & li : attrlist) {
        filler.assign(maxtype-li.first.size()+1, ' ');
        hdr << "extern " << li.first << filler << li.second << ";\n";
    }
}

void CCxxClass::DumpStaticAttributeDefinition(std::ostream& hdr) {
    for (auto & vi : vis) {
        for (auto & ma : Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(ma.ptr());

            if ((ma->visibility == vi.first) && (a->isStatic)) {
                if (a->isReadOnly) {
                    hdr << "const ";
                }
                std::string cname;
                if (a->Classifier) {
                    cname = a->Classifier->FQN();
                } else {
                    cname = a->ClassifierName;
                }
                if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                    hdr << cname << " " << name << "::" << a->name;
                    if (a->defaultValue.empty()) {
                        hdr << ";\n";
                    } else {
                        hdr << " = " << a->defaultValue <<  ";\n";
                    }
                } else {
                    if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                        hdr << "std::vector<" << cname << "> " << name << "::" << a->name;
                        if (a->defaultValue.empty()) {
                            hdr << ";\n";
                        } else {
                            hdr << " = {" << a->defaultValue <<  "};\n";
                        }
                    } else {
                        hdr << cname << " " << name << "::" << a->name;
                        hdr << "[" << a->Multiplicity << "];\n";
                    }
                }
            }
        }
    }
}

void CCxxClass::DumpPackageAttributeDefinition(std::ostream& hdr) {
    for (auto & ma : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(ma.ptr());

        if (ma->visibility == vPackage) {
            std::string cname;
            if (a->Aggregation == eAggregation::aShared) {
                hdr << "volatile ";
            }
            if (a->isReadOnly) {
                hdr << "const ";
            }
            if (a->Classifier) {
                cname = a->Classifier->FQN();
            } else {
                cname = a->ClassifierName;
            }
            hdr << cname << " " << a->name;
            if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                if (a->defaultValue.empty()) {
                    hdr << ";\n";
                } else {
                    hdr << " = " << a->defaultValue <<  ";\n";
                }
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    hdr << "[]";
                    if (a->defaultValue.empty()) {
                        hdr << ";\n";
                    } else {
                        hdr << " = {" << a->defaultValue <<  " };\n";
                    }
                } else {
                    hdr << "[" << a->Multiplicity << "]";
                    if (a->defaultValue.empty()) {
                        hdr << ";\n";
                    } else {
                        hdr << " = {" << a->defaultValue <<  " };\n";
                    }

                }

            }
        }
    }
    std::list<std::pair<std::string, std::string> >  attrlist;

    for (auto & me : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*me);

        if ((me->visibility == vPackage) && (a->isNavigable()) && (!a->mOwner)){
            std::string cname;
            if (a->Classifier->type == eElementType::SimObject) {
                cname = "tSimObj";
            } else {
                cname = a->Classifier->FQN();
            }
            auto pe0 = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr())->ends[0];
            auto pe1 = std::dynamic_pointer_cast<CAssociation>(a->parent.ptr())->ends[1];

            if (pe0 == me) {
                if (std::dynamic_pointer_cast<CAssociationEnd>(*pe1)->Aggregation == aShared) {
                    cname.push_back('*');
                }
            } else {
                if (std::dynamic_pointer_cast<MAssociationEnd>(*pe0)->Aggregation == aShared) {
                    cname.push_back('*');
                }
            }
            std::ostringstream tname;
            std::ostringstream aname;

            if ((a->Multiplicity == "1") || (a->Multiplicity.empty())) {
                tname << cname;
                aname << a->name;
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    tname << "std::vector<" << cname << ">";
                    aname << a->name;
                } else {
                    tname << cname;
                    aname << a->name << "[" << a->Multiplicity << "]";
                }

            }
            attrlist.emplace_back(tname.str(), aname.str());
        }
    }
    size_t maxtype=0;
    for (auto & li : attrlist) {
        if (li.first.size()>maxtype) {
            maxtype=li.first.size();
        }
    }
    std::string filler;
    for (auto & li : attrlist) {
        filler.assign(maxtype-li.first.size()+1, ' ');
        hdr << li.first << filler << li.second << ";\n";
    }
}

void CCxxClass::DumpPackageOperationDefinition(std::ostream &src, bool aStatic) {
    std::string rettype;

    for (auto & mo : Operation) {
        auto op = std::dynamic_pointer_cast<COperation>(*mo);

        if ((mo->visibility == vPackage) && (op->isStatic == aStatic)) {
            std::string pdef=op->GetParameterDefinition(mNameSpace);

            op->DumpComment(src, 0, 130, "//", "//", "");

            rettype=op->GetReturnType(mNameSpace);
            if (rettype.at(rettype.size()-1)== '&') {
                rettype=rettype.substr(0, rettype.size()-1);
            }
            if (op->isStatic) {
                src << "static ";
            }
            if (op->HasStereotype("CLinkage")) {
                src << "extern \"C\" ";
            }
            if ((op->name != mTypeTree.mName) && (op->name != std::string("~") + mTypeTree.mName)) {
                src << op->GetReturnType(mNameSpace) << " ";
                src << op->name << "(" << pdef << ") {\n";
            } else {
                std::cerr << "Constructor/Destructor cannot be defined with package scope\n";
            }
            if ((op->GetReturnType(mNameSpace) != "void") && (op->GetReturnType(mNameSpace) == rettype) && (!op->GetReturnName().empty())) {
                if (op->GetReturnDefault().empty()) {
                    src << "    " << rettype << " " << op->GetReturnName() << ";\n";
                } else {
                    src << "    " << rettype << " " << op->GetReturnName() << " = " << op->GetReturnDefault() << ";\n";
                }
            }
            //
            //  Dump the activity if the operation has one.
            if (op->Activity) {
                auto a = std::dynamic_pointer_cast<CActivity>(*op->Activity);
                src << "// User-Defined-Code:" << op->id << "-pre\n";
                src << "// End-Of-UDC:" << op->id << "-pre\n";
                a->DumpCxx(src);
                src << "// User-Defined-Code:" << op->id << "-post\n";
                src << "// End-Of-UDC:" << op->id << "-post\n";
            } else {
                src << "// User-Defined-Code:" << op->id << "\n";
                src << "// End-Of-UDC:" << op->id << "\n";
            }

            if ((op->GetReturnType(mNameSpace) != "void") && (op->GetReturnType(mNameSpace) == rettype) && (!op->GetReturnName().empty())) {
                src << "    return  ("<< op->GetReturnName() << ");\n";
            }
            src << "}\n\n";
        }
    }
}

void CCxxClass::DumpOperationDefinition(std::ostream &src) {
    std::string rettype;

    for (auto & vi : vis) {
        for (auto & mo : Operation) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);
            if ((mo->visibility == vi.first) && (!mo->HasStereotype("Signal")) && !op->isInline && !op->isTemplateOperation() && !op->isPure) {
                //
                //  No implementation for operations that have a default or delete stereotype.
                if ((!op->HasStereotype("default")) && (!op->HasStereotype("delete")) &&
                        (!(op->isAbstract && op->HasStereotype("pure")))) {
                    std::string pdef=op->GetParameterDefinition(mNameSpace);

                    src  << op->getSourceHeader(0);

                    rettype=op->GetReturnType(mNameSpace);
                    if ((!rettype.empty()) && (rettype.at(rettype.size()-1)== '&')) {
                        rettype=rettype.substr(0, rettype.size()-1);
                    }
                    std::string ctorinit = op->GetCTORInitList();
                    if ((op->name != mTypeTree.mName) && (op->name != std::string("~") + mTypeTree.mName) && (!op->IsCastOperator())) {
                        src << op->GetReturnType(mNameSpace) << " ";
                        src << mTypeTree.mName << "::" << op->name << "(" << pdef << ")";
                        if (op->isQuery) {
                            src << " const ";
                        }
                    } else {
                        if (pdef == "void") {
                            src << mTypeTree.mName << "::" << op->name << "()";
                        } else {
                            src << mTypeTree.mName << "::" << op->name << "(" << pdef << ")";
                        }
                        if (op->isQuery) {
                            src << " const ";
                        }
                        if (op->name == mTypeTree.mName) {
                            if (!ctorinit.empty()) {
                                src << ctorinit;
                            }
                        }
                    }
                    src << " {\n";
                    if ((op->GetReturnType(mNameSpace) != "void") && (op->GetReturnType(mNameSpace) == rettype) && (!op->GetReturnName().empty())) {
                        if (op->GetReturnDefault().empty()) {
                            src << "    " << rettype << " " << op->GetReturnName() << ";\n";
                        } else {
                            src << "    " << rettype << " " << op->GetReturnName() << " = " << op->GetReturnDefault() << ";\n";
                        }
                    }

                    //
                    //  Dump the activity if the operation has one.
                    if (op->Activity) {
                        auto a = std::dynamic_pointer_cast<CActivity>(*op->Activity);

                        src << "// User-Defined-Code:" << op->id << "-pre\n";
                        src << "// End-Of-UDC:" << op->id << "-pre\n";
                        a->DumpCxx(src);
                        src << "// User-Defined-Code:" << op->id << "-post\n";
                        src << "// End-Of-UDC:" << op->id << "-post\n";
                    } else if (!op->mCollaboration.empty()) {
                        src << "// User-Defined-Code:" << op->id << "-pre\n";
                        src << "// End-Of-UDC:" << op->id << "-pre\n";
                        for (auto& col : op->mCollaboration) {
                            if (col->IsClassBased()) {
                                DumpCollaboration(src, std::dynamic_pointer_cast<CClassBase>(*col));
                            }
                        }
                        src << "// User-Defined-Code:" << op->id << "-post\n";
                        src << "// End-Of-UDC:" << op->id << "-post\n";
                    } else {
                        src << "// User-Defined-Code:" << op->id << "\n";
                        src << "// End-Of-UDC:" << op->id << "\n";
                    }
                    if ((op->GetReturnType(mNameSpace) != "void") && (op->GetReturnType(mNameSpace) == rettype) && (!op->GetReturnName().empty())) {
                        src << "    return  ("<< op->GetReturnName() << ");\n";
                    }
                    src << "}\n\n";
                }
            }  //  end of checking stereotypes.
        }
    }
}

std::string CCxxClass::GetBaseClasses() {
    std::string retval;
    eVisibility v=vNone;       //  the last seen visibility
    bool        delim = false; //  should we dump a delimiter
    //
    //  While parsing the Model we sorted the generalization so that the derived class
    //  has them attached.
    //
    //  Set the initial colon only if we have any Generalization.
    if (!Generalization.empty()) {
        retval = " : ";
        //
        //  Loop along the generalizations of this class.
        for (auto & gi : Generalization) {
            if (gi) {
                auto gip = std::dynamic_pointer_cast<CGeneralization>(*gi);
                if (delim) {
                    retval += ", ";
                } else {
                    delim = true;
                }
                v = gi->visibility;
                if (v == vPublic) {
                    retval+= "public ";
                } else if (v == vProtected) {
                    retval+= "protected ";
                } else if (v == vPrivate) {
                    retval+= "private ";
                }
                if (gip->base != nullptr) {
                    auto cb = std::dynamic_pointer_cast<CClassBase>(*gip->base);
                    retval+=cb->mTypeTree.getFQN();
                    //
                    // Ok check for template parameter
                    if (gip->isTemplateBinding()) {
                        retval +="<";
                        for (auto & tp : gip->mTemplateParameter) {
                            //
                            //  Add the separator if needed.
                            if (tp != *gip->mTemplateParameter.begin()) {
                                retval +=", ";
                            }
                            //
                            //  We use the actual parameter if it is set or
                            //  the classifier if actual is not set.
                            auto parameter = std::dynamic_pointer_cast<MParameter>(*tp.second);
                            //
                            //  Different ways to get the classifier/name for EA and StarUML
                            if (!parameter->mActual) {
                                if (parameter->Classifier) {
                                    auto cb = std::dynamic_pointer_cast<CClassBase>(*parameter->Classifier);

                                    retval += cb->mTypeTree.getFQN();
                                } else {
                                    retval += parameter->name;
                                }
                            } else {
                                if (parameter->mActual->IsClassBased()) {
                                    auto cb = std::dynamic_pointer_cast<CClassBase>(*parameter->mActual);

                                    retval += cb->mTypeTree.getFQN();
                                } else if (parameter->mActual->type == eElementType::Parameter) {
                                    auto tp = std::dynamic_pointer_cast<MParameter>(*parameter->mActual);

                                    retval += tp->name;
                                }
                            }
                        }
                    }
                    if (gip->isTemplateBinding()) {
                        retval += ">";
                    }
                }
            }
        }
    }

    return retval;
}

void CCxxClass::DumpAliases(std::ostream& file, int a_indent, eVisibility a_vis) {
    //
    //  Get all enclosed classes within this class.
    //  This is not recursive.
    auto enclosed = getEnclosedClasses();
    //
    //  Find all primitive types that has only itself as classifier and
    //  has a "use" dependency to another classifier.
    bool dump = true;

    for (auto & e : enclosed) {
        if (e->type == eElementType::PrimitiveType) {
            if (e->visibility == a_vis) {
                auto p = std::dynamic_pointer_cast<CPrimitiveType>(e);

                if (!p->mTypeTree.isCompositeType()) {
                    if (!p->Supplier.empty()) {
                        for (auto & u : p->Supplier) {
                            if ((u.getConnector()->type == eElementType::Dependency) && (u.getConnector()->HasStereotype("use")) && (u.getElement()->IsClassBased())) {
                                std::string filler;


                                if (dump && (a_vis != eVisibility::vPackage)) {
                                    dump = false;
                                    if (a_indent >= IndentSize) {
                                        filler.assign(a_indent-IndentSize, ' ');
                                    }
                                    file << filler << mapVisibility(a_vis) << ": //  From alias.\n";
                                }

                                filler.assign(a_indent, ' ');
                                file << filler << "using " << p->name << " = " << std::dynamic_pointer_cast<CClassBase>(u.getElement())->mTypeTree.getFQN() << ";" << std::endl;
                            }
                        }
                    } else {
                        if (!p->mAlias.empty()) {
                            std::string filler;


                            if (dump && (a_vis != eVisibility::vPackage)) {
                                dump = false;
                                if (a_indent >= IndentSize) {
                                    filler.assign(a_indent-IndentSize, ' ');
                                }
                                file << filler << mapVisibility(a_vis) << ": //  From alias.\n";
                            }

                            filler.assign(a_indent, ' ');
                            auto bestAlias = MClass::findBestMatch(p->mAlias, p->GetNameSpace());

                            if (bestAlias) {
                                auto cc = std::dynamic_pointer_cast<CClassBase>(bestAlias);

                                file << filler << "using " << p->name << " = " << cc->mTypeTree.getFQN() << ";" << std::endl;
                            } else {
                                file << filler << "using " << p->name << " = " << p->mAlias << ";" << std::endl;
                            }
                        }
                    }
                }
            }
        } else if (e->type == eElementType::CxxClass) {
             auto cxx = std::dynamic_pointer_cast<CCxxClass>(e);

             cxx->DumpAliases(file, a_indent, a_vis);
        }
    }
}

void CCxxClass::DumpSerializerDecl(std::ostream& aHdr, int indent) const {
    if (mSerialize) {
        aHdr << "public:\n"
            "    virtual size_t getSize(void) ;\n"
            "    virtual void* pack(void* aToPtr) ;\n"
            "    virtual void* unpack(void* aFromPtr) ;\n";
    }
}

void CCxxClass::DumpSerializerDefinition(std::ostream& aSrc) {
    if (mSerialize) {
        aSrc <<
            "size_t " << name << "::getSize(void) {\n"
            "    size_t retval = 0;\n\n";
        
        for (auto & gi : Generalization) {
            aSrc << "    retval += " << std::dynamic_pointer_cast<CGeneralization>(*gi)->base->name << "::getSize();\n";
        }
        for (auto& ma : Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

            if (a->Classifier) {
                if ((a->Classifier->type == eElementType::PrimitiveType) || (IsCardinalType(a->Classifier->name))) {
                    aSrc <<
                        "    retval += sizeof(" << a->Classifier->name << ");\n";
                }
                else if (a->Classifier->IsClassBased()) {
                    aSrc <<
                        "    retval += " << ma->name << "::getSize()\n";
                }
            }
        }

        aSrc <<
            "    return retval;\n"
            "}\n"
            "void* " << name << "::pack(void* aToPtr) {\n"
            "    uint8_t* retval = static_cast<uint8_t*>(aToPtr);\n\n";
        
        for (auto & gi : Generalization) {
            aSrc <<
                "    retval = static_cast<uint8_t*>(" << std::dynamic_pointer_cast<CGeneralization>(*gi)->base->name << "::pack(retval));\n";
        }
        for (auto& ma : Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

            if (a->Classifier) {
                if ((a->Classifier->type == eElementType::PrimitiveType) || (IsCardinalType(a->Classifier->name)) || (a->Classifier->type == eElementType::Enumeration)) {
                    if (a->Classifier->type == eElementType::Enumeration) {
                    }
                    aSrc <<
                        "    std::memcpy(retval, &" << ma->name << ", sizeof(" << a->Classifier->name << "));\n";
                    if (mByteOrder == eByteOrder::Network) {
                        
                    } else {
                    }
                    aSrc <<
                        "    retval += sizeof(" << a->Classifier->name << ");\n";
                }
                else if (a->Classifier->IsClassBased()) {
                    aSrc <<
                        "    retval += static_cast<uint8_t*>(" << ma->name << "::pack(retval));\n";
                }
            }
        }
        //for (auto& me : OtherEnd) {
        //}
        
        aSrc <<
            "    return retval;\n"
            "}\n"
            "void* " << name << "::unpack(void* aFromPtr) {\n"
            "    uint8_t* retval = static_cast<uint8_t*>(aFromPtr);\n\n";
        
        for (auto & gi : Generalization) {
            aSrc << 
                "    retval = static_cast<uint8_t*>(" << std::dynamic_pointer_cast<CGeneralization>(*gi)->base->name << "::unpack(retval));\n";
        }
        for (auto& ma : Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

            if (a->Classifier != nullptr) {
                if ((a->Classifier->type == eElementType::PrimitiveType) || (IsCardinalType(a->Classifier->name)) || (a->Classifier->type == eElementType::Enumeration)) {
                    if (a->Classifier->type == eElementType::Enumeration) {
                    }
                    aSrc <<
                        "    std::memcpy(&" << ma->name << ", retval, sizeof(" << a->Classifier->name << "));\n";
                    if (mByteOrder == eByteOrder::Network) {
                        
                    }
                    else {
                    }
                    aSrc <<
                        "    retval += sizeof(" << a->Classifier->name << ");\n";
                }
                else if (a->Classifier->IsClassBased()) {
                    aSrc <<
                        "    retval += static_cast<uint8_t*>(" << ma->name << "::pack(retval));\n";
                }
            }
        }
        //for (auto& me : OtherEnd) {
        //}
        
        aSrc <<
            "    return retval;\n"
            "}\n";
    }
}

#endif

void CModuleClass::Dump(std::shared_ptr<MModel> model) {
    bool                  dump;  //  A helper to decide whether to dump a commentary header.
    std::set<std::shared_ptr<MElement>>   oplist;
    std::set<std::string> includesdone;
    std::string           wxappclass    = GetTaggedValue("WxAppClass");

    //
    //  Collect the needed modell header.
    CollectNeededModelHeader(shared_this(), neededmodelheader);
    CollectForwards();
    //
    //  Before starting to create the files we suppress the creation of a source file
    //  for the obvious model elements that do not use a source file.
    if ((mIsInterface) || (!mClassParameter.empty())) {
        has_src.clear();
    }
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);
    DumpBase(cmodel, name);
    DumpFileHeader(hdr, name, ".h");
    //
    //  Check for requirements.
    bool req=false;

    for (auto & s : Supplier) {
        if ((s.getConnector()->type == eElementType::Dependency) && (s.getElement()->type == eElementType::Requirement)) {
            auto requirement = std::dynamic_pointer_cast<CRequirement>(s.getElement());

            if (!req) {
                hdr << gDoxygenCommentStart << std::endl;
                hdr << gDoxygenCommentStart <<" @coversreqs" << std::endl;
                req = true;
            }
            //
            //  requirement id
            hdr << gDoxygenCommentStart << " @req{" << helper::toDash(helper::tolower(requirement->name)) <<  "}" << std::endl;
        }
    }
    if (req) {
        hdr << gDoxygenCommentStart << " @endcoversreqs\n";
    }

    DumpGuardHead(hdr, name, mNameSpace.getString());
    if (!mIsInterface) {
        DumpFileHeader(src, name, ".cpp");
    }

    //
    //  Dump public macros into the header file.
    //  So they are defined where needed.
    DumpPublicMacros(hdr);
    donelist.clear();
    if (mSerialize) {
        includesdone.insert("string");
        hdr << "#include <string>\n";
    }
    //  Dump system headers.
    DumpSystemHeader(hdr);
    //
    //  Dump header includes
    if (IsQtDerived()) {
        DumpExtraIncludes(hdr, donelist, oplist);
        donelist.clear();
        dump = true;
        donelist.insert(name);
        DumpNeededIncludes(hdr, shared_this(), donelist, oplist);
    }
    //
    //
    donelist.clear();
    //
    //  Create forwards from list of optional headers.
    dump = true;
    DumpMessageForwards(hdr);
    DumpForwards(hdr);
    //
    //  Dump namespace start
    donelist.clear();
    DumpNameSpaceIntro(hdr);
    //
    //  Dump the class declaration with zero indentation.
    DumpClassDecl(hdr, -4);

    DumpInlineOperations(hdr);
    DumpPackageAttributeDecl(hdr);
    DumpPackageOperationDecl(hdr);

    DumpNameSpaceClosing(hdr);

    if ((HasTaggedValue("WxAppClass")) || (helper::tolower(wxappclass) == "true")) {
        hdr << "\n\nwxDECLARE_APP(" << name << ");\n\n";
    }

    DumpGuardTail(hdr, name, mNameSpace.getString());


    //
    //   Here the model header is complete.
    //if (gGenerateSelfContainedHeader) {
    if (visibility == vPackage) {
        //
        //   Dump self-contained header.
        CPath headerdir = gPackageHeaderDir + cmodel->pathstack.back() + "/." + name + ".hpp";
        headerdir.Create();
        std::string sysheaderpath = (std::string)headerdir;

        cmodel->generatedfiles.push_back(  tGenFile { sysheaderpath, id, "//", "mSysHeader"} );
        OpenStream(mSysHeader, sysheaderpath);
        if (!mSysHeader.is_open()) {
            std::cerr << "Cannot open file " << sysheaderpath << std::endl;
        } else {
            CollectSelfContainedHeader();

            DumpFileHeader(mSysHeader, name, ".h");
            DumpGuardHead(mSysHeader, name);
            //
            //  Dump public macros into the header file.
            //  So they are defined where needed.
            DumpPublicMacros(mSysHeader);
            donelist.clear();

            dump = true;
            donelist.insert(name+".h");

            for (auto & h : mSelfContainedExtras) {
                //
                //  Working on the CClassBase
                auto ch = std::dynamic_pointer_cast<CClassBase>(h);

                if (dump) {
                    dump = false;
                    mSysHeader << "//\n//  System Headers\n";
                }
                //
                //  System header may come from external classes or from tags added to a class
                //  that is part of the model. But even external classes may have extra headers defined.
                //
                //  Checking the tagged value content.
                if (ch->extraheader.empty()) {
                    //
                    //  No tagged value content available. Using the class name.
                    mSysHeader << "#include <" << h->name << ".h>\n";
                } else {
                    DumpExtraIncludes(ch->extraheader, mSysHeader, donelist, false);
                }
            }
            for (auto & h : mSelfContainedQt) {
                if (dump) {
                    dump = false;
                    mSysHeader << "//\n//  Qt Headers\n";
                }
                mSysHeader << "#include <" << h->name << ">\n";
            }
            dump = true;
            if (mSerialize) {
                includesdone.insert("string");
                mSysHeader << "#include <string>\n";
            }

            for (auto & h : mSelfContainedHeaders) {
                if (h->type == eElementType::ExternClass) {
                    //
                    //  Working on the CClassBase
                    auto ch = std::dynamic_pointer_cast<CClassBase>(h);
                    //
                    //  Checking the tagged value content.
                    if (ch->extraheader.empty()) {
                        std::string fname = ch->name+".hpp";

                        if (donelist.find(fname) == donelist.end()) {
                            //
                            //  No tagged value content available. Using the class name.
                            mSysHeader << "#include <" << fname << ".hpp>\n";
                            donelist.insert(fname);
                        } else {
                            DumpExtraIncludes(ch->extraheader, mSysHeader, donelist, false);
                        }
                    }
                } else {
                    std::string fname = h->name+".hpp";

                    if (donelist.find(fname) == donelist.end()) {
                        if (dump) {
                            dump = false;
                            mSysHeader << "//\n//  Model Headers\n";
                        }
                        mSysHeader << "#include <" << h->name << ".hpp>\n";
                        donelist.insert(fname);
                    }
                }
            }

            //
            //
            donelist.clear();
            //
            //  Create forwards from list of optional headers.
            dump = true;
            for (auto & f : forwards) {
                if (donelist.find(f->name) == donelist.end()) {
                    if (f->type != eElementType::PrimitiveType) {
                        if (dump) {
                            dump = false;
                            mSysHeader << "//\n"
                                   "//  List of forwards from forwards list.\n";
                        }
                        //
                        //  Output the forward.
                        //
                        //  We need to remove trailing reference chars
                        //  So we need a copy of the name
                        std::string fname = f->name;
                        size_t      fpos;

                        while ((fpos = fname.find_first_of("*&")) != std::string::npos) fname.erase(fpos, 1);
                        if (f->HasStereotype("QtDesigner")) {
                            mSysHeader << "namespace Ui { class " << fname << "; }\n";
                        } else {
                            mSysHeader << mClassifierType << ' ' << fname << ";\n";
                        }
                        //
                        //  Add name to donelist to prevent us from doing things twice.
                        donelist.insert(fname);
                    }
                }
            }
            DumpMessageForwards(mSysHeader);
            //
            //  Dump namespace-intro.
            DumpNameSpaceIntro(mSysHeader);

            mSysHeader << "//\n//  This is the class\n";
            //
            //  Output the class declaration.
            mSysHeader << mClassifierType << ' ' << name << GetBaseClasses() << " {\n";
            if (IsQtDerived()) {
                mSysHeader << "Q_OBJECT\n";
            }
            DumpOperationDecl(mSysHeader, 0);
    //        DumpQtConnectorDecl(mSysHeader);
            DumpAttributeDecl(mSysHeader, 0);
            mSysHeader << "};\n";
            DumpInlineOperations(mSysHeader);
            DumpPackageAttributeDecl(mSysHeader);
            DumpPackageOperationDecl(mSysHeader);
            //
            //  Dump namespace-intro.
            DumpNameSpaceClosing(mSysHeader);

            if ((HasTaggedValue("WxAppClass")) || (helper::tolower(wxappclass) == "true")) {
                mSysHeader << "\n\nwxDECLARE_APP(" << name << ");\n\n";
            }
            DumpGuardTail(mSysHeader, name);
        }
    }

    donelist.clear();

    if (!mIsInterface) {
        if (IsQtDerived()) {
            src << "//\n//  All needed headers in header file. This is needed for the moc tool.\n";
            src << "#include \"" << name << ".h\"\n";
        } else {
            DumpExtraIncludes(src, donelist, oplist);
            //donelist.clear();
            dump = true;
            DumpNeededIncludes(src, shared_this(), donelist, oplist);
        }
        src << "// Optional\n";
        oplist.clear();
        DumpOptionalIncludes(src, shared_this(), donelist, oplist);
        //
        //  Dump include statements for all incoming messages and signals.
        DumpMessageIncludes(src, donelist, includesdone);

//        DumpQtConnectorIncludes(src, donelist, includesdone);
        DumpPrivateMacros(src);

        if (gNameSpaceInCPP) {
            DumpNameSpaceIntro(src);
        }

        DumpStaticAttributeDefinition(src);
        DumpPackageAttributeDefinition(src);
        DumpPackageOperationDefinition(src, true);
        DumpPackageOperationDefinition(src, false);
//        DumpQtConnectorDefinition(src);
        DumpSerializerDefinition(src);
        DumpOperationDefinition(src);

        if (gNameSpaceInCPP) {
            DumpNameSpaceClosing(src);
        }

        if ((HasTaggedValue("WxAppClass")) || (helper::tolower(wxappclass) == "true")) {
            src << "wxIMPLEMENT_APP(" << name << ");\n\n";
        }
    }

    CloseStreams();
    mSysHeader.close();
}

#if 0

void CCxxClass::DumpMessageForwards(std::ostream & hdr) {
    //
    //
    //  Here we create the function prototypes for the message processing functions
    std::set<std::string> haveit;
    for (auto & mi : Incoming) {
        if (mi->type == eElementType::SimMessage) {
            auto s = std::dynamic_pointer_cast<CSimMessage>(*mi);
            auto sc=std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

            if (sc) {
                if (haveit.find(s->name)==haveit.end()) {
                    hdr << mClassifierType << ' ' << sc->name << ";\n";
                    haveit.insert(s->name);
                }
            }
        } else if (mi->type == eElementType::JSONMessage) {
            auto s = std::dynamic_pointer_cast<CJSONMessage>(*mi);

            if (s->Class && (s->Class->type == eElementType::SimMessageClass)) {
                auto sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

                if (sc) {
                    if (haveit.find(s->name)==haveit.end()) {
                        hdr << mClassifierType << ' ' << sc->name << ";\n";
                        haveit.insert(s->name);
                    }
                }
            } else if (s->Class && (s->Class->type == eElementType::SimSignalClass)) {
                auto sc = std::dynamic_pointer_cast<CSignalClass>(s->GetClass());

                if (sc) {
                    if (haveit.find(s->name)==haveit.end()) {
                        hdr << mClassifierType << ' ' << sc->name << ";\n";
                        haveit.insert(s->name);
                    }
                }
            }
        } else if (mi->type == eElementType::SimSignal) {
            auto s = std::dynamic_pointer_cast<CSimSignal>(*mi);

            auto sc = std::dynamic_pointer_cast<CSignalClass>(s->GetClass());

            if (sc) {
                if (haveit.find(s->name)==haveit.end()) {
                    hdr << mClassifierType << ' ' << sc->name << ";\n";
                    haveit.insert(s->name);
                }
            }
        }
    }
    //
    //
    for (auto & deplist : Dependency) {
        if ((deplist->HasStereotype("Import")) && (std::dynamic_pointer_cast<CDependency>(*deplist)->target->type == eElementType::CxxClass)) {
            std::dynamic_pointer_cast<CCxxClass>(*std::dynamic_pointer_cast<CDependency>(*deplist)->target)->DumpMessageForwards(src);
        }
    }

    for (auto & baselist : Base) {
        if (baselist.getElement()->type == eElementType::CxxClass) {
            std::dynamic_pointer_cast<CCxxClass>(baselist.getElement())->DumpMessageForwards(src);
        }
    }
}


bool CCxxClass::HasAnyVirtuals(void) {
    bool retval = false;

    for (auto & op : Operation) {
        if (std::dynamic_pointer_cast<COperation>(*op)->isAbstract) {
            retval = true;
            break;
        }
    }
    if (!retval) {
        for( auto & b : Base) {
            if ((b.getElement()->type == eElementType::QtClass) || (b.getElement()->type == eElementType::SimObject)) {
                retval = true;
            } else if (b.getElement()->type == eElementType::CxxClass) {
                retval = std::dynamic_pointer_cast<CCxxClass>(b.getElement())->HasAnyVirtuals();
            }
            if (retval) {
                break;
            }
        }
    }
    return retval;
}


bool CCxxClass::HasDTOR(void) {
    bool retval = false;

    for (auto & op : Operation) {
        if (op->name == "~"+name) {
            retval = true;
            break;
        }
    }
    return retval;
}

std::shared_ptr<COperation> CCxxClass::findBySignature(const std::string& aSignature) {
    std::shared_ptr<COperation> retval;

    for (auto & o : Operation) {
        std::string os = o->name + "(" + std::dynamic_pointer_cast<COperation>(*o)->GetParameterSignature(NameSpace())  +")";
        if (os == aSignature) {
            retval = std::dynamic_pointer_cast<COperation>(*o);
            break;
        }
    }
    return retval;
}


void CCxxClass::DumpForwards(std::ostream& file) {
    NameSpaceNode nstree;

    for (auto & f : forwards) {
        nstree.add(f->mNameSpace.get(), f);
    }
    nstree.dump(file);
}

void CCxxClass::DumpInlineOperations(std::ostream &hdr) {
    std::string rettype;

    for (auto & vi : vis) {
        for (auto & mo : Operation) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);
            if ((mo->visibility == vi.first) && (!mo->HasStereotype("Signal")) && (op->isInline || isTemplateClass() || op->isTemplateOperation())) {
                //
                //  No implementation for operations that have a default or delete stereotype.
                if ((!op->HasStereotype("default")) && (!op->HasStereotype("delete"))) {
                    std::string pdef=op->GetParameterDefinition(mNameSpace);

                    op->DumpComment(hdr, 0, 130, "//", "//", "");
                    if (isTemplateClass()) {
                        DumpTemplatePrefix(hdr);
                    }
                    if (op->isTemplateOperation()) {
                        op->DumpTemplateOperationPrefix(hdr, true);
                    }
                    rettype=op->GetReturnType(mNameSpace);
                    if ((!rettype.empty()) && (rettype.at(rettype.size()-1)== '&')) {
                        rettype=rettype.substr(0, rettype.size()-1);
                    }
                    //
                    // Get the init list.
                    std::string ctorinit = op->GetCTORInitList();

                    if ((op->name != mTypeTree.mName) && (op->name != std::string("~") + mTypeTree.mName) && (!op->IsCastOperator())) {
                        hdr << op->GetReturnType(mNameSpace) << " ";
                        hdr << mTypeTree.mName;
                        if (isTemplateClass()) {
                            DumpTemplateParameter(hdr);
                        }
                        hdr << "::" << op->name << "(" << pdef << ")";
                        if (op->isQuery) {
                            hdr << " const ";
                        }
                    } else {
                        if (pdef == "void") {
                            hdr << mTypeTree.mName;
                            if (isTemplateClass()) {
                                DumpTemplateParameter(hdr);
                            }
                            hdr << "::" << op->name << "() ";
                        } else {
                            hdr << mTypeTree.mName;
                            if (isTemplateClass()) {
                                DumpTemplateParameter(hdr);
                            }
                            hdr << "::" << op->name << "(" << pdef << ") ";
                        }
                        if (!ctorinit.empty()) {
                            hdr << ctorinit;
                        }
                    }
                    hdr << " {\n";
                    if ((op->GetReturnType(mNameSpace) != "void") && (op->GetReturnType(mNameSpace) == rettype) && (!op->GetReturnName().empty())) {
                        if (op->GetReturnDefault().empty()) {
                            hdr << "    " << rettype << " " << op->GetReturnName() << ";\n";
                        } else {
                            hdr << "    " << rettype << " " << op->GetReturnName() << " = " << op->GetReturnDefault() << ";\n";
                        }
                    }

                    //
                    //  Dump the activity if the operation has one.
                    if (op->Activity != nullptr) {
                        auto a = std::dynamic_pointer_cast<CActivity>(*op->Activity);

                        hdr << "// User-Defined-Code:" << op->id << "-pre\n";
                        hdr << "// End-Of-UDC:" << op->id << "-pre\n";
                        a->DumpCxx(hdr);
                        hdr << "// User-Defined-Code:" << op->id << "-post\n";
                        hdr << "// End-Of-UDC:" << op->id << "-post\n";
                    } else if (!op->mCollaboration.empty()) {
                        hdr << "// User-Defined-Code:" << op->id << "-pre\n";
                        hdr << "// End-Of-UDC:" << op->id << "-pre\n";
                        for (auto& col : op->mCollaboration) {
                            if (col->IsClassBased()) {
                                DumpCollaboration(hdr, std::dynamic_pointer_cast<CClassBase>(*col));
                            }
                        }
                        hdr << "// User-Defined-Code:" << op->id << "-post\n";
                        hdr << "// End-Of-UDC:" << op->id << "-post\n";
                    } else {
                        hdr << "// User-Defined-Code:" << op->id << "\n";
                        hdr << "// End-Of-UDC:" << op->id << "\n";
                    }
                    if ((op->GetReturnType(mNameSpace) != "void") && (op->GetReturnType(mNameSpace) == rettype) && (!op->GetReturnName().empty())) {
                        hdr << "    return  ("<< op->GetReturnName() << ");\n";
                    }
                    hdr << "}\n\n";
                }
            }  //  end of checking stereotypes.
        }
    }
}

void CCxxClass::CollectForwards(std::shared_ptr<CClassBase> aClass) {
    if (aClass && (aClass->IsClassBased())) {
        auto cil = aClass->getTypes();

        for (auto & cili : cil) {
            if (!(cili->IsExternClass()) && (cili->type != eElementType::QtClass) && (cili.get() != this) && 
                !((cili->type == eElementType::PrimitiveType) && (std::dynamic_pointer_cast<CPrimitiveType>(cili)->isAlias())) && aClass->mClassParameter.empty()) {
                forwards.insert(cili);
            }
        }
    }
}


void CCxxClass::CollectForwardRefs(std::shared_ptr<CClassBase> aClass) {
    if (aClass) {
        auto cil = aClass->getRefTypes();

        for (auto & cili : cil) {
            if (!(cili->IsExternClass()) && (cili->type != eElementType::QtClass) && (cili.get() != this) &&
                !((cili->type == eElementType::PrimitiveType) && (std::dynamic_pointer_cast<CPrimitiveType>(cili)->isAlias())) ) {
                forwards.insert(cili);
            }
        }
    }
}

void CCxxClass::DumpCollaboration(std::ostream &src, std::shared_ptr<CClassBase> aCollab) {
    if (aCollab) {
        for (auto & in : aCollab->mInteraction) {
            if (in) {
                for (auto & msg : std::dynamic_pointer_cast<CInteraction>(*in)->messages) {
                    auto message = std::dynamic_pointer_cast<CMessage>(*msg);

                    auto source      = std::dynamic_pointer_cast<CLifeLine>(*message->source);
                    auto target      = std::dynamic_pointer_cast<CLifeLine>(*message->target);
                    std::string signature  = message->GetSignature();

                    if (source && target && (!signature.empty()) && source->role && target->role) {
                        src << "    connect(" << source->role->name << ", SIGNAL(" << signature << "), " << target->role->name << ", SLOT(" << signature << "));\n";
                    }
                }
            }
        }
    }
}

uint8_t CCxxClass::getOneOfFive() {
    uint8_t mask = 0U;

    for (auto & o : Operation) {
        auto op = std::dynamic_pointer_cast<COperation>(*o);
        //
        //  We do the checks here. Step by step no sophisticated approach here.
        //
        //  check the constructors.
        if (o->name == name) {
            if (op->Parameter.empty()) {  //  Check for a default constructor.
                mask |= CTOR_MASK;
            } else {
                //
                //  If we have parameters it may be a copy constructor or move constructor. So check that.
                auto p = std::dynamic_pointer_cast<CParameter>(*(*(op->Parameter.begin())));

                std::string   tname;
                TypeExtension text;
                //
                //  Parameter is linked to a classifier.
                if (p->Classifier) {
                    tname = mTypeTree.mName;
                    text  = mTypeTree.mExtension;
                } else {
                    //
                    //  Create a temporary typetree.
                    TypeNode temp(TypeNode::parse(p->ClassifierName));

                    temp.fill(mNameSpace.getString());
                    tname = temp.mName;
                    text  = temp.mExtension;
                }
                if (tname == name) {
                    if (text == TypeExtension::Reference) {
                        mask |= COPY_CTOR_MASK;
                    }
                    if (text == TypeExtension::Moveable) {
                        mask |= MOVE_CTOR_MASK;
                    }
                }
            }
        }
        //
        //  check the assignment operators.
        if (o->name == "operator=") {
            if (!op->Parameter.empty()) {  //  Check for a default constructor.
                //
                //  If we have parameters it may be a copy constructor or move constructor. So check that.
                auto p = std::dynamic_pointer_cast<CParameter>(*(*(op->Parameter.begin())));
                std::string   tname;
                TypeExtension text;
                //
                //  Parameter is linked to a classifier.
                if (p->Classifier) {
                    tname = mTypeTree.mName;
                    text  = mTypeTree.mExtension;
                } else {
                    //
                    //  Create a temporary typetree.
                    TypeNode temp(TypeNode::parse(p->ClassifierName));

                    temp.fill(mNameSpace.getString());
                    tname = temp.mName;
                    text  = temp.mExtension;
                }
                if (tname == name) {
                    if (text == TypeExtension::Reference) {
                        mask |= COPY_ASSIGN_MASK;
                    }
                    if (text == TypeExtension::Moveable) {
                        mask |= MOVE_ASSIGN_MASK;
                    }
                }
            }
        }
        //
        //  check the destructor
        if (o->name == "~"+name) {
            mask |= DTOR_MASK;
        }

    }
    return mask;
}

#endif


void CModuleClass::DumpClassDecl(std::ostream& file, int indent) {
    std::string filler;
    int localindent = (indent < 0)?0:indent;

    if (isTemplateClass()) {
        DumpTemplatePrefix(file);
    }
    filler.assign(localindent, ' ');
    //
    //  Dump the usings.
    file << filler << "//\n" << filler << "//  type aliases\n";
    DumpAliases(file, localindent);
    if (comment.empty()) {
        file << filler << "///\n" << filler << "///  TODO: Add class description\n";
    } else {
        DumpComment(file, localindent, 120, "///", "///", "///");
    }
    //
    //  Insert the enclosed classes.
    bool dump = true;
    //
    //  First we dump all "normal" operations.
    for (auto& vi : vis) {
        DumpAliases(file, indent + IndentSize, vi.first);
        //
        //
        auto enclosed = getEnclosedClasses();

        for (auto& e : enclosed) {
            if (e->visibility == vi.first) {

                switch (e->type) {
                case eElementType::CxxClass:
                case eElementType::Struct:
                    if (indent >= 0) {
                        if (dump) {
                            dump = false;
                            file << filler << vi.second << ": // For enclosed class.\n";
                        }
                    }
                    std::dynamic_pointer_cast<CCxxClass>(e)->DumpClassDecl(file, indent + IndentSize);
                    break;
                case eElementType::Enumeration:
                    if (indent >= 0) {
                        if (dump) {
                            dump = false;
                            file << filler << vi.second << ": // For enclosed class.\n";
                        }
                    }
                    std::dynamic_pointer_cast<CEnumeration>(e)->DumpClassDecl(file, indent + IndentSize);
                    break;
                default:
                    break;
                }
            }
        }
    }
    DumpOperationDecl(file, localindent);
    DumpSerializerDecl(file, localindent);
    //    DumpQtConnectorDecl(file);
    DumpAttributeDecl(file, localindent);
    file << filler << "};\n";
}

