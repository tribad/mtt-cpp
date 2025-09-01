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
#include <iomanip>
#include <sstream>

#include "main.h"
#include "helper.h"
#include "crc64.h"
#include "massociationend.h"
#include "cassociationend.h"
#include "massociation.h"
#include "cassociation.h"
#include "mdependency.h"
#include "cdependency.h"
#include "mattribute.h"
#include "cclassbase.h"
#include "csimobjectv2.h"
#include "cattribute.h"
#include "mparameter.h"
#include "cparameter.h"
#include "moperation.h"
#include "coperation.h"
#include "mclass.h"
#include "cclass.h"
#include "cmodel.h"
#include "mmessage.h"
#include "cmessage.h"
#include "msimmessage.h"
#include "csimmessage.h"
#include "cmessageclass.h"
#include "mjsonmessage.h"
#include "cjsonmessage.h"
#include "msimsignal.h"
#include "csimsignal.h"
#include "csignalclass.h"
#include "mstate.h"
#include "cstate.h"
#include "maction.h"
#include "mpseudostate.h"
#include "cpseudostate.h"
#include "mtransition.h"
#include "mevent.h"
#include "msimstatemachine.h"
#include "csimstatemachine.h"
#include "cgeneralization.h"

#include "medge.h"
#include "mpin.h"
#include "mactionnode.h"
#include "mactivity.h"
#include "cactivity.h"

#include "cprimitivetype.h"

#include "namespacenode.h"

#include "ccxxclass.h"

static std::set<std::string> inttypes =
                       {"int64_t",
                        "int32_t",
                        "int16_t",
                        "int8_t",
                        "char",
                        "short",
                        "int",
                        "long"};

static std::set<std::string> uinttypes =
                        {"uint64_t",
                         "uint32_t",
                         "uint16_t",
                         "uint8_t",
                         "objectid_t",
                         "valueid_t",
                         "unsigned",
                         "unsigned char",
                         "unsigned int",
                         "unsigned long",
                         "unsigned short"
                        };

static const char* doubletypes[] = {"double",
                             "float"
                             };

extern bool doDump;

std::string CSimObjectV2::MapDBAttrType(const std::string& classifier) {
    std::string retval;

    if (uinttypes.find(classifier) != uinttypes.end()) {
        retval = "eMemberValueType::Int";
    } else if (inttypes.find(classifier) != inttypes.end()) {
        retval = "eMemberValueType::Int";
    } else if (classifier == "bool") {
        retval = "eMemberValueType::Int";
    } else if (classifier == "double") {
        retval = "eMemberValueType::Double";
    } else if (classifier == "float") {
        retval = "eMemberValueType::Double";
    } else  if (classifier == "string") {
        retval = "eMemberValueType::String";
    } else if (classifier == "char*") {
        retval = "eMemberValueType::String";
    } else {
        retval = "eMemberValueType::Reference";
    }

    return retval;
}

std::string CSimObjectV2::MapVariant(const std::string& classifier) {
    std::string retval = classifier;

    if (IsVariantType(classifier)) {
        retval = "tVariant";
    } else if (classifier == "__simobject__") {
        retval = "tObjectRef";
    }
    return retval;
}
std::string CSimObjectV2::MapSimAttr(const std::string& classifier) {
    std::string retval = classifier;

    if (IsVariantType(classifier)) {
        retval = "tMemberValue";
    } else if (classifier == "__simobject__") {
        retval = "tMemberRef";
    }
    return retval;
}

std::string CSimObjectV2::MapVariantType(const std::string& classifier) {
    std::string retval;

    if ((classifier == "double") || (classifier == "float")) {
        retval = "double";
    } else if ((classifier == "int64_t") || (classifier == "int32_t") || (classifier == "int16_t") ||
               (classifier == "int8_t")  || (classifier == "char") || (classifier == "short") || (classifier == "int") ||
               (classifier == "long")) {
        retval = "long long";
    } else if (classifier == "bool") {
        retval = "bool";
    } else if ((classifier == "uint64_t") || (classifier == "uint32_t") || (classifier == "uint16_t") ||
               (classifier == "uint8_t") || (classifier == "objectid_t") || (classifier == "valueid_t") ||
               (classifier == "unsigned") || (classifier == "unsigned char") || (classifier == "unsigned int") ||
               (classifier == "unsigned long") || (classifier == "unsigned short")) {
        retval = "unsigned long long";
    } else if (classifier == "string") {
    } else if (classifier == "__simobject__") {
    }
    return retval;
}

static std::string GetStateFncName(std::shared_ptr<MState> m) {
    std::string retval;

    if (m->parent->type==eElementType::State) {
        retval = GetStateFncName(std::dynamic_pointer_cast<MState>(*m->parent))+"_"+helper::normalize(helper::tolower(m->name));
    } else {
        retval=helper::normalize(helper::tolower(m->name));
    }

    return retval;
}

void CSimObjectV2::SetFromTags(const std::string& name, const std::string&value)
{
    if (helper::tolower(name) == "basename") {
        basename=value;
    }
    if (helper::tolower(name) == "mainviewport") {
        MainViewPort = true;
    }
    if (helper::tolower(name) == "releasetimeout") {
        ReleaseTimeout = std::atol(value.c_str());
    }
}
//
//  This is an easy indication for SimObject Types.
//  If something in the code-generator runs bad this type will be dumped into the files.
//  The compiler will complain about it later.
std::string CSimObjectV2::FQN() const {
    return "__simobject__";
}

#if 0
void CSimObjectV2::CollectNeededModelHeader(MElement *e) {
    MClass*     c   = (MClass*)e;
    std::list<CClassBase*>::iterator cbi;

    if (donelist.find(e->name)==donelist.end()) {
        CClassBase* cbe=(CClassBase*)e;
        //
        //  do not come here again.
        donelist.insert(e->name);
        //
        //  Go to the base-classes
        for (std::list<CClassBase*>::iterator bi=cbe->Base.begin(); bi != cbe->Base.end(); ++bi) {
            CollectNeededModelHeader(*bi);
        }
        //
        //  Process all attributes even the imported ones.
        std::vector<MAttribute*> alla = cbe->GetImportAttributes();

        for (std::vector<MAttribute*>::iterator i=alla.begin(); i!= alla.end(); ++i) {
            CAttribute *a=(CAttribute*)(*i);

            if (((a->Aggregation == aNone) || (a->Aggregation == aComposition)) && (a->Classifier!=0)) {
                eElementType et=a->Classifier->type;

                if ((et != eElementType::SimObject) && (et != eElementType::ExternClass)) {
                    CollectNeededModelHeader(a->Classifier);
                }
            } else {
                std::list<std::pair<std::string, std::string> > tlist=helper::typelist(a->ClassifierName);
                std::list<std::pair<std::string, std::string> >::iterator tli;

                for (tli = tlist.begin(); tli != tlist.end(); tli++) {
                    //
                    //  Check that the type is no pointer types.
                    //  Pointer types are optional and can generate forwards.
                    if (tli->second.find('*') == std::string::npos) {
                        std::map<std::string, MClass*>::iterator ci=MClass::ByName.find(tli->second);

                        if (ci != MClass::ByName.end()) {
                            if ((ci->second->type != eElementType::SimObject) && (ci->second->type != eElementType::ExternClass)) {
                                CollectNeededModelHeader(ci->second);
                            } else {
                                if (ci->second->type == eElementType::ExternClass) {
                                    optionalmodelheader.push_back(ci->second);
                                }
                            }
                        }
                    } else {
                        //
                        //  Here we handle pointer types.
                        //  Pointer types are stored in optionals.
                        std::string  cname=tli->second.substr(0, tli->second.find('*'));
                        std::map<std::string, MClass*>::iterator ci=MClass::ByName.find(cname);

                        if (ci != MClass::ByName.end()) {
                            optionalmodelheader.push_back(ci->second);
                        }
                    }
                }
            }
        }
        std::vector<MAssociationEnd*> alle = cbe->GetImportAssociationEnds();
        //
        //  Processing the aggregations/compositions that are navigable.
        for (std::vector<MAssociationEnd*>::iterator i=alle.begin(); i!= alle.end(); ++i) {
            CAssociationEnd* a       = (CAssociationEnd*)(*i);
            CAssociation*    assoc   = (CAssociation*)a->parent;
            CAssociationEnd* otherend = a;
            CAssociationEnd* thisend = (CAssociationEnd*)(assoc->OtherEnd(a->ClassifierRef));

            if ((otherend != 0) && (otherend->Classifier!=0) && (otherend->Navigable)) {
                if ((thisend != 0) && (thisend->Aggregation == aShared)) {
                    if (c==this) {
                        optionalmodelheader.push_back(otherend->Classifier);
                    }
                } else if ((thisend != 0) && (thisend->Aggregation == aComposition)) {
                    eElementType et=a->Classifier->type;

                    if ((et != eElementType::SimObject) && (et != eElementType::ExternClass)) {
                        CollectNeededModelHeader(a->Classifier);
                    } else {
                        if (et == eElementType::ExternClass) {
                            optionalmodelheader.push_back(a->Classifier);
                        }
                    }
                } else if ((thisend != 0) && (thisend->Aggregation == aNone)) {
                    optionalmodelheader.push_back(otherend->Classifier);
                }
                if (otherend->Multiplicity == "*") {
                    //
                    //  Search the default collection class.
                    std::map<std::string, MClass*>::iterator ci;
                    if (!otherend->Qualifier.empty()) {
                        ci=MClass::ByName.find("map");
                        //
                        //  Check if we found the default collection class.
                        if (ci != MClass::ByName.end()) {
                            optionalmodelheader.push_back(ci->second);
                        }
                        if (!otherend->QualifierType.empty()) {
                            std::list<std::pair<std::string, std::string> > tlist=helper::typelist(otherend->QualifierType);
                            std::list<std::pair<std::string, std::string> >::iterator tli;

                            for (tli = tlist.begin(); tli != tlist.end(); tli++) {
                                //
                                //  Check that the type is no pointer types.
                                //  Pointer types are optional and can generate forwards.
                                if (tli->second.find('*') == std::string::npos) {
                                    std::map<std::string, MClass*>::iterator ci=MClass::ByName.find(tli->second);

                                    if (ci != MClass::ByName.end()) {
                                        if ((ci->second->type != eElementType::SimObject) && (ci->second->type != eElementType::ExternClass)) {
                                            CollectNeededModelHeader(ci->second);
                                        } else {
                                            if (ci->second->type == eElementType::ExternClass) {
                                                optionalmodelheader.push_back(ci->second);
                                            }
                                        }
                                    }
                                } else {
                                    //
                                    //  Here we handle pointer types.
                                    //  Pointer types are stored in optionals.
                                    std::string  cname=tli->second.substr(0, tli->second.find('*'));
                                    std::map<std::string, MClass*>::iterator ci=MClass::ByName.find(cname);

                                    if (ci != MClass::ByName.end()) {
                                        optionalmodelheader.push_back(ci->second);
                                    }
                                }
                            }

                            //
                            //  Check if we found the default collection class.
                            if (ci != MClass::ByName.end()) {
                                optionalmodelheader.push_back(ci->second);
                            }
                        }
                    } else {
                        ci=MClass::ByName.find("vector");
                        //
                        //  Check if we found the default collection class.
                        if (ci != MClass::ByName.end()) {
                            optionalmodelheader.push_back(ci->second);
                        }
                    }
                }
            }
        }

        neededmodelheader.push_back(e);

        if (!((CClassBase*)(e))->GetExtraHeader().empty()) {
            extramodelheader.push_back(((CClassBase*)(e)));
        }

        if (e == this) {
            std::vector<MElement*> alls = cbe->GetImportSupplier();

            for (std::vector<MElement*>::iterator i=alls.begin(); i != alls.end(); ++i) {
                if (((*i)->type==eElementType::CxxClass) || ((*i)->type==eElementType::CClass) || ((*i)->type==eElementType::Struct) || ((*i)->type==eElementType::Enumeration) || ((*i)->type==eElementType::Union) ) {
                    CollectNeededModelHeader(*i);
                }
            }
        }
    }
}


void CSimObjectV2::CollectNeededRecursive(std::shared_ptr<MElement> e) {
    eElementType et=e->type;

    switch (et) {
//    case eElementType::SimObject:
//        break;
    case eElementType::ExternClass:
        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(e));
        break;
        //
        //  PrimitiveTypes shall be handled as any other class at this point.
    case eElementType::PrimitiveType:
        if (std::dynamic_pointer_cast<CClassBase>(e)->Supplier.empty()) {
            CollectNeededModelHeader(e);
        } else {
            for (auto s : std::dynamic_pointer_cast<CClassBase>(e)->Supplier) {
                if (s.getElement()->type == eElementType::ExternClass) {
                    extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(s.getElement()));
                } else {
                    CollectNeededModelHeader(s.getElement());
                }
            }
        }
        break;
    default:
        CollectNeededModelHeader(e);
        break;
    }
}
#endif

void CSimObjectV2::CollectNeededFromMessages(std::shared_ptr<MElement> e, HeaderList & a_headerlist) {
    auto ec = std::dynamic_pointer_cast<CClassBase>(e);
    auto  i = ec->GetImportIncoming();

    for (auto & mi : i) {
        auto mic = std::dynamic_pointer_cast<CMessage>(mi);

        if (mic->m_implementation) {
            auto s = mic->m_implementation->sharedthis<CClassBase>();

            if (mic->isMessage() || mic->isSignal()) {
                CollectNeededModelHeader(s, a_headerlist);
            }
        }
    }
    i = ec->GetImportOutgoing();

    for (auto & mi : i) {
        auto mic = std::dynamic_pointer_cast<CMessage>(mi);

        if (mic->m_implementation) {
            auto s = mic->m_implementation->sharedthis<CClassBase>();

            if (mic->isMessage() || mic->isSignal()) {
                CollectNeededModelHeader(s, a_headerlist);
            }
        }
    }
    for (auto& baselist : ec->Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            CollectNeededFromMessages(baselist.getElement(), a_headerlist);
        }
    }
#if 0
    if (e->name == "tSigSQLFetchReply") {
        std::cerr << e->name << std::endl;
    }
    if (e->IsClassBased()) {

        for (auto & mi : std::dynamic_pointer_cast<CClassBase>(e)->Incoming) {
            std::shared_ptr<MElement> me;

            switch (mi->type) {
            case eElementType::SimMessage:
            case eElementType::JSONMessage:
            case eElementType::SimSignal:
                me = std::dynamic_pointer_cast<CSimSignal>(*mi)->GetClass();
                break;
            default:
                break;
            }
            if (me) {
                CollectNeededModelHeader(me);
            }
        }
        for (auto & mo : std::dynamic_pointer_cast<CClassBase>(e)->Outgoing) {
            std::shared_ptr<MElement> me;

            switch (mo->type) {
            case eElementType::SimMessage:
            case eElementType::JSONMessage:
            case eElementType::SimSignal:
                me = std::dynamic_pointer_cast<CJSONMessage>(*mo)->GetClass();
                break;
            default:
                break;
            }
            if (me) {
                CollectNeededModelHeader(me);
            }
        }
    }
#endif
}

#if 0
void CSimObjectV2::CollectNeededFromTextTyped(std::string aTextType) {
    //
    //  Split the un-classified type into several subtypes. Catching templates as well.
    auto tlist = helper::typelist(aTextType);

    for (auto & tli : tlist) {
        //
        //  Check that the type is no pointer types.
        //  Pointer types are optional and can generate forwards.
        if (tli.second.find('*') == std::string::npos) {
            //
            //  "Not a pointer" handling
            auto ci = MClass::mByFQN.find(tli.second);

            if (ci != MClass::mByFQN.end()) {
                CollectNeededRecursive(ci->second);
            }
        } else {
            //
            //  Here we handle pointer types.
            //  Pointer types are stored in optionals.
            std::string cname = tli.second.substr(0, tli.second.find('*'));
            auto        ci    = MClass::mByFQN.find(cname);

            if (ci != MClass::mByFQN.end()) {
                eElementType et = ci->second->type;

                if ((et == eElementType::ExternClass) || (et == eElementType::QtClass)) {
                    extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                } else if (et == eElementType::SimObject) {
                    auto st = MClass::mByFQN.find("tSimObj");

                    if (st != MClass::mByFQN.end()) {
                        CollectNeededModelHeader(st->second);
                    }
                } else {
                    //
                    // We create a forward for the class and continue for the optionalheaders.
                    forwards.insert(std::dynamic_pointer_cast<CClassBase>(ci->second));
                    CollectOptionalModelHeader(ci->second);
                }
            }
        }
    }  //  end of for-loop over typenames.
}
/*
 * This method collects all model headers into a list.
 * They are needed to create a sorted list of #include statements on
 * top of a c++ source file.
 */
void CSimObjectV2::CollectNeededModelHeader(std::shared_ptr<MElement> e) {
    //MClass *c=(MClass*)e;

    if (donelist.find(e->name)==donelist.end()) {
        auto cbe = std::dynamic_pointer_cast<CClassBase>(e);
        donelist.insert(e->name);
        //
        //  Go to the base-classes
        //  Base classes only can be external or similar or are in the needed list.
        for (auto & bi : cbe->Base) {
            if ((bi.getElement()->type == eElementType::ExternClass) || (bi.getElement()->type == eElementType::QtClass)){
                extramodelheader.push_back(bi.getElement());
            } else {
                CollectNeededModelHeader(bi.getElement());
            }
        }
        //
        //  Go through all attributes.
        auto alla = GetImportAttributes();

        for (auto & aa : alla) {
            auto a = std::dynamic_pointer_cast<CAttribute>(aa);
            //
            //  Different ways to handle the attribute type.
            //  If the attribute type is linked to some classifier we use it.
            if (a->Classifier) {
                //
                //  If the multiplicity is '*' we add the vector definition to the extramodelheaders.
                if (a->Multiplicity == "*") {
                    auto ci = MClass::mByFQN.find("std::vector");
                    //
                    //  Check if we found the default collection class.
                    if (ci != MClass::mByFQN.end()) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                    }
                }
                //
                //  We have a look on the aggregation type of the attribute.
                //  Shared attributes are realized through pointers. And pointers only need the class forward.
                if ((a->Aggregation == aNone) || (a->Aggregation == aComposition)) {
                    CollectNeededRecursive(a->Classifier);
                } else {
                    //
                    // Shared attributes are pointers. So we add them to the forwards list.
                    forwards.insert(std::dynamic_pointer_cast<CClassBase>(*a->Classifier));
                    //
                    // And we check any optional headers.
                    CollectOptionalModelHeader(a->Classifier);
                }
            } else {
                CollectNeededFromTextTyped(a->ClassifierName);
            }
        }  //  end of foo-loop over attributes.
        //
        //  Go through all association ends.
        auto alle =std::dynamic_pointer_cast<CClassBase>(e)->GetImportAssociationEnds();

        for (auto & ae : alle) {
            auto a = std::dynamic_pointer_cast<CAssociationEnd>(ae);
            //
            //  The specific classes of an association.
            auto    assoc   = std::dynamic_pointer_cast<CAssociation>(*a->parent);
            auto    otherend = a;
            auto    thisend = std::dynamic_pointer_cast<CAssociationEnd>(assoc->OtherEnd(a));
            //
            //  Sanity check of otherend and check if navigable.
            if (otherend&& (otherend->Classifier) && (otherend->isNavigable())) {
                //
                //  Check if pointer or not.
                if (thisend && (thisend->Aggregation == aShared)) {
                    eElementType et=otherend->Classifier->type;

                    if ((et == eElementType::ExternClass) || (et == eElementType::QtClass)) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(*a->Classifier));
                    } else if (et == eElementType::SimObject) {
                        auto st = MClass::mByFQN.find("tSimObj");

                        if (st != MClass::mByFQN.end()) {
                            CollectNeededModelHeader(st->second);
                        }
                    } else {
                        //
                        // We create a forward for the class and continue for the optionalheaders.
                        forwards.insert(std::dynamic_pointer_cast<CClassBase>(*a->Classifier));
                        CollectOptionalModelHeader(a->Classifier);
                    }
                } else if (thisend && (thisend->Aggregation == aComposition)) {
                    //CollectNeededRecursive(a->Classifier);
                    eElementType et=otherend->Classifier->type;

                    if ((et == eElementType::ExternClass) || (et == eElementType::QtClass)) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(*a->Classifier));
                    } else if (et == eElementType::SimObject) {
                        auto st = MClass::mByFQN.find("tSimObj");

                        if (st != MClass::mByFQN.end()) {
                            CollectNeededModelHeader(st->second);
                        }
                    }
                }
                //
                //  As the otherend defines the properties of the attribute to generate we go-on
                //  with more specials.
                //
                //  Use the standard collection class if multiplicity is '*'
                if (otherend->Multiplicity == "*") {
                    //
                    //  Search the default collection class.
                    std::map<std::string, std::shared_ptr<MClass>>::iterator ci;
                    //
                    //  Only if we have not Qualifier defined we use vector as the collection class.
                    if (!otherend->mQualifier.empty()) {
                        if (thisend->Aggregation == aShared) {
                            ci=MClass::mByFQN.find("LockedMap");
                        } else {
                            //
                            //  We need map as we have a qualifier defined.
                            ci=MClass::mByFQN.find("std::map");
                        }
                        //
                        // The qualifier type is not empty. So we need to split the text to the types involved.
                        if (!otherend->QualifierType.empty()) {
                            CollectNeededFromTextTyped(otherend->QualifierType);
                        }
                    } else {  //  Qualifier empty.
                        if (thisend->Aggregation == aShared) {
                            if (otherend->Classifier->type == eElementType::SimObject) {
                                ci = MClass::mByFQN.find("LockedMap");
                            } else {
                                //
                                //  No qualifier set. vector container is enough.
                                ci=MClass::mByFQN.find("LockedVector");
                            }
                        } else {
                            if (otherend->Classifier->type == eElementType::SimObject) {
                                ci = MClass::mByFQN.find("std::map");
                            } else {
                                //
                                //  No qualifier set. vector container is enough.
                                ci=MClass::mByFQN.find("std::vector");
                            }
                        }
                    }
                    //
                    //  Check if we found the default collection class.
                    if (ci != MClass::mByFQN.end()) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                    }
                }
            } else {
                //std::cerr << "Classifier not Set:" << a->ClassifierRef << std::endl;
            }
        }
        //
        //  Go through the operations.
        for (auto & oi : cbe->Operation) {
            auto operation = std::dynamic_pointer_cast<COperation>(*oi);
            //
            //  Go through the parameters and return values.
            for (auto & pi : operation->Parameter) {
                auto param = std::dynamic_pointer_cast<CParameter>(*pi);

                if (param->Classifier) {
                    //
                    //  If we find any star at the classifier name we have a pointer type
                    if (param->ClassifierName.find('*') != std::string::npos) {
                        forwards.insert(std::dynamic_pointer_cast<CClassBase>(*param->Classifier));
                        CollectOptionalModelHeader(param->Classifier);
                    } else {
                        CollectNeededRecursive(param->Classifier);
                    }
                } else {
                    CollectNeededFromTextTyped(param->ClassifierName);
                }
            }
        }
        neededmodelheader.add(e);

        if (!(std::dynamic_pointer_cast<CClassBase>(e))->GetExtraHeader().empty()) {
            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(e));
        }
        CollectNeededFromMessages(e);
        //
        //  If we are on the way out the tree we may add the dependencies.
        if (e == sharedthis<MElement>()) {
            for (auto & i : Supplier) {
                if ((i.getElement()->type==eElementType::CxxClass) || (i.getElement()->type==eElementType::CClass) || (i.getElement()->type==eElementType::Struct) ||
                        (i.getElement()->type==eElementType::Enumeration) || (i.getElement()->type==eElementType::Union) || (i.getElement()->type==eElementType::SimEnumeration) ) {
                    CollectNeededModelHeader(i.getElement());
                    CollectOptionalModelHeader(i.getElement());
                } else {
                    if ((i.getElement()->type==eElementType::ExternClass) || (i.getElement()->type==eElementType::QtClass)) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(i.getElement()));
                    }
                }
            }
        }
    }
}

void CSimObjectV2::CollectOptionalModelHeader(std::shared_ptr<MElement> e) {
    //
    //  Check if already processed.
        //
        //  Add the element to the done list so we do not walk it along again.
        auto cbe=std::dynamic_pointer_cast<CClassBase>(e);
        //
        //  Go to the base-classes
        for (auto & bi : cbe->Base) {
            if (bi.getElement()->type == eElementType::ExternClass) {
                extramodelheader.push_back(bi.getElement());
            } else {
                CollectOptionalModelHeader(bi.getElement());
            }
        }
        //
        //  Go through all attributes. Even the imported ones.
        for (auto & a : cbe->allAttr) {
            //
            // Its different if we have a classifier from the model or only a string
            // to identify the classifier.
            if (a->Classifier) {
                //CVertexArrayObject
                //  Check the multiplicity to add the default collection class 'vector' to the extraheaders.
                if (a->Multiplicity == "*") {
                    auto ci = MClass::mByFQN.find("std::vector");
                    //
                    //  Check if we found the default collection class.
                    if (ci != MClass::mByFQN.end()) {
                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                    }
                }
                //
                //  Processing only attributes that are not shared.
                if ((a->Aggregation == aNone) || (a->Aggregation == aComposition)) {
                    eElementType et=a->Classifier->type;
                    //
                    //  SimObjects allways added through extra headers.
                    //  Same for external classes. We do not parse them.
                    if ((et != eElementType::SimObject) && (et != eElementType::ExternClass)) {
                        CollectOptionalModelHeader(a->Classifier);
                    } else {
                        //
                        //  Extern class goes through into the CVertexArrayObjectextra headers.
                        //  SimObjects are ignored here.
                        if (et == eElementType::ExternClass) {
                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(*a->Classifier));
                        }
                    }
                }
            } else {
                //
                //  We have a classifier by text. Not from the model.
                //  We first need a list of all types in the string.
                std::list<std::pair<std::string, std::string> > tlist=helper::typelist(a->ClassifierName);
                //
                //  Go through the list of types extracted from the string.
                for (auto & tli : tlist) {
                    //
                    //  Check that the type is no pointer types.
                    //  Pointer types are optional and can generate forwards.
                    if (tli.second.find('*') == std::string::npos) {
                        std::map<std::string, std::shared_ptr<MClass>>::iterator ci = MClass::mByFQN.find(tli.second);

                        if (ci != MClass::mByFQN.end()) {
                            if ((ci->second->type != eElementType::SimObject) && (ci->second->type != eElementType::ExternClass)) {
                                CollectOptionalModelHeader(ci->second);
                            } else {
                                if (ci->second->type == eElementType::ExternClass) {
                                    extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                                }
                            }
                        }
                    } else {
                        //
                        //  Here we handle pointer types.
                        //  Pointer types are stored in optionals.
                        std::string cname = tli.second.substr(0, tli.second.find('*'));
                        auto        ci  = MClass::mByFQN.find(cname);

                        if (ci != MClass::mByFQN.end()) {
                            if (ci->second->type == eElementType::ExternClass) {
                                extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                            } else {
                                if (ci->second != e) {
                                    CollectOptionalModelHeader(ci->second);
                                }
                            }
                        }
                    }
                }
            }
        }
        //
        //  Next we go through all association ends. Even the imported ones.
        for (auto & a : cbe->allEnds) {
            auto    assoc   = std::dynamic_pointer_cast<CAssociation>(*a->parent);
            auto    otherend = a;
            auto    thisend = std::dynamic_pointer_cast<CAssociationEnd>(assoc->OtherEnd(a));
            //
            //  Check that the otherend is navigable.
            if (otherend && otherend->Classifier && (otherend->isNavigable())) {
                //
                //
                if (thisend && (thisend->Aggregation == aShared)) {
                    //
                    //  check if the element point to ourselves.
                    if (e == sharedthis<MElement>()) {
                        eElementType et=a->Classifier->type;

                        if (et != eElementType::SimObject) {
                            if (otherend->Classifier != e) {
                                CollectOptionalModelHeader(otherend->Classifier);
                            }
                        } else {
                            auto st = MClass::mByFQN.find("tSimObj");

                            if (st != MClass::mByFQN.end()) {
                                CollectOptionalModelHeader(st->second);
                            }
                        }
                    }
                } else if (thisend && (thisend->Aggregation == aComposition)) {
                    eElementType et=a->Classifier->type;

                    if ((et != eElementType::SimObject) && (et != eElementType::ExternClass)) {
                        CollectOptionalModelHeader(a->Classifier);
                    } else {
                        if (et == eElementType::ExternClass) {
                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(*a->Classifier));
                        }
                    }
                }

                if (otherend->Multiplicity == "*") {
                    //
                    //  Search the default collection class.
                    std::map<std::string, std::shared_ptr<MClass>>::iterator ci;

                    if (!otherend->mQualifier.empty()) {
                        ci = MClass::mByFQN.find("std::map");
                        //
                        //  Check if we found the default collection class.
                        if (ci != MClass::mByFQN.end()) {
                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                        }
                        if (!otherend->QualifierType.empty()) {
                            std::list<std::pair<std::string, std::string> > tlist=helper::typelist(otherend->QualifierType);
                            std::list<std::pair<std::string, std::string> >::iterator tli;

                            for (tli = tlist.begin(); tli != tlist.end(); tli++) {
                                //
                                //  Check that the type is no pointer types.
                                //  Pointer types are optional and can generate forwards.
                                if (tli->second.find('*') == std::string::npos) {
                                    auto ci = MClass::mByFQN.find(tli->second);

                                    if (ci != MClass::mByFQN.end()) {
                                        if ((ci->second->type != eElementType::SimObject) && (ci->second->type != eElementType::ExternClass)) {
                                            CollectOptionalModelHeader(ci->second);
                                        } else {
                                            if (ci->second->type == eElementType::ExternClass) {
                                                extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                                            }
                                        }
                                    }
                                } else {
                                    //
                                    //  Here we handle pointer types.
                                    //  Pointer types are stored in optionals.
                                    std::string  cname=tli->second.substr(0, tli->second.find('*'));
                                    auto ci = MClass::mByFQN.find(cname);

                                    if (ci != MClass::mByFQN.end()) {
                                        CollectOptionalModelHeader(ci->second);
                                    }
                                }
                            }

                            //
                            //  Check if we found the default collection class.        //dumpHelp = true;
                            if (ci != MClass::mByFQN.end()) {
                                CollectOptionalModelHeader(ci->second);
                            }
                        }
                    } else {
                        ci=MClass::mByFQN.find("std::vector");
                        //
                        //  Check if we found the default collection class.
                        if (ci != MClass::mByFQN.end()) {
                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                        }
                    }
                }
            }
        }        //dumpHelp = true;

        if (e->IsClassBased()) {
            //
            //  Go through the operations.

            for (auto oi : cbe->Operation) {
                auto operation = std::dynamic_pointer_cast<COperation>(*oi);
                //
                //  Go through the parameters and return values.
                for (auto & pi : operation->Parameter) {
                    auto param = std::dynamic_pointer_cast<CParameter>(*pi);

                    if (param->Classifier) {
                        if (param->ClassifierName.at(param->ClassifierName.size()-1) == '*') {
                            CollectOptionalModelHeader(param->Classifier);        //dumpHelp = true;
                        } else {
                            eElementType et=param->Classifier->type;

                            if ((et != eElementType::SimObject) && (et != eElementType::ExternClass) && (param->Classifier != e)) {
                                CollectOptionalModelHeader(param->Classifier);
                            } else {
                                if (et == eElementType::ExternClass) {
                                    extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(*param->Classifier));
                                }
                            }
                        }
                    } else {
                        std::list<std::pair<std::string, std::string> > tlist=helper::typelist(param->ClassifierName);
                        std::list<std::pair<std::string, std::string> >::iterator tli;

                        for (tli = tlist.begin(); tli != tlist.end(); tli++) {
                            //
                            //  Check that the type is no pointer types.
                            //  Pointer types are optional and can generate forwards.
                            if (tli->second.find('*') == std::string::npos) {
                                std::string tname;

                                if (tli->second.find('&') == std::string::npos) {
                                    tname = tli->second;
                                } else {
                                    tname = tli->second.substr(0, tli->second.size()-1);
                                }
                                auto ci = MClass::mByFQN.find(tname);

                                if (ci != MClass::mByFQN.end()) {
                                    if ((ci->second->type != eElementType::SimObject) && (ci->second->type != eElementType::ExternClass) && (ci->second != e)) {
                                        CollectOptionalModelHeader(ci->second);
                                    } else {
                                        if (ci->second->type == eElementType::ExternClass) {
                                            extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                                        }
                                    }
                                }
                            } else {
                                //
                                //  Here we handle pointer types.
                                //  Pointer types are stored in optionals.
                                std::string  cname=tli->second.substr(0, tli->second.find('*'));
                                auto ci = MClass::mByFQN.find(cname);

                                if (ci != MClass::mByFQN.end()) {
                                    if (ci->second->type == eElementType::ExternClass) {
                                        extramodelheader.push_back(std::dynamic_pointer_cast<CClassBase>(ci->second));
                                    } else {
                                        CollectOptionalModelHeader(ci->second);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        //
        //  Add the optional header to the list.
        optionalmodelheader.add(e);
    //}
}
#endif



//
//  Collect all data from various sources.
void CSimObjectV2::Prepare(void) {
    //
    // This gets overwritten in Prepare if MainViewPort Tag is set.
    MainViewPort = false;
    if (!PrepDone) {
        CollectAttributes();
        PrepareBase();
        //
        //  Create the Basename and the lowercase version.
        if (basename.empty()) {
            basename=name;
        }
        lower_basename=helper::tolower(basename);
        upper_basename=helper::toupper(basename);

        if (name == "DirectedRelationship") {
            std::cerr << "Huhu\n";
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
        PrepDone = true;
    }
}

bool CSimObjectV2::HaveOperation(const char *opname) {
    for (auto & i : Operation) {
        if (i->name == opname) {
            return (true);
        }
    }
    return (false);
}

std::string CSimObjectV2::MkType(std::shared_ptr<CAttribute> a) {
    std::string retval;
    std::string fqn=a->FQN();

    if (!(a->ClassifierName.empty())) {
        fqn = MapVariant(fqn);
        if (fqn == "__simobject__") {
            fqn = "tObjectRef";
        } else {
        }
        if (a->Multiplicity == "1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..1") {
            retval=fqn+"*";
        } else if (a->Multiplicity == "0..*") {
            if (a->QualifierType.empty()) {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        } else if (a->Multiplicity == "1..*") {
            if (a->QualifierType.empty()) {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        } else if (a->Multiplicity == "*") {
            if (a->QualifierType.empty()) {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        } else if (a->Multiplicity.empty()) {
            retval=fqn;
        } else {
            retval=std::string("std::map< uint64_t, ")+fqn+" >";
        }
    }
    return retval;
}

std::string CSimObjectV2::MkType(std::shared_ptr<CAssociationEnd> a) {
    std::string retval;
    std::string fqn=a->FQN();

    fqn = MapVariant(fqn);
    if (fqn == "__simobject__") {
        fqn = "tObjectRef";
    } else {
    }
    if (a->Multiplicity == "1") {
        retval=fqn;
    } else if (a->Multiplicity == "0..1") {
        retval=fqn+"*";
    } else if (a->Multiplicity == "0..*") {
        if (a->QualifierType.empty()) {
            retval=std::string("std::map< uint64_t, ")+fqn+" >";
        } else {
            retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
        }
    } else if (a->Multiplicity == "1..*") {
        if (a->QualifierType.empty()) {
            retval=std::string("std::map< uint64_t, ")+fqn+" >";
        } else {
            retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
        }
    } else if (a->Multiplicity == "*") {
        if (a->QualifierType.empty()) {
            retval=std::string("std::map< uint64_t, ")+fqn+" >";
        } else {
            retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
        }
    } else if (a->Multiplicity.empty()) {
        retval=fqn;
    } else {
        retval=std::string("std::map< uint64_t, ")+fqn+" >";
    }

    return retval;
}

std::string CSimObjectV2::MkSimAttr(std::shared_ptr<CAttribute> a) {
    std::string retval;
    std::string fqn = a->FQN();

    if ((!a->ClassifierName.empty()) || (a->Classifier && !a->Classifier->name.empty())) {
        if (a->Classifier != nullptr) {
            if (a->Classifier->type == eElementType::SimEnumeration) {
                fqn = "uint64_t";
            } else if (a->Classifier->type == eElementType::PrimitiveType) {
                auto pr = std::dynamic_pointer_cast<CPrimitiveType>(*a->Classifier);
                auto prcb = pr->getAliasBase();

                if (prcb) {
                    fqn = prcb->name;
                }
            }
        }
        fqn = MapSimAttr(fqn);
        if (a->Multiplicity == "1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..*") {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            }
        } else if (a->Multiplicity == "1..*") {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap < uint64_t, ")+fqn+" >";
            }
        } else if (a->Multiplicity == "*") {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            }
        } else if (a->Multiplicity.empty()) {
            retval=fqn;
        } else {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            }
        }
    }
    return retval;
}

std::string CSimObjectV2::MkSimAttr(std::shared_ptr<CAssociationEnd> a) {
    std::string retval;
    std::string fqn=a->FQN();

    if (!(a->Classifier->name.empty())) {
        if (a->Classifier->type == eElementType::SimEnumeration) {
            fqn = "uint64_t";
        }
        fqn = MapSimAttr(fqn);
        if (a->Multiplicity == "1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..*") {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            }
        } else if (a->Multiplicity == "1..*") {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            }
        } else if (a->Multiplicity == "*") {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            }
        } else if (a->Multiplicity.empty()) {
            retval=fqn;
        } else {
            if (fqn == "tMemberValue") {
                retval="tMemberValueArray";
            } else if (fqn == "tMemberRef") {
                retval="tMemberRefArray";
            } else {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            }
        }
    }
    return retval;
}

std::string CSimObjectV2::MkVariant(std::shared_ptr<CAttribute> a) {
    std::string retval;
    std::string fqn=a->FQN();

    if (!(a->ClassifierName.empty())) {
        fqn = MapVariant(fqn);
        if (a->Multiplicity == "1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..*") {
            retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
        } else if (a->Multiplicity == "1..*") {
            retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
        } else if (a->Multiplicity == "*") {
            retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
        } else if (a->Multiplicity.empty()) {
            retval=fqn;
        } else {
            retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
        }
    }
    return retval;
}

std::string CSimObjectV2::MkVariant(std::shared_ptr<CAssociationEnd> a) {
    std::string retval;
    std::string fqn=a->FQN();
    std::string qualifier;

    if (!(a->Classifier->name.empty())) {
        fqn = MapVariant(fqn);
        if (a->QualifierType.empty()) {
            qualifier = "uint64_t";
        } else {
            qualifier = a->QualifierType;
        }
        if (a->Multiplicity == "1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..*") {
            retval=std::string("LockedMap< ") + qualifier + ", " +fqn +" >";
        } else if (a->Multiplicity == "1..*") {
            retval=std::string("LockedMap< ") + qualifier + ", " +fqn +" >";
        } else if (a->Multiplicity == "*") {
            retval=std::string("LockedMap< ") + qualifier + ", " +fqn +" >";
        } else if (a->Multiplicity.empty()) {
            retval=fqn;
        } else {
            retval=std::string("LockedMap< ") + qualifier + ", " +fqn +" >";
        }
    }
    return retval;
}


std::string CSimObjectV2::MapType(std::shared_ptr<CAttribute> attr) {
    std::string retval;

    if ((attr->Classifier) && (!attr->Classifier->name.empty())) {
        auto cb = attr->Classifier;
        auto cbn = cb->name;

        if (cb->type == eElementType::SimEnumeration) {
            retval="uint64_t";
        } else if (cb->type == eElementType::PrimitiveType) {
            auto pr = std::dynamic_pointer_cast<CPrimitiveType>(*cb);
            auto prcb = pr->getAliasBase();

            if (prcb) {
                retval = prcb->name;
            }
        } else {
            if (inttypes.find(attr->ClassifierName) != inttypes.end()) {
                retval="int64_t";
            }
            if (retval.empty()) {
                if (uinttypes.find(attr->ClassifierName) != uinttypes.end()) {
                    retval="uint64_t";
                }
                if (retval.empty()) {
                    for (size_t i=0; i<(sizeof(doubletypes)/sizeof(doubletypes[0])); ++i) {
                        if (attr->ClassifierName == doubletypes[i]) {
                            retval="double";
                            break;
                        }
                    }

                    if (retval.empty()) {
                        retval = attr->ClassifierName;
                    }
                }
            }
        }
    }
    return retval;
}

std::string CSimObjectV2::MapType(std::shared_ptr<CAssociationEnd> attr) {
    std::string retval;

    if ((attr->Classifier != nullptr) && (!attr->Classifier->name.empty())) {
        auto cb = attr->Classifier;
        auto cbn = cb->name;

        if (cb->type == eElementType::SimEnumeration) {
            retval="uint64_t";
        } else if (cb->type == eElementType::PrimitiveType) {
            auto pr = std::dynamic_pointer_cast<CPrimitiveType>(*cb);
            auto prcb = pr->getAliasBase();

            if (prcb != nullptr) {
                retval = prcb->name;
            }
        } else {
            if (inttypes.find(attr->Classifier->name) != inttypes.end()) {
                retval="int64_t";
            }
            if (retval.empty()) {
                if (uinttypes.find(attr->Classifier->name) != uinttypes.end()) {
                    retval="uint64_t";
                }
                if (retval.empty()) {
                    for (size_t i=0; i<(sizeof(doubletypes)/sizeof(doubletypes[0])); ++i) {
                        if (attr->Classifier->name == doubletypes[i]) {
                            retval="double";
                            break;
                        }
                    }
                    if (retval.empty()) {
                        retval = attr->Classifier->name;
                    }
                }
            }
        }
    }
    return retval;
}

int CSimObjectV2::GetStateIndex(std::shared_ptr<MState> st) {
    int x=0;
    std::string tofncname=GetStateFncName(st);

    for (auto & i : statelist) {
        if (i == tofncname+"_state") {
            return (x);
        }
        x++;
    }
    return (-1);
}

std::shared_ptr<MState> CSimObjectV2::GetInitialTargetState(std::shared_ptr<MState> st) {
    for (auto & i : st->States) {
        if (i->type == eElementType::PseudoState) {
            auto ps = std::dynamic_pointer_cast<MPseudoState>(*i);

            if (ps->kind=="initial") {
                if (!ps->Outgoing.empty()) {
                    return std::dynamic_pointer_cast<MState>(*std::dynamic_pointer_cast<MTransition>(*ps->Outgoing[0])->to);
                }
            }
        }

    }
    return (std::shared_ptr<MState>());
}


void CSimObjectV2::DumpSetStateSwitch(std::ostream& output) {
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        output << "    case IDA_STATE:\n";
        output << "        "<< "state = value;\n";
        output << "        break;\n";
        for (auto & st : sm->states) {
            if (st->type == eElementType::State) {
                auto substate = std::dynamic_pointer_cast<CState>(*st);

                substate->DumpSetStateSwitch(output, basename);
            }
        }
    }
}


void CSimObjectV2::DumpGetStateSwitch(std::ostream& output) {
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        output << "    case IDA_STATE:\n";
        output << "        "<< "retval = state;\n";
        output << "        break;\n";
        for (auto & st : sm->states) {
            if (st->type == eElementType::State) {
                auto substate = std::dynamic_pointer_cast<CState>(*st);

                substate->DumpGetStateSwitch(output, basename);
            }
        }
    }
}

void CSimObjectV2::DumpSetValueSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<CClassBase>(e);
    Crc64 crc;
    //
    //  Because the import of attributes moves the imported attributes into
    //  the scope of the simobject we need the pointers to the attributes
    //  to do the typemapping the right way.

    for (auto & i : c->allAttr) {
        auto a = std::dynamic_pointer_cast<CAttribute>(i);

        std::string uppername=prefix+helper::toupper(a->name);

        if (aDoneList.find(uppername) == aDoneList.end()) {
            aDoneList.insert(uppername);

            if (sharedthis<CSimObjectV2>() == where) {
                std::string cname = a->ClassifierName;

                if (a->Classifier && !a->Classifier->name.empty()) {
                    cname = a->Classifier->name;
                }
                //
                //  Prevent macros from beeing inserted.
                if (!cname.empty()) {
                    id_map.insert(std::pair<std::string, uint64_t>(uppername, crc.calc(uppername)));
                    id_name_map.insert(std::pair<std::string, uint64_t>(a->name, crc.calc(uppername)));
                }
            }
            if (a->visibility==vPublic) {
                if ((!a->Classifier) && (!a->ClassifierName.empty())) {
                    where->src << "    case " << uppername << ":\n";
                    where->src << "        *((tVariant*)(&" << localobject <<  a->name << ")) = value;\n";
                    where->src << "        break;\n";
                } else {

                    if (a->Classifier) {
                        switch (a->Classifier->type) {
                        case eElementType::SimObject:
                            //
                            //  Add to list of refs to initialize.
                            where->refs.push_back(a);
                            //
                            //  Create the case label.
                            where->src << "    case " << uppername << ":\n";
                            if ((a->Multiplicity == "1") || (a->Multiplicity == "0..1") || a->Multiplicity.empty()) {
                                where->src << "        ((tObjectRef&)" << localobject <<  a->name << ") = tObjectRef(value, nullptr);\n";
                            } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                                where->src << "        (tObjectRef&)(" << localobject << a->name << "[valueindex]) = tObjectRef(value, nullptr);\n";
                            } else {
                            }
                            where->src << "        break;\n";
                            break;
                        case eElementType::Struct:
                            if (a->isMultiple) {
                                DumpSetValueSwitch(where, a->Classifier, localobject+a->name+".At(valueindex).", uppername+"_", aDoneList);
                            } else {
                                DumpSetValueSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                            }
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            where->src << "    case " << uppername << ":\n";
                            if ((a->Multiplicity == "1") || (a->Multiplicity == "0..1") || (a->Multiplicity.empty()) ) {
                                where->src << "        ((tVariant&)" << localobject <<  a->name << ") = value;\n";
                            } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                                if (IsVariantType(a->ClassifierName)) {
                                    where->src << "        (tVariant&)(" << localobject << a->name << "[valueindex]) = value;\n";
                                } else {
                                    where->src << "        (tVariant&)(" << localobject << a->name << "[valueindex]) = *((" << a->ClassifierName << "*)(value));\n";
                                }
                            } else {
                            }

                            where->src << "        break;\n";
                            break;
                        }
                    }
                }
            }
        }
    }
    //
    //  Processing the aggregations/compositions that are navigable.
    for (auto & i : c->allEnds) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(i);
        /*
         * We found situations where the array definition is put into the name.
         * So we remove the array info from the name before using it for the macro generation.
         */
        std::string      fixedname;
        std::string      uppername;
        if (a->name.find_first_of("[]") != std::string::npos) {
            fixedname = a->name.substr(0, a->name.find_first_of("[]"));
            uppername=prefix+helper::toupper(fixedname);
        } else {
            uppername=prefix+helper::toupper(a->name);
        }
        /*
         * we add only values from the actual produced class into the id_map.
         * without this restrictions lots of attributes would be generated multiple times.
         */
        if (sharedthis<CSimObjectV2>() == where) {
            if ((!a->Classifier->name.empty()) && (!a->name.empty())) {
                id_map.insert(std::pair<std::string, uint64_t>(uppername, crc.calc(uppername)));
                id_name_map.insert(std::pair<std::string, uint64_t>(a->name, crc.calc(uppername)));
            }
        }
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/*(a->Navigable) && */ (a->Classifier!=e) && (a->visibility == vPublic) && (a->name.size() > 0)) {
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {

                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        //
                        //  Add to list of refs to initialize.
                        where->refs.push_back(a);
                        //
                        //  Create the case label.
                        where->src << "    case " << uppername << ":\n";
                        if ((a->Multiplicity == "1") || (a->Multiplicity == "0..1") || (a->Multiplicity.size() == 0)) {
                            where->src << "        ((tObjectRef&)" << localobject <<  a->name << ") = tObjectRef(value, nullptr);\n";
                        } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            where->src << "        (tObjectRef&)(" << localobject << a->name << "[valueindex]) = tObjectRef(value, nullptr);\n";
                        } else {
                        }
                        where->src << "        break;\n";
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpSetValueSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpSetValueSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::SimEnumeration:
                    case eElementType::Enumeration:
                        where->src << "    case " << uppername << ":\n";
                        if ((a->Multiplicity == "1") || (a->Multiplicity == "0..1") || (a->Multiplicity.size() == 0) ) {
                            where->src << "        ((tVariant&)" << localobject <<  a->name << ") = value;\n";
                        } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            if (IsVariantType(a->Classifier->name)) {
                                where->src << "        (tVariant&)(" << localobject << a->name << "[valueindex]) = value;\n";
                            } else {
                                where->src << "        (tVariant&)(" << localobject << a->name << "[valueindex]) = value;\n";
                            }
                        } else {
                        }

                        where->src << "        break;\n";
                        break;
                    case eElementType::Class:
                        break;

                    default:
                        where->src << "    case " << uppername << ":\n";
                        if ((a->Multiplicity == "1") || (a->Multiplicity == "0..1") || (a->Multiplicity.size() == 0) ) {
                            where->src << "        ((tVariant&)" << localobject <<  a->name << ") = value;\n";
                        } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            if (IsVariantType(a->Classifier->name)) {
                                where->src << "        (tVariant&)(" << localobject << a->name << "[valueindex]) = value;\n";
                            } else {
                                where->src << "        (tVariant&)(" << localobject << a->name << "[valueindex]) = *((" << a->Classifier->name << "*)(value));\n";
                            }
                        } else {
                        }

                        where->src << "        break;\n";
                        break;
                    }

                }
            }
        }
    }

    for (auto & baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpSetValueSwitch(where, baselist.getElement(), localobject, prefix,  aDoneList);
        }
    }
}

void CSimObjectV2::DumpGetValueSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string uppername=prefix+helper::toupper(a->name);

        if (aDoneList.find(uppername) == aDoneList.end()) {
            aDoneList.insert(uppername);

            if ((a->visibility==vPublic) || (a->visibility == vProtected)) {
                if (!(a->Classifier) && (!a->ClassifierName.empty())) {
                    where->src << "    case " << uppername << ":\n";
                    if (a->isMultiple) {
                        where->src << "        retval = " << localobject <<  a->name << "[valueindex];\n";
                    } else {
                        where->src << "        retval = " << localobject <<  a->name << ";\n";
                    }
                    where->src << "        break;\n";
                } else {

                    if (a->Classifier) {
                        switch (a->Classifier->type) {
                        case eElementType::SimObject:
                            break;
                        case eElementType::Struct:
                            if (a->isMultiple) {
                                DumpGetValueSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                            } else {
                                DumpGetValueSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                            }
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            where->src << "    case " << uppername << ":\n";
                            if (a->isMultiple) {
                                where->src << "        retval = " << localobject <<  a->name << "[valueindex];\n";
                            } else {
                                where->src << "        retval = " << localobject <<  a->name << ";\n";
                            }
                            where->src << "        break;\n";
                            break;
                        }
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/*(a->Navigable) && */(a->Classifier!=e) && ((a->visibility == vPublic) || (a->visibility == vProtected)) && (!a->name.empty())) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpGetValueSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpGetValueSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        where->src << "    case " << uppername << ":\n";
                        if (a->Classifier->name != "string") {
                            if ((a->Multiplicity == "1") || (a->Multiplicity.size() == 0)) {
                                where->src << "        retval = (" << MapType(a) << ")(" << localobject <<  a->name << ");\n";
                            } else if (a->Multiplicity == "0..1") {
                            } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                                if (a->QualifierType.empty()) {
                                    where->src << "        retval = (" << MapType(a) << ")(" << localobject <<  a->name << "[valueindex]);\n";
                                } else {
                                    if (a->QualifierType=="uint64_t") {
                                        where->src << "        retval = (" << MapType(a) << ")(" << localobject <<  a->name << "[valueindex]);\n";
                                    }
                                }
                            } else {
                            }
                        } else {
                            where->src << "        retval = " << localobject <<  a->name << ";\n";
                        }
                        where->src << "        break;\n";
                        break;
                    }
                }
            }
        }
    }
    //
    //
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpGetValueSwitch(where, *target, localobject, prefix,  aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpGetValueSwitch(where, baselist.getElement(), localobject, prefix,  aDoneList);
        }
    }
}


void CSimObjectV2::DumpSetValueDBSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);

        std::string uppername=prefix+helper::toupper(a->name);

        if ((aDoneList.find(uppername) == aDoneList.end() && ((a->visibility == vPublic) || (a->visibility == vProtected)))) {
            aDoneList.insert(uppername);
            if (!(a->Classifier) && (!a->ClassifierName.empty())){
                std::string maptype=MapType(a);

                where->src << "    case " << uppername << ":\n";
                if (a->isMultiple) {
                    where->src << "        " << localobject << a->name << "[valueindex] = value;\n";
                } else {
                    where->src << "        " << localobject << a->name << " = value;\n";
                }
                where->src << "        break;\n";
            } else {
                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpSetValueSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpSetValueSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        where->src << "    case " << uppername << ":\n";
                        if (a->isMultiple) {
                            where->src << "        " << localobject << a->name << "[valueindex] = value;\n";
                        } else {
                            where->src << "        " << localobject << a->name << " = value;\n";
                        }

                        where->src << "        break;\n";
                        break;
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/* (a->Navigable) && */ (a->Classifier!=e) && ((a->visibility == vPublic) || (a->visibility == vProtected)) && (!a->name.empty())) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpSetValueSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpSetValueSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        where->src << "    case " << uppername << ":\n";
                        if (a->isMultiple) {
                            where->src << "        " << localobject << a->name << "[valueindex] = value;\n";
                        } else {
                            where->src << "        " << localobject << a->name << " = value;\n";
                        }

                        where->src << "        break;\n";
                        break;
                    }
                }
            }
        }
    }
    //
    //
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if (((*deplist)->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpSetValueDBSwitch(where, *target, localobject, prefix,  aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpSetValueDBSwitch(where, baselist.getElement(), localobject, prefix,  aDoneList);
        }
    }
}

void CSimObjectV2::DumpGetReferenceSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);

        std::string uppername=prefix+helper::toupper(a->name);

        if (aDoneList.find(uppername) == aDoneList.end()) {
            aDoneList.insert(uppername);

            if (a->visibility==vPublic) {
                if (!(a->Classifier) && (!a->ClassifierName.empty())) {
                } else {
                    if (a->Classifier) {
                        switch (a->Classifier->type) {
                        case eElementType::SimObject:
                            //
                            //  Create the case label.
                            where->src << "    case " << uppername << ":\n";
                            if ((!a->isMultiple) || (a->Multiplicity == "0..1")) {
                                where->src << "        retval = " << localobject <<  a->name << ";\n";
                            } else {
                                where->src << "        retval = " << localobject <<  a->name << "[valueindex];\n";
                            }

                            where->src << "        break;\n";
                            break;
                        case eElementType::Struct:
                            if (a->isMultiple) {
                                DumpGetReferenceSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                            } else {
                                DumpGetReferenceSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                            }
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/*(a->Navigable) &&(a->Classifier!=e) && */ (a->visibility == vPublic) && (!a->name.empty())) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        //
                        //  Create the case label.
                        where->src << "    case " << uppername << ":\n";
                        if ((!a->isMultiple) || (a->Multiplicity == "0..1")) {
                            where->src << "        retval = " << localobject <<  a->name << ";\n";
                        } else {
                            where->src << "        retval = " << localobject <<  a->name << "[valueindex];\n";
                        }
                        where->src << "        break;\n";
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpGetReferenceSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpGetReferenceSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    //
    //
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpGetReferenceSwitch(where, *target, localobject, prefix,  aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpGetReferenceSwitch(where, baselist.getElement(), localobject, prefix,  aDoneList);
        }
    }
}


void CSimObjectV2::DumpSetReferenceSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string uppername=prefix+helper::toupper(a->name);

        if ((aDoneList.find(uppername) == aDoneList.end() && ((a->visibility == vPublic) || (a->visibility == vProtected)))) {
            aDoneList.insert(uppername);
            if (!(a->Classifier) && (!a->ClassifierName.empty())){
            } else {

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        where->src << "    case " << uppername << ":\n";
                        if (a->isMultiple) {
                            where->src << "        " << localobject << a->name << "[valueindex] = value;\n";
                        } else {
                            where->src << "        " << localobject << a->name << " = value;\n";
                        }
                        where->src << "        break;\n";
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpSetReferenceSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpSetReferenceSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/* (a->Navigable) && (a->Classifier!=e) &&  */((a->visibility == vPublic) || (a->visibility == vProtected)) && (!a->name.empty())) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        where->src << "    case " << uppername << ":\n";
                        if (a->isMultiple) {
                            where->src << "        " << localobject << a->name << "[valueindex] = value;\n";
                        } else {
                            where->src << "        " << localobject << a->name << " = value;\n";
                        }
                        where->src << "        break;\n";
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpSetReferenceSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpSetReferenceSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    //
    //
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpSetReferenceSwitch(where, *target, localobject, prefix,  aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpSetReferenceSwitch(where, baselist.getElement(), localobject, prefix,  aDoneList);
        }
    }
}


void CSimObjectV2::DumpRemoveReferenceSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string uppername=prefix+helper::toupper(a->name);

        if ((aDoneList.find(uppername) == aDoneList.end() && ((a->visibility == vPublic) || (a->visibility == vProtected)))) {
            aDoneList.insert(uppername);
            if (!(a->Classifier) && (!a->ClassifierName.empty())){
            } else {

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        where->src << "    case " << uppername << ":\n";
                        if (a->isMultiple) {
                            where->src << "        " << localobject << a->name << ".Remove(valueindex);\n";
                        } else {
                            where->src << "        " << localobject << a->name << " = tObjectRef();\n";
                        }
                        where->src << "        break;\n";
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpRemoveReferenceSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpRemoveReferenceSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/* (a->Navigable) &&  (a->Classifier!=e) &&*/ ((a->visibility == vPublic) || (a->visibility == vProtected)) && (a->name.size() > 0)) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        where->src << "    case " << uppername << ":\n";
                        if (a->isMultiple) {
                            where->src << "        " << localobject << a->name << ".Remove(valueindex);\n";
                        } else {
                            where->src << "        " << localobject << a->name << " = tObjectRef();\n";
                        }
                        where->src << "        break;\n";
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            DumpRemoveReferenceSwitch(where, a->Classifier, localobject+a->name+"[valueindex].", uppername+"_", aDoneList);
                        } else {
                            DumpRemoveReferenceSwitch(where, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    //
    //
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpRemoveReferenceSwitch(where, *target, localobject, prefix,  aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpRemoveReferenceSwitch(where, baselist.getElement(), localobject, prefix,  aDoneList);
        }
    }
}

//
//  this only initializes the membervalues with its DB infos. No init of var content done here.
void CSimObjectV2::DumpInitEmptyObject(std::ostream &output, std::shared_ptr<MElement> e, std::string localobject,
                                       std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        //
        //  Check if classifier name is empty. This should only happen on macros.
        if (!a->ClassifierName.empty()) {
            //
            //  uppername is the value id macro name
            std::string uppername=prefix+helper::toupper(a->name);
            //
            //  We are tracking these valueid name as we cannot allow duplicates in a single object.
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);
                //
                //  No modelled type used.
                if (!a->Classifier) {
                    if (a->visibility == vPublic) {
                        //
                        //  Try to get the dbattrtype from the classifier name.
                        std::string dbattrtype = MapDBAttrType(a->ClassifierName);
                        //
                        //  Single attribute or multiples.
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype <<   ", oid, " << uppername << ", 0);\n";
                            if (!a->defaultValue.empty()) {
                                output << "        " << localobject << "InitMember(" << uppername << ", 0, " << a->defaultValue << ");\n";
                            }
                        } else {
                            output << "        " << localobject << a->name << " = tMemberValueArray(" << dbattrtype <<   ", oid, " << uppername << ");\n";
                        }
                    } else if (a->visibility == vProtected) {
                        //
                        //  These attributes are mapped to tVariant.
                        //  We only need to init them if a default-Value has been defined.
                        if (!a->defaultValue.empty()) {
                            //
                            //  Single attribute or multiples.
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (IsVariantType(a->ClassifierName)) {
                                    //
                                    //  If it is a type that can be mapped to the tVariant we only have to pass it into the constructor
                                    output << "        " << localobject << a->name << " = " << a->defaultValue <<  ";\n";
                                } else {
                                    //
                                    //  We put a string into the simattr.
                                    output << "        " << localobject << a->name << " = \"" << a->defaultValue <<  "\";\n";
                                }
                            } else {
                                std::cerr << "No support for default values on tVariant maps. " << name << "::" << uppername << std::endl;
                            }
                        } else {
                            //
                            //  the tVariant constructor initializes the attribute. No further action needed.
                        }
                    } else if (a->visibility == vPrivate) {
                        //
                        //  The privates are un-changed types. So the are only getting a line here if they have a default value.
                        if  (!a->defaultValue.empty()) {
                            //
                            //  As we use plain C++ here initialization would be possible on any type if it defines some mechanism
                            //  for that. So we dump the code and wait what the compiler does on it.
                            output << "        " << localobject << a->name << " = " << a->defaultValue <<  ";\n";
                        }
                    }
                } else {
                    //
                    //  Linked to type. (Classifier)
                    //
                    //  Check the classifier reference

                    if (a->Classifier) {
                        if ((a->Classifier->IsClassBased()) && (a->Classifier->type != eElementType::Class)) {
                            //
                            //  For all public attributes.
                            if (a->visibility == vPublic) {
                                //
                                //  Try to get the dbattrtype from the classifier name.
                                std::string dbattrtype = MapDBAttrType(a->ClassifierName);
                                //
                                switch (a->Classifier->type) {
                                    case eElementType::SimObject:
                                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) ||
                                            (a->Multiplicity == "0..1") || (a->Multiplicity.empty())) {
                                            output << "        " << localobject << a->name << " = tMemberRef(oid, "
                                                   << uppername << ");\n";
                                        } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") ||
                                                   (a->Multiplicity == "*") || (!a->Multiplicity.empty())) {
                                            output << "        " << localobject << a->name
                                                   << " = tMemberRefArray(oid, " << uppername << ");\n";
                                        } else {
                                            std::cerr << "Weird multiplicity :" << a->Multiplicity << ":\n";
                                        }
                                        break;
                                    case eElementType::Struct:
                                        if (!a->isMultiple) {
                                            DumpInitEmptyObject(src, e, localobject, uppername + "_", aDoneList);
                                        }
                                        break;
                                    case eElementType::CxxClass:
                                    case eElementType::ExternClass:
                                        if (!dbattrtype.empty()) {
                                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) ||
                                                (a->Multiplicity == "0..1") || (a->Multiplicity.empty())) {
                                                output << "        " << localobject << a->name << " = tMemberValue("
                                                       << dbattrtype << ", oid, " << uppername << ", 0);\n";
                                            } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") ||
                                                       (a->Multiplicity == "*") || (!a->Multiplicity.empty())) {
                                                output << "        " << localobject << a->name
                                                       << " = tMemberValueArray(" << dbattrtype << ", oid, "
                                                       << uppername << ");\n";
                                            } else {
                                                std::cerr << "Weird multiplicity :" << a->Multiplicity << ":\n";
                                            }
                                        } else {
                                            std::cerr << "Do not know how to handle " << a->Classifier->name
                                                      << " in db\n";
                                        }
                                        break;
                                    case eElementType::Class:
                                        break;
                                    default:
                                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) ||
                                            (a->Multiplicity == "0..1") || (a->Multiplicity.empty())) {
                                            output << "        " << localobject << a->name << " = tMemberValue("
                                                   << dbattrtype << ", oid, " << uppername << ", 0);\n";
                                            if (!a->defaultValue.empty()) {
                                                output << "        " << localobject << "InitMember(" << uppername
                                                       << ", 0, " << a->defaultValue << ");\n";
                                            }
                                        } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") ||
                                                   (a->Multiplicity == "*") || (!a->Multiplicity.empty())) {
                                            output << "        " << localobject << a->name
                                                   << " = tMemberValueArray(" << dbattrtype << ", oid, "
                                                   << uppername << ");\n";
                                        } else {
                                            std::cerr << "Weird multiplicity :" << a->Multiplicity << ":\n";
                                        }
                                        break;
                                }
                            } else if (a->visibility == vProtected) {
                                //
                                //  These attributes are mapped to tVariant.
                                //  We only need to init them if a default-Value has been defined.
                                if (!a->defaultValue.empty()) {
                                    //
                                    //  Single attribute or multiples.
                                    if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) ||
                                        (a->Multiplicity == "0..1")) {
                                        if (IsVariantType(a->ClassifierName)) {
                                            //
                                            //  If it is a type that can be mapped to the tVariant we only have to pass it into the constructor
                                            output << "        " << localobject << a->name << " = "
                                                   << a->defaultValue << ";\n";
                                        } else {
                                            //
                                            //  We put a string into the simattr.
                                            output << "        " << localobject << a->name << " = \""
                                                   << a->defaultValue << "\";\n";
                                        }
                                    } else {
                                        std::cerr << "No support for default values on tVariant maps. " << name
                                                  << "::" << uppername << std::endl;
                                    }
                                } else {
                                    //
                                    //  the tVariant constructor initializes the attribute. No further action needed.
                                }
                            } else {
                                //
                                //  The privates are un-changed types. So the are only getting a line here if they have a default value.
                                if (!a->defaultValue.empty()) {
                                    //
                                    //  As we use plain C++ here initialization would be possible on any type if it defines some mechanism
                                    //  for that. So we dump the code and wait what the compiler does on it.
                                    output << "        " << localobject << a->name << " = " << a->defaultValue
                                           << ";\n";
                                }
                            }
                        }
                    } else {
                        std::cerr << "No objects stored for the classifier reference\n";
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/*(a->Navigable) && */(a->Classifier!=e) && (a->visibility == vPublic) && (a->name.size() >0)) {
            std::string uppername=prefix+helper::toupper(a->name);

            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            output << "        " << localobject << a->name << " = tMemberRef(oid, " << uppername << ");\n";
                        } else  if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            output << "        " << localobject << a->name << " = tMemberRefArray(oid, " << uppername << ");\n";
                        }
                        break;
                    case eElementType::SimEnumeration:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            output << "        " << localobject << a->name << " = tMemberValue(eMemberValueType::Int, oid, " << uppername << ", 0);\n";
                            if (!a->defaultValue.empty()) {
                                output << "        " << localobject << "InitMember(" << uppername << ", 0, " << a->defaultValue << ");\n";
                            }
                        } else  if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            output << "        " << localobject << a->name << " = tMemberValueArray(eMemberValueType::Int, oid, " << uppername << ");\n";
                        }
                        break;
                    case eElementType::Struct:
                        if (!a->isMultiple) {
                            DumpInitEmptyObject(src, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        if (a->Classifier->name != "string") {
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                output << "        " << localobject << a->name << " = 0;\n";
                            }
                        } else {
                            //
                            //  This is a classifier name string
                            if (!a->defaultValue.empty()) {
                                output << "        " << localobject << a->name << " = " << a->defaultValue << ";\n";
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    //
    //  Add attributes and associations from imported classes.
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpInitEmptyObject(src, *target, localobject, prefix, aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpInitEmptyObject(src, baselist.getElement(), localobject, prefix, aDoneList);
        }
    }
}

//
//  this initializes the membervalues with its DB infos and send them to the DB.
void CSimObjectV2::DumpInitDBObject(std::ostream &output, std::shared_ptr<MElement> e, std::string localobject,
                                    std::string prefix, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i: c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        //
        //  Check if classifier name is empty. This should only happen on macros.
        if (!a->ClassifierName.empty()) {
            //
            //  uppername is the value id macro name
            std::string uppername=prefix+helper::toupper(a->name);
            //
            //  We are tracking these valueid name as we cannot allow duplicates in a single object.
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);
                //
                //  No modelled type used.
                if (!a->Classifier) {
                    if (a->visibility == vPublic) {
                        //
                        //  Try to get the dbattrtype from the classifier name.
                        std::string dbattrtype = MapDBAttrType(a->ClassifierName);
                        //
                        //  Check if we have a default value.
                        if (a->defaultValue.empty()) {
                            //
                            //  Single attribute or multiples.
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype <<   ", oid, " << uppername << ", 0);\n";
                            } else {
                                output << "        " << localobject << a->name << " = tMemberValueArray(" << dbattrtype <<   ", oid, " << uppername << ");\n";
                            }
                        } else {
                            //
                            //  Single attribute or multiples.
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (IsVariantType(a->ClassifierName)) {
                                    //
                                    //  If it is a type that can be mapped to the tVariant we only have to pass it into the constructor
                                    output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype <<   ", oid, " << uppername << ", 0, " << a->defaultValue <<  ");\n";
                                } else {
                                    //
                                    //  We put a string into the simattr.
                                    output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype <<   ", oid, " << uppername << ", 0, \"" << a->defaultValue <<  "\");\n";
                                }
                            } else {
                                output << "        " << localobject << a->name << " = tMemberValueArray(" << dbattrtype <<   ", oid, " << uppername << ");\n";
                                std::cerr << "No support for default values on simattrarray. " << name << "::" << uppername << std::endl;
                            }
                        }
                    } else if (a->visibility == vProtected) {
                        //
                        //  These attributes are mapped to tVariant.
                        //  We only need to init them if a default-Value has been defined.
                        if (!a->defaultValue.empty()) {
                            //
                            //  Single attribute or multiples.
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (IsVariantType(a->ClassifierName)) {
                                    //
                                    //  If it is a type that can be mapped to the tVariant we only have to pass it into the constructor
                                    output << "        " << localobject << a->name << " = " << a->defaultValue <<  ";\n";
                                } else {
                                    //
                                    //  We put a string into the simattr.
                                    output << "        " << localobject << a->name << " = \"" << a->defaultValue <<  "\";\n";
                                }
                            } else {
                                std::cerr << "No support for default values on tVariant maps. " << name << "::" << uppername << std::endl;
                            }
                        } else {
                            //
                            //  the tVariant constructor initializes the attribute. No further action needed.
                        }
                    } else if (a->visibility == vPrivate) {
                        //
                        //  The privates are un-changed types. So the are only getting a line here if they have a default value.
                        if  (!a->defaultValue.empty()) {
                            //
                            //  As we use plain C++ here initialization would be possible on any type if it defines some mechanism
                            //  for that. So we dump the code and wait what the compiler does on it.
                            output << "        " << localobject << a->name << " = " << a->defaultValue <<  ";\n";
                        }
                    }
                } else {
                    //
                    //  Linked to type. (Classifier)
                    //
                    //  Check the classifier reference
                    if (a->Classifier) {
                        //
                        //  For all public attributes.
                        if (a->visibility == vPublic) {
                            //
                            //  Try to get the dbattrtype from the classifier name.
                            std::string dbattrtype = MapDBAttrType(a->ClassifierName);
                            //
                            switch (a->Classifier->type) {
                            case eElementType::SimObject:
                                if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1") || (a->Multiplicity.empty())) {
                                    output << "        " << localobject << a->name << " = tMemberRef(oid, " << uppername << ");\n";
                                } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (!a->Multiplicity.empty())) {
                                    output << "        " << localobject << a->name << " = tMemberRefArray(oid, " << uppername << ");\n";
                                } else {
                                    std::cerr << "Weird multiplicity :" << a->Multiplicity << ":\n";
                                }
                                break;
                            case eElementType::Struct:
                                if (!a->isMultiple) {
                                    DumpInitEmptyObject(src, e, localobject, uppername+"_", aDoneList);
                                }
                                break;
                            case eElementType::CxxClass:
                            case eElementType::ExternClass:
                                if (!dbattrtype.empty()) {
                                    if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1") || (a->Multiplicity.empty())) {
                                        if (a->defaultValue.empty()) {
                                            output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype << ", oid, " << uppername << ", 0);\n";
                                        } else {
                                            output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype << ", oid, " << uppername << ", 0, " << a->defaultValue << ");\n";
                                        }
                                    } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (!a->Multiplicity.empty())) {
                                        output << "        " << localobject << a->name << " = tMemberValueArray(" << dbattrtype<< ", oid, " << uppername << ");\n";
                                    } else {
                                        std::cerr << "Weird multiplicity :" << a->Multiplicity << ":\n";
                                    }
                                } else {
                                    std::cerr << "Do not know how to handle " << a->Classifier->name << " in db\n";
                                }
                                break;
                            case eElementType::Class:
                                break;
                            default:
                                if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1") || (a->Multiplicity.empty())) {
                                    if (a->defaultValue.empty()) {
                                        output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype << ", oid, " << uppername << ", 0);\n";
                                    } else {
                                        output << "        " << localobject << a->name << " = tMemberValue(" << dbattrtype << ", oid, " << uppername << ", 0, " << a->defaultValue << ");\n";
                                    }
                                } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (!a->Multiplicity.empty())) {
                                    output << "        " << localobject << a->name << " = tMemberValueArray(" << dbattrtype<< ", oid, " << uppername << ");\n";
                                } else {
                                    std::cerr << "Weird multiplicity :" << a->Multiplicity << ":\n";
                                }
                                break;
                            }
                        } else if (a->visibility == vProtected) {
                            //
                            //  These attributes are mapped to tVariant.
                            //  We only need to init them if a default-Value has been defined.
                            if (!a->defaultValue.empty()) {
                                //
                                //  Single attribute or multiples.
                                if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                    if (IsVariantType(a->ClassifierName)) {
                                        //
                                        //  If it is a type that can be mapped to the tVariant we only have to pass it into the constructor
                                        output << "        " << localobject << a->name << " = " << a->defaultValue <<  ";\n";
                                    } else {
                                        //
                                        //  We put a string into the simattr.
                                        output << "        " << localobject << a->name << " = \"" << a->defaultValue <<  "\";\n";
                                    }
                                } else {
                                    std::cerr << "No support for default values on tVariant maps. " << name << "::" << uppername << std::endl;
                                }
                            } else {
                                //
                                //  the tVariant constructor initializes the attribute. No further action needed.
                            }
                        } else {
                            //
                            //  The privates are un-changed types. So the are only getting a line here if they have a default value.
                            if  (!a->defaultValue.empty()) {
                                //
                                //  As we use plain C++ here initialization would be possible on any type if it defines some mechanism
                                //  for that. So we dump the code and wait what the compiler does on it.
                                output << "        " << localobject << a->name << " = " << a->defaultValue <<  ";\n";
                            }
                        }
                    } else {
                        std::cerr << "No objects stored for the classifier reference\n";
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if (/*(a->Navigable) && */(a->Classifier!=e) && (a->visibility == vPublic) && (a->name.size() >0)) {
            std::string uppername=prefix+helper::toupper(a->name);

            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            output << "        " << localobject << a->name << " = tMemberRef(oid, " << uppername << ");\n";
                        } else  if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            output << "        " << localobject << a->name << " = tMemberRefArray(oid, " << uppername << ");\n";
                        }
                        break;
                    case eElementType::SimEnumeration:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            if (a->defaultValue.empty()) {
                                output << "        " << localobject << a->name << " = tMemberValue(eMemberValueType::Int, oid, " << uppername << ", 0, 0);\n";
                            } else {
                                output << "        " << localobject << a->name << " = tMemberValue(eMemberValueType::Int, oid, " << uppername<< ", 0, " << a->defaultValue << ");\n";
                            }
                        } else  if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            if (a->defaultValue.empty()) {
                                output << "        " << localobject << a->name << " = tMemberValueArray(eMemberValueType::Int, oid, " << uppername << ");\n";
                            } else {
                                output << "        " << localobject << a->name << " = tMemberValueArray(eMemberValueType::Int, oid, " << uppername  << ");\n";
                            }
                        }
                        break;
                    case eElementType::Class:
                        break;
                    case eElementType::Struct:
                        if (!a->isMultiple) {
                            DumpInitEmptyObject(src, a->Classifier, localobject+a->name+".", uppername+"_", aDoneList);
                        }
                        break;
                    default:
                        if (a->Classifier->name != "string") {
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (a->defaultValue.empty()) {
                                        output << "        " << localobject << a->name << " = 0;\n";
                                } else {
                                        output << "        " << localobject << a->name << " = " << a->defaultValue << ";\n";
                                }
                            }
                        } else {
                            //
                            //  This is a classifier name string
                            if (!a->defaultValue.empty()) {
                                output << "        " << localobject << a->name << " = " << a->defaultValue << ";\n";
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    //
    //  Add attributes and associations from imported classes.
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpInitEmptyObject(src, *target, localobject, prefix, aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpInitEmptyObject(src, baselist.getElement(), localobject, prefix, aDoneList);
        }
    }
}

void CSimObjectV2::DumpCopyTemplate(std::ostream &output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix,
                                  std::string found, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string uppername=prefix+helper::toupper(a->name);

        if (aDoneList.find(uppername) == aDoneList.end()) {
            aDoneList.insert(uppername);

            if ((a->visibility==vPublic) || (a->visibility==vProtected)) {

                if (!a->Classifier) {
                    if (a->ClassifierName == "double") {
                        output << "            " << localobject << a->name << " = " << found << a->name << ";\n";
                    }
                } else {
                    if (a->Classifier) {
                        switch (a->Classifier->type) {
                        case eElementType::SimObject:
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                output << "            " << localobject << a->name << "=tSimObjRef{0, 0};\n";
                            } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            }
                            break;
                        case eElementType::Struct:
                            DumpCopyTemplate(src, a->Classifier, localobject, uppername+"_", found, aDoneList);
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            if (a->ClassifierName != "string") {
                                output << "            " << localobject << a->name << " = " << found << a->name << ";\n";
                            }
                            break;
                        }
                    }
                }
            } else {
                if (!a->Classifier) {
                    if (a->ClassifierName == "double") {
                        if (a->defaultValue.empty()) {
                            output << "            " << localobject << a->name << " = 0;\n";
                        } else {
                            output << "            " << localobject << a->name << " = " << a->defaultValue << ";\n";
                        }
                    } else if (a->ClassifierName == "bool") {
                        if (a->defaultValue.empty()) {
                            output << "            " << localobject << a->name << " = false;\n";
                        } else {
                            output << "            " << localobject << a->name << " = " << a->defaultValue << ";\n";
                        }
                    }

                } else {
                    auto e = *a->Classifier;

                    if (e!=0) {
                        switch (e->type) {
                        case eElementType::SimObject:
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (a->visibility==vPublic) {
                                    output << "            " << localobject << a->name << "=tSimObjRef{0, 0};\n";
                                } else {
                                    output << "            " << localobject << a->name << " = 0;\n";
                                }
                            } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
                            }
                            break;
                        case eElementType::Struct:
                            if (a->isMultiple) {
                                output << "            for (auto element : " << localobject << a->name << ") {\n";
                                    DumpCopyTemplate(src, a->Classifier, "    "+localobject+a->name+"[element.first].", uppername+"_", "element.second.", aDoneList);
                                output << "            }\n";
                            } else {
                                DumpCopyTemplate(src, a->Classifier, localobject+a->name+".", uppername+"_", found+a->name+".", aDoneList);
                            }
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            if (a->ClassifierName != "string") {
                                if (a->defaultValue.empty()) {
                                    if (a->ClassifierName == "pthread_spinlock_t") {
                                        output << "            pthread_spin_init(&" << localobject << a->name << ", PTHREAD_PROCESS_PRIVATE);\n";
                                    } else {
                                        output << "            " << localobject << a->name << " = 0;\n";
                                    }
                                } else {

                                    output << "            " << localobject << a->name << " = " << a->defaultValue << ";\n";
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if ((a->isNavigable()) && (a->Classifier!=e) && (a->visibility == vPublic) && (a->name.size() > 0)) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            output << "            " << localobject << a->name << "=tSimObjRef{0, 0};\n";
                        }
                        break;
                    case eElementType::Struct:
                        if (a->isMultiple) {
                            output << "            for (auto element : " << localobject << a->name << ") {\n";
                                DumpCopyTemplate(src, a->Classifier, "    "+localobject+a->name+"[element.first].", uppername+"_", "element.second.", aDoneList);
                            output << "            }\n";
                        } else {
                            DumpCopyTemplate(src, a->Classifier, localobject+a->name+".", uppername+"_", found+a->name+".", aDoneList);
                        }
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        if (a->Classifier->name != "string") {
                            output << "            " << localobject << a->name << " = " << found << a->name << ";\n";
                        }
                        break;
                    }
                }
            }
        }
    }
    //
    //  Add attributes and associations from imported classes.
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpCopyTemplate(src, *target, localobject, prefix, found, aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpCopyTemplate(src, baselist.getElement(), localobject, prefix, found, aDoneList);
        }
    }
}


void CSimObjectV2::DumpCopyFromTemplate(std::ostream &output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix,
                                      std::string found, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);
    std::string f;

    //f.assign(filler,' ');
    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string uppername=prefix+helper::toupper(a->name);

        if (aDoneList.find(uppername) == aDoneList.end()) {
            aDoneList.insert(uppername);
            if (a->visibility==vPublic) {
                if (!a->Classifier) {
                    std::string maptype=MapType(a);
                    //
                    //  If we got a maptype we could store it in the DB.
                    if (!maptype.empty()) {
                        output << f <<  "        setvaluedb(obj, " << uppername << ", 0, &" << found << a->name << ");\n";
                    }
                } else {
                    auto e = *a->Classifier;

                    if (e) {
                        switch (e->type) {
                        case eElementType::SimObject:
                            break;
                        case eElementType::Struct:
                            DumpCopyFromTemplate(src, e, localobject, uppername+"_", found+a->name+".", aDoneList);
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (a->ClassifierName != "string") {
                                    output << f << "        setvaluedb(obj, " << uppername << ", 0, &" << found << a->name << ");\n";
                                } else {
                                    output << f << "        setvaluedb(obj, " << uppername << ", 0, (void*)" << found << a->name << ".c_str());\n";
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if ((a->isNavigable()) && (a->Classifier!=e) && (a->visibility == vPublic) && (a->name.size() > 0)) {
            std::string uppername=prefix+helper::toupper(a->name);
            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        break;
                    case eElementType::Struct:
                        DumpCopyFromTemplate(src, a->Classifier, localobject+a->name+".", uppername+"_", found+a->name+".", aDoneList);
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            if (a->Classifier->name != "string") {
                                output << f << "        setvaluedb(obj, " << uppername << ", 0, &" << found << a->name << ");\n";
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    //
    //  Add attributes and associations from imported classes.
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpCopyFromTemplate(src, *target, localobject, prefix, found, aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpCopyFromTemplate(src, baselist.getElement(), localobject, prefix, found, aDoneList);
        }
    }
}

void CSimObjectV2::DumpInitAttributesInDB(std::ostream &output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix,
                                        int filler, std::set<std::string>& aDoneList) {
    auto c = std::dynamic_pointer_cast<MClass>(e);
    std::string f;

    f.assign(filler,' ');
    for (auto & i : c->Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string uppername=prefix+helper::toupper(a->name);

        if (aDoneList.find(uppername) == aDoneList.end()) {
            aDoneList.insert(uppername);

            if (a->visibility==vPublic) {
                if (!a->Classifier) {
                    std::string maptype = MapType(a);

                    if (!maptype.empty()) {
                        output << f <<  "        stdb_createdata(oid, " << uppername << ", 0, (" << maptype << ")(" << localobject << a->name << "));\n";
                    }
                } else {
                    auto e = a->Classifier;

                    if (e) {
                        switch (e->type) {
                        case eElementType::SimObject:
                            break;
                        case eElementType::Struct:
                            DumpInitAttributesInDB(src, e, localobject, uppername+"_", filler, aDoneList);
                            break;
                        case eElementType::Class:
                            break;
                        default:
                            if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                                if (a->ClassifierName != "string") {
                                    std::string maptype=MapType(a);
                                    if (maptype.empty()) {
                                        output << f << "        stdb_createdata(oid, " << uppername << ", 0, " << localobject << a->name << ");\n";
                                    } else {
                                        output << f << "        stdb_createdata(oid, " << uppername << ", 0, ("  << maptype << ")(" << localobject << a->name << "));\n";
                                    }
                                } else {
                                    output << f << "        stdb_createdata(oid, " << uppername << ", 0, ((std::string)(" << localobject << a->name << ")).c_str());\n";
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    //
    //  Here we process the associations ending at the class.
    for (auto & i : c->OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);
        //
        //  Only public association ends are used. This means that we do not have shared (pointers) ends.
        if ((a->isNavigable()) && (a->Classifier!=e) && (a->visibility == vPublic) && (a->name.size() > 0)) {
            std::string uppername=prefix+helper::toupper(a->name);

            if (aDoneList.find(uppername) == aDoneList.end()) {
                aDoneList.insert(uppername);

                if (a->Classifier) {
                    switch (a->Classifier->type) {
                    case eElementType::SimObject:
                        break;
                    case eElementType::Struct:
                        DumpInitAttributesInDB(src, a->Classifier, localobject+a->name+".", uppername+"_", filler, aDoneList);
                        break;
                    case eElementType::Class:
                        break;
                    default:
                        if ((a->Multiplicity == "1") || (a->Multiplicity.empty()) || (a->Multiplicity=="0..1")) {
                            if (a->Classifier->name != "string") {
                                std::string maptype=MapType(a);

                                if (maptype.empty()) {
                                    output << f << "        stdb_createdata(oid, " << uppername << ", 0, " << localobject << a->name << ");\n";
                                } else {
                                    output << f << "        stdb_createdata(oid, " << uppername << ", 0, ("  << maptype << ")(" << localobject << a->name << "));\n";
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    //
    //  Add attributes and associations from imported classes.
    for (auto & deplist : Dependency) {
        auto target = std::dynamic_pointer_cast<CDependency>(*deplist)->target;

        if ((deplist->HasStereotype("Import")) && (target->type == eElementType::SimObject)) {
            std::dynamic_pointer_cast<CSimObjectV2>(*target)->DumpInitAttributesInDB(src, *target, localobject, prefix, filler, aDoneList);
        }
    }

    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpInitAttributesInDB(src, baselist.getElement(), localobject, prefix, filler, aDoneList);
        }
    }
}

void CSimObjectV2::DumpEventTransition(std::shared_ptr<MState> compositestate, std::ostream &src, const std::string& evname) {
    for (auto & i : compositestate->States) {
        auto state  = std::dynamic_pointer_cast<CState>(*i);

        for (auto & tri : state->Outgoing) {
            auto trans = std::dynamic_pointer_cast<MTransition>(*tri);

            if ((!trans->events.empty()) && (trans->events[0]->name==evname)){
                //
                //  This is the event handling.
                std::string filler;

                if (!trans->guard.empty()) {
                    filler.assign(4, ' ');
                    src << "    if (" << trans->guard << ") {\n";
                }

                //SetUpdateFunctions(src, state, trans->to);
                if (!trans->guard.empty()) {
                    src << "    }\n";
                }
                //
                //  We transitioned so nothing else todo.
                return;
            } else {
            }
        }
        //
        //  Check for substates.
        if (!state->States.empty()) {
            DumpEventTransition(state, src, evname);
        }
    }
}



void CSimObjectV2::DumpEventTransition(std::ostream &src, const std::string& evname) {
    int x=0;

    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        if (sm->HasNormalStates()) {
            for (auto & s : sm->GetStates()) {
                auto state=std::dynamic_pointer_cast<MState>(*s);

                for (auto & tri : state->Outgoing) {
                    auto trans = std::dynamic_pointer_cast<MTransition>(*tri);

                    for (auto & tre : trans->events) {
                        if (tre->name == evname) {
                            //
                            //  This is the event handling.
                            std::string filler;

                            if (!trans->guard.empty()) {
                                filler.assign(4, ' ');
                                src << "    if (" << trans->guard << ") {\n";
                            }

                            src << "    " << filler << "state = e" << basename << "State" << "::" << helper::normalize( trans->to->name ) << ";\n";
                            for (auto & i : statelist) {
                                if (i==helper::tolower(trans->to->name)+"_state") {
                                    src << "    " << filler << "obj->state  = 0x" << std::hex << std::setw(16) << std::setfill('0') << x << ";\n";
                                    src << "    " << filler << "stdb_updatedata(obj->objid, IDA_STATE, 0, obj->state);\n";
                                    break;
                                }
                                x++;
                            }

                            if (!trans->guard.empty()) {
                                src << "    }\n";
                            }
                            //
                            //  We transitioned so nothing else todo.
                            return;
                        } else {
                        }
                    }
                }
                //
                //  Check for substates.
                if (!state->States.empty()) {
                    DumpEventTransition(state, src, evname);
                }
            }
        }
    } else {
    }
}

void CSimObjectV2::DumpStateFunctionPrototypes(std::ostream &output, std::shared_ptr<MState> st, std::string prefix) {

    for (auto & i : st->States) {
        auto state = std::dynamic_pointer_cast<MState>(*i);

        if (i->type == eElementType::State) {
            output << "    void " << prefix << "_" << helper::normalize(helper::tolower((*i)->name)) << "_state(uint64_t step);\n";
            statelist.push_back(prefix + "_" + helper::normalize(helper::tolower((*i)->name))+"_state");
            if (!state->States.empty()) {
                DumpStateFunctionPrototypes(output, state,  prefix+"_"+helper::normalize(helper::tolower((*i)->name)));
            }
        }
    }
}


void CSimObjectV2::DumpStateFunctions(std::ostream &output, std::shared_ptr<MState> st) {

    for (auto & i : st->States) {
        auto state = std::dynamic_pointer_cast<MState>(*i);

        if (state->type == eElementType::State) {
            std::string fncname=GetStateFncName(state);

            output << "/* **************************************************************************\n";
            output << " *\n";
            output << " *  Method-Name   : " << fncname << "_state()\n";
            output << " *\n";
            output << " *  Partial generated source code.\n";
            output << " *\n";
            output << " * *************************************************************************/\n";
            output << "void " << name << "::" << fncname << "_state(uint64_t aCycle) {\n";
            output << "// User-Defined-Code:" << state->id << "\n";
            output << "// End-Of-UDC:" << state->id << "\n";

            for (auto & tri : state->Outgoing) {
                auto trans = std::dynamic_pointer_cast<MTransition>(*tri);
                DumpTransition(output, trans, 0, "state");
            }
            output << "}\n";
            if (!state->States.empty()) {
                DumpStateFunctions(output, state);
            }
        }
    }

}

std::string CSimObjectV2::GetBaseClasses() {
    std::string retval;
    eVisibility v = vNone;

    if (!Generalization.empty()) {
        for (auto & gi :Generalization) {
            if (gi->visibility != v) {
                v = gi->visibility;
                auto b = std::dynamic_pointer_cast<MGeneralization>(*gi)->base;
//
//  remove the limitation to model items.
//  But maybe we do need the line as a reminder. So only commented out.
//                if ((gi->base != nullptr) && (gi->base->type != eElementType::SimObject)) {
                if (b) {
                    retval = ", ";
                    if (v==vPublic) {
                        retval+= "virtual public ";
                    } else if (v==vProtected) {
                        retval+= "virtual protected ";
                    } else if (v==vPrivate) {
                        retval+= "virtual private ";
                    }
                    retval += b->name;
                }
            }
        }
    }
    return retval;
}


bool CSimObjectV2::HaveSimObjectBase() {
    bool        retval = false;

    if (!Generalization.empty()) {
        for (auto & gi : Generalization) {
            auto b = std::dynamic_pointer_cast<MGeneralization>(*gi)->base;

            if (b) {
                if (b->type == eElementType::SimObject) {
                    retval = true;
                    break;
                }
            }
        }
    }
    return retval;
}


void CSimObjectV2::DumpReturnTypeForwards(std::ostream &output) {

    for (auto & i : Operation) {
        auto o = std::dynamic_pointer_cast<COperation>(*i);

        for (auto & pi : o->Parameter) {
            auto param = std::dynamic_pointer_cast<CParameter>(*pi);

            if (helper::tolower(param->Direction) == "return") {
                //
                //  Check for pointer type.
                if (param->ClassifierName.find_last_of("*") != std::string::npos) {
                    std::string cname = param->ClassifierName.substr(0, param->ClassifierName.find_last_of("*") );

                    auto ci = MClass::mByFQN.find(cname);

                    if (ci != MClass::mByFQN.end()) {
                        if ((ci->second->type == eElementType::CxxClass) || (ci->second->type == eElementType::SimObject) ||
                            (ci->second->type == eElementType::SimMessageClass)) {
                            output << "class " << cname << ";\n";
                        }
                    }
                }
            }
        }
    }
}

void CSimObjectV2::Dump(std::shared_ptr<MModel> model) {
    std::list<std::pair<std::string, std::string> >           alist;
    std::set<std::string>                                     donelist;
    std::set<std::string>                                     includesdone;
    size_t                                                    alistmax=0;
    std::set<std::shared_ptr<MElement>>                       msgdone;

    CollectNeededModelHeader(sharedthis<MElement>(), neededmodelheader);
    CollectNeededFromMessages(sharedthis<MElement>(), neededmodelheader);
    DumpBase(std::dynamic_pointer_cast<CModel>(model), name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, basename);

    DumpFileHeader(src, name, ".cpp");

    hdr << "\n#include <membertypes.h>\n\n";
    DumpPublicMacros(hdr);
    hdr << "//\n"
           "//  Forward declarations\n";
    bool dump = true;

    for (auto & f : forwards) {
        if (donelist.find(f->name) == donelist.end()) {
            if (f->type != eElementType::PrimitiveType) {
                if (dump) {
                    dump = false;
                    hdr << "//\n"
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
                hdr << "class " << fname << ";\n";
                //
                //  Add name to donelist to prevent us from doing things twice.
                donelist.insert(fname);
            }
        }
    }
    DumpMessageForwards(hdr);
    DumpReturnTypeForwards(hdr);
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        sm->DumpStateEnumerators(hdr, basename);
    }

    hdr << "//\n";
    hdr << "//                   S i m o b j e c t    d e c l a r a t i o n\n";
    hdr << "class " << name << " : virtual public tSimObj" << GetBaseClasses();
    hdr << " {\n"
           "public:\n";
    for (auto const& i : allEnds) {
        auto const a = std::dynamic_pointer_cast<CAssociationEnd>(i);
        //
        //  Add a constructor to set the parent type.
        //  Only if the name is set for the end.
        if ((a->name.empty()) && (a->isNavigable()) && (a->Classifier) && (a->Classifier->type == eElementType::SimObject)) {
            auto const parentclass = std::dynamic_pointer_cast<CSimObjectV2>(*a->Classifier);

            hdr << "    " << name << "() {parenttype = IDO_" << parentclass->upper_basename << ";}\n";
        }
    }
    //
    //  Add the virtual destructor. Its always there because we always have virtuals in the model item.
    hdr <<       "    virtual ~" << name << "();\n";
    //
    //  Create a list of all operations including the ones from import dependencies.
    auto io = GetImportOperation();

    for (auto const& i : io) {
        auto const o = std::dynamic_pointer_cast<COperation>(i);

        hdr << "    " << o->GetReturnType(NameSpace()) << " " << o->name << "(" << o->GetParameterDecl(NameSpace()) << ");\n";
    }
    hdr << "    /*\n"
           "     *  Getter/Setter\n"
           "     */\n"
           "    void SetValue(uint64_t vid, uint64_t vidx, const tVariant& value) override;\n"
           "    tVariant GetValue(uint64_t vid, uint64_t vidx = 0ul) override;\n\n"
           "    void SetReference(uint64_t vid, uint64_t vidx, const tObjectRef& ref) override;\n"
           "    void RemoveReference(uint64_t vid, uint64_t vidx) override;\n"
           "    tObjectRef GetReference(uint64_t vid, uint64_t vidx=0ul) override;\n\n"
           "    void InitMember(uint64_t vid, uint64_t vidx, const tVariant& value) override;\n"
           "    void SetParent(const tObjectRef& aParent) override;\n";

    hdr <<  "    /*\n"
            "     *  These are the message and signal processing functions.\n"
            "     */\n";
    hdr << "public:\n"
            "    tMsg* Process(std::shared_ptr<tMsg> aMsg) override;\n";
    if (MainViewPort) {
        hdr << "    void process(std::shared_ptr<tSignalStartCycle> aMsg);\n";
        hdr << "    void process(std::shared_ptr<tSignalEndCycle> aMsg);\n";
    }
    DumpMessageProcessingProto(hdr, msgdone);
    msgdone.clear();
    hdr << "    tMsg* DefaultMsgHandler(std::shared_ptr<tMsg> aMsg);\n"
           "    bool  DefaultSigHandler(std::shared_ptr<tSig> aSignal);\n";

    //
    //  Dump the statemachine processing funcions.
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        if (sm->HasNormalStates()) {
            hdr << "    /*\n";
            hdr << "     *  The state function prototypes.\n";
            hdr << "     */\n";
            for (auto & i : sm->GetStates()) {

                if (i->type == eElementType::State) {
                    hdr << "    void " << helper::normalize(helper::tolower(i->name)) << "_state(uint64_t step);\n";
                    statelist.push_back(helper::normalize(helper::tolower(i->name))+"_state");
                    if (!std::dynamic_pointer_cast<MState>(*i)->States.empty()) {
                        DumpStateFunctionPrototypes(hdr, std::dynamic_pointer_cast<MState>(*i),  helper::normalize(helper::tolower(i->name)));
                    }
                }
            }
        }
    }
    hdr << "    bool update(uint64_t aCycle) override;\n";
    hdr << "    /*\n"
           "     *  Here are the attributes of the object defined.\n"
           "     */\n"
           "public:\n";
    //
    //  Because the import of attributes moves the imported attributes into
    //  the scope of the simobject we need the pointers to the attributes
    //  to do the typemapping the right way.

    for (auto const& i : allAttr) {
        auto const a = std::dynamic_pointer_cast<CAttribute>(i);
        std::string cname = a->name;
        if (a->Classifier && !a->Classifier->name.empty()) {
            cname = a->Classifier->name;
        }
        //
        //  Only if the name is set and it is not derived.
        if ((!a->name.empty()) && (!a->isDerived)) {
            if (!(cname.empty())) {
                if (a->visibility == vPublic) {
                    alist.emplace_back( MkSimAttr(a), a->name);
                } else {
                    //
                    //  Any other visibility.
                    alist.emplace_back(MkVariant(a), a->name);
                }
            }
        }
    }
    //
    //  we no longer collect the derived attributes in the class.
#if 0
    auto deriveda = GetDerivedAttributes();
    for (auto const i : deriveda) {
        auto const a = (CAttribute*)i;
        //
        //  Only if the name is set and it is not derived.
        if ((a->name.size() > 0) && (!a->isDerived)) {
            if (!(a->ClassifierName.empty())) {
                if (a->visibility == vPublic) {
                    alist.emplace_back( std::pair<std::string, std::string>(MkSimAttr(a), a->name));
                } else /*if (a->visibility == vProtected) */ {
                    alist.emplace_back( std::pair<std::string, std::string>(MkVariant(a), a->name));
                } /*else {
                    alist.push_back( std::pair<std::string, std::string>(a->ClassifierName, a->name));
                }*/
            }
        }
    }
#endif
    //
    //  Add the attributes that come in from association ends.
    for (auto const & i : allEnds) {
        auto const a = std::dynamic_pointer_cast<CAssociationEnd>(i);
        //
        // only if the name is set for the end.
        if (!a->name.empty()) {
            bool foundentry = false;

            for (auto const& ilist : alist) {
                if (ilist.second == a->name) {
                    foundentry = true;
                }
            }
            if ((!foundentry) &&
            (a->Classifier) && (a->Classifier->IsClassBased()) && (a->Classifier->type != eElementType::Class)) {
                if (a->name == "constraint") {
                    std::cerr << "got constraint\n";
                }
                if (a->visibility == vPublic) {
                    alist.emplace_back( MkSimAttr(a), a->name);
                } else {
                    alist.emplace_back( MkVariant(a), a->name);
                }
            }
        }
    }
    //
    //  We no longer collect the derived associations.
#if 0
    for (auto const i : derived) {
        auto const a = (CAssociationEnd*)i;
        //
        // only if the name is set for the end.
        if (!a->name.empty()) {
            bool foundentry = false;

            for (auto const& ilist : alist) {
                if (ilist.second == a->name) {
                    foundentry = true;
                }
            }
            if (!foundentry) {
                if (a->visibility == vPublic) {
                    alist.emplace_back( std::pair<std::string, std::string>(MkSimAttr(a), a->name));
                } else {
                    alist.emplace_back( std::pair<std::string, std::string>(MkVariant(a), a->name));

                }
            }
        }
    }
#endif
    //
    //  Collect the names of all states to store from the statemachine.
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        sm->GetStateVars(alist, id_map, basename);
    }
#if 0
    for (auto const baselist : Base) {
        if (baselist->type == eElementType::SimObject) {
            auto morealist = baselist->GetDerivedAttributes();

            for (auto const i : morealist) {
                //
                // only if the name is set for the end.
                if (!i->name.empty()) {
                    bool foundentry = false;

                    for (auto const& ilist : alist) {
                        if (ilist.second == i->name) {
                            foundentry = true;
                        }
                    }
                    if (!foundentry) {
                        if (i->visibility == vPublic) {
                            alist.emplace_back( std::pair<std::string, std::string>(MkSimAttr((CAttribute*)i), i->name));
                        } else {
                            alist.emplace_back( std::pair<std::string, std::string>(MkVariant((CAttribute*)i), i->name));

                        }
                    }
                }
            }
        }
    }
#endif
    //
    //  Find the longest typename for alignment of dump.
    for (auto const & ilist : alist) {
        if (ilist.first.size() > alistmax) {
            alistmax = ilist.first.size();
        }
    }
    //
    //  Now dump the list of attributes of the simulation object.
    for (auto const& ilist : alist) {
        if (donelist.find(ilist.second) == donelist.end()) {
            std::string filler;
            filler.assign(alistmax - ilist.first.size()+1, ' ');
            hdr << "    " << ilist.first << filler << ilist.second << ";\n";
            donelist.insert(ilist.second);
        }
    }
    hdr << "};\n";
    hdr << "\n";    

    hdr << "extern tObjLib " << lower_basename << "_factory;\n";

    DumpGuardTail(hdr, basename);

    src << "//\n";
    src << "//                      S y s t e m   i n c l u d e s\n";
    src << "#include <stdint.h>\n";
    src << "#include <stdlib.h>\n";
    src << "#include <simifc.h>\n";
    src << "#include <simobjfactory.h>\n";
    src << "#include <simapi.h>\n";
    src << "#include <vector>\n";
    src << "#include <map>\n";
    if (MainViewPort) {
        src << "#include <tSignalStartCycle.h>\n";
        src << "#include <tSignalEndCycle.h>\n";
    }

    src << "#include \"generated.h\"\n";
    //
    //  Dump the extra includes.
    donelist.clear();
    //
    //  Fill donelist with hard coded headers.
    includesdone.insert("map");
    includesdone.insert("vector");
    includesdone.insert("simobjfactory.h");
    includesdone.insert("simapi.h");
    includesdone.insert("stdlib.h");
    includesdone.insert("stdint.h");
    includesdone.insert("simifc.h");
    includesdone.insert("generated.h");
    includesdone.insert("membertypes.h");
    if (MainViewPort) {
        includesdone.insert("tSignalStartCycle.h");
        includesdone.insert("tSignalEndCycle.h");
    }
    std::set<std::shared_ptr<MElement>>  doneclasses;
    DumpExtraIncludes(src, includesdone, doneclasses);
    //
    //  Dump the header files that are needed for correct compilation
    donelist.clear();

    src << "#include \"ids.h\"\n";
    if (HaveOperation("deleteconfirm")) {
        src << "#include \"tSigDeleteConfirm.h\"\n";
    }
    if (HaveOperation("deleteindication")) {
        src << "#include \"tSigDeleteIndication.h\"\n";
    }
    if (HaveOperation("deletereq")) {
        src << "#include \"tMsgDeleteReq.h\"\n";
    }
    if (HaveOperation("deletereply")) {
        src << "#include \"tMsgDeleteReply.h\"\n";
    }
    src << "//\n";
    src << "//                       M o d e l   i n c l u d e s\n";

    DumpNeededIncludes(src, sharedthis<CClassBase>(), donelist, doneclasses);
    src << "// Optional\n";
    doneclasses.clear();
    if (doDump) {
        std::cerr << "Optionals\n";
        for (auto & n : optionalmodelheader) {
            std::cerr << n->name << std::endl;
        }
        doDump = false;
    }
    DumpOptionalIncludes(src, sharedthis<CClassBase>(), donelist, doneclasses);

    if (statemachine) {
        auto sm=std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);
        std::list<std::string> transitions=sm->GetExternalTransitions();

        for (auto & et : transitions) {
            if (donelist.find(std::string("tMsg")+et) == donelist.end()) {
                src << "#include \"" << std::string("tMsg")+et << ".h\"\n";
                donelist.insert(std::string("tMsg")+et);
            }
        }
        transitions=sm->GetInternalTransitions();
        for (auto & et : transitions) {
            if (donelist.find(std::string("tSig") + et) == donelist.end()) {
                src << "#include \"" << std::string("tSig") + et << ".h\"\n";
                donelist.insert(std::string("tSig")+ et);
            }
        }
    }
    src << "/*\n"
           " *  private macros\n"
           " */\n";

    for (auto & i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);

        if ((a->ClassifierName.empty()) && (a->visibility!=vPublic)) {
            src << "#define " << a->name << " " << a->defaultValue << "\n";
        }
    }

    src << "/*\n"
           " * These are function prototypes for local object functions defined within \n"
           " * the simulation object.\n"
           " */\n"
           "/*\n"
           " *  The template storage map\n"
           " */\n"
           "static std::map<objectid_t, " << name << "*> t_store;\n"
           "/*\n"
           " *\n"
           " *       !!!!    Here is a collection of functions that are editable.   !!!!\n"
           " */\n";
    src << name << "::~" << name << "() {\n";
    src << "// User-Defined-Code: virtual destructor " << "-- " << name << std::endl;
    src << "// End-Of-UDC: virtual destructor " << "-- " << name << std::endl;
    src << "}\n";
    io = GetImportOperation();

    for (auto & i : io) {
        auto o = std::dynamic_pointer_cast<COperation>(i);

        o->DumpComment(src);
        src << "" << o->GetReturnType(NameSpace()) << " " << this->name << "::"<< o->name << "(" << o->GetParameterDefinition(NameSpace()) << ") {\n";
        if (o->GetReturnType(NameSpace()) != "void") {
            src << "    " << o->GetReturnType(NameSpace()) << " retval";
            std::string defvalue = o->GetReturnDefault();
            if (!defvalue.empty()) {
                src << " = " << defvalue;
            }
            src << ";\n";
        }
        //
        //  Dump the activity if the operation has one.
        if (o->Activity) {
            auto a = std::dynamic_pointer_cast<CActivity>(*o->Activity);
            a->DumpCxx(src);
            src << "// User-Defined-Code:" << o->id << "\n";
            src << "// End-Of-UDC:" << o->id << "\n";
        } else {
            src << "// User-Defined-Code:" << o->id << "\n";
            src << "// End-Of-UDC:" << o->id << "\n";
        }
        if (o->GetReturnType(NameSpace()) != "void") {
            src << "    return  (retval);\n";
        }
        src << "}\n";
    }
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        if (sm->HasNormalStates()) {
            src << "/*\n";
            src << " *  The state function implementation.\n";
            src << " */\n";
            for (auto & i : sm->GetStates()) {
                auto state=std::dynamic_pointer_cast<MState>(*i);
                //
                //  The state is anything that makes a processing.
                if (state->type == eElementType::State) {
                    src << "/* **************************************************************************\n";
                    src << " *\n";
                    src << " *  Method-Name   : " << helper::normalize(helper::tolower(state->name)) << "_state()\n";
                    src << " *\n";
                    src << " *  Partial generated source code.\n";
                    src << " *\n";
                    src << " * *************************************************************************/\n";
                    src << "void " << name << "::"<< GetStateFncName(state) << "_state(uint64_t aCycle) {\n";
                    src << "// User-Defined-Code:" << state->id << "--pre\n";
                    src << "// End-Of-UDC:" << state->id << "--pre\n";

                    for (auto & doact : state->DoActions) {
                        doact->DumpComment(src, 4);
                        src << "    " << doact->name << ";\n";
                    }

                    if (!state->States.empty()) {
                        DumpCompositeStatemachine(src, state);
                    }

                    for (auto & tri : state->Outgoing) {
                        auto trans=std::dynamic_pointer_cast<MTransition>(*tri);
                        DumpTransition(src, trans, 0, "State");
                    }
                    src << "// User-Defined-Code:" << state->id << "--post\n";
                    src << "// End-Of-UDC:" << state->id << "--post\n";
                    src << "}\n";
                    if (!state->States.empty()) {
                        DumpStateFunctions(src, state);
                    }
                }
            }
        }
    }
    //
    //  Dump the statemachine
    src << "/* **************************************************************************\n"
           " *\n"
           " *  Method-Name   : statemachine(uint64_t aCycle)\n"
           " *\n"
           " *  Partial generated source code.\n"
           " *\n"
           " * *************************************************************************/\n"
           "bool " << name << "::update(uint64_t aCycle) {\n";
           DumpStatemachine(src, "State");
    src << "}\n\n";

    //
    //
    //  Here we create the message and signal processing functions.
    donelist.clear();
    DumpMessageProcessingFunctions(sharedthis<CSimObjectV2>(), msgdone);
    DumpSaveFunction(src);
    src << "//\n";
    src << "//\n";
    src << "//       !!!!          End of editable section of functions.             !!!!\n";
    src << "//\n";
    //
    //
    //
    // setvalue_
    //
    //
    //
    DumpFunctionHeader(src, "InitMember", "", "");

    src << "void " << name << "::InitMember(uint64_t  valueid, uint64_t  valueindex, const tVariant& value) {\n";
    src << "    switch (valueid) {\n";
    src << "    case IDA_OBJ_PARENT:\n"
           "        parent = tObjectRef(value, nullptr);\n"
           "        break;\n";
    DumpSetStateSwitch(src);
    donelist.clear();
    DumpSetValueSwitch(sharedthis<CSimObjectV2>(), sharedthis<MElement>(), "", std::string("IDA_"), donelist);

    src << "    default:\n";
    src << "        break;\n";
    src << "    }\n";
    src << "}\n";

    DumpFunctionHeader(src, "SetParent", "", "");
    src << "void " << name << "::SetParent(const tObjectRef& aParent) {\n"
           "    if (parent != aParent) {\n"
           "        parent = aParent;\n"
           "        stdb_set((uint8_t)eMemberValueType::Reference, objid, IDA_OBJ_PARENT, 0, (uint64_t)(parent));\n"
           "    }\n"
           "}\n";

    DumpFunctionHeader(src, "GetValue", "", "");

    src << "tVariant " << name << "::GetValue(uint64_t  valueid, uint64_t  valueindex) {\n"
           "    tVariant retval;\n\n"
           "    switch (valueid) {\n";
    DumpGetStateSwitch(src);
    donelist.clear();
    DumpGetValueSwitch(sharedthis<CSimObjectV2>(), sharedthis<MElement>(), "", std::string("IDA_"), donelist);

    src << "    default:\n";
    src << "        break;\n";
    src << "    }\n";
    src << "    return (retval);\n";
    src << "}\n";

    //
    //
    //
    // setvalue
    //
    //
    //
    DumpFunctionHeader(src, std::string("SetValue"), "", "");

    src << "void " << name << "::SetValue(valueid_t  valueid, valueindex_t  valueindex, const tVariant& value) {\n";
    src << "    switch (valueid) {\n";
    DumpSetStateSwitch(src);
    donelist.clear();
    DumpSetValueDBSwitch(sharedthis<CSimObjectV2>(), sharedthis<MElement>(), "", std::string("IDA_"), donelist);

    src << "    default:\n";
    src << "        break;\n";
    src << "    }\n";
    src << "}\n";
    donelist.clear();

    DumpFunctionHeader(src, "GetReference", "", "");

    src << "tObjectRef " << name << "::GetReference(uint64_t  valueid, uint64_t  valueindex) {\n"
           "    tObjectRef retval;\n\n"
           "    switch (valueid) {\n";
    donelist.clear();
    DumpGetReferenceSwitch(sharedthis<CSimObjectV2>(), sharedthis<MElement>(), "", std::string("IDA_"), donelist);

    src << "    default:\n";
    src << "        break;\n";
    src << "    }\n";
    src << "    return (retval);\n";
    src << "}\n";

    //
    //
    //
    // setvalue
    //
    //
    //
    DumpFunctionHeader(src, std::string("SetReference"), "", "");

    src << "void " << name << "::SetReference(valueid_t  valueid, valueindex_t  valueindex, const tObjectRef& value) {\n";
    src << "    switch (valueid) {\n";
    donelist.clear();
    DumpSetReferenceSwitch(sharedthis<CSimObjectV2>(), sharedthis<MElement>(), "", std::string("IDA_"), donelist);

    src << "    default:\n";
    src << "        break;\n";
    src << "    }\n";
    src << "}\n";
    donelist.clear();

    DumpFunctionHeader(src, std::string("RemoveReference"), "", "");

    src << "void " << name << "::RemoveReference(valueid_t  valueid, valueindex_t  valueindex) {\n";
    src << "    switch (valueid) {\n";
    donelist.clear();
    DumpRemoveReferenceSwitch(sharedthis<CSimObjectV2>(), sharedthis<MElement>(), "", std::string("IDA_"), donelist);

    src << "    default:\n";
    src << "        break;\n";
    src << "    }\n";
    src << "}\n";

    donelist.clear();
    DumpDefaultSigHandler(src);
    DumpDefaultMsgHandler(src);
    DumpNewProcessMsg(src);

    DumpCreateObject(src);
    DumpCreateNewObject(src);
    DumpCreateNewObjectFromTemplate(src);
    src << "\ntObjLib " << lower_basename << "_factory= {0, 0, 0, IDO_" << upper_basename << ", create_" << lower_basename << "_obj, create_new_";
    src << lower_basename << "_obj, create_new_" << lower_basename << "_obj_from_template};\n";
    CloseStreams();
}

void CSimObjectV2::DumpStateHistory(std::ostream &output) {
    if (!statelist.empty()) {
        src << "        /*\n";
        src << "         * This is the shallowHistory functionality.\n";
        src << "         */\n";
        src << "        if (state >= (int)e" << basename << "State::LastState) {\n";
        src << "            state = 0;\n";
        src << "        }\n";
    } else {
        src << "        /*\n";
        src << "         * We found a shallow history state but no other states to switch to.\n";
        src << "         */\n";
        src << "        obj->update   = 0;\n";
    }
}

void CSimObjectV2::DumpCompositeStateEnum(std::ostream &output, std::string prefix, std::shared_ptr<MState> aState) {
    //
    //  Dump the state enumerator for the states that are defined within the statemachine.

    output << "enum class e" << prefix << aState->name << "State {\n";
    for (auto & st : aState->States) {
        if ((st->type == eElementType::PseudoState) && (helper::tolower((std::dynamic_pointer_cast<MPseudoState>(*st))->kind) == "shallowhistory")) {

        } else {
            output << "    " << helper::normalize((*st)->name) << ",\n";
        }
    }
    output << "    LastState\n};\n\n";

    //
    //  Now search for the composite states. These are detected because they have states defined
    for (auto & st : aState->States) {
        auto state = std::dynamic_pointer_cast<MState>(*st);

        if (!state->States.empty()) {
            DumpCompositeStateEnum(src, prefix+aState->name, state);
        }
    }
}

void CSimObjectV2::DumpStateEntry(std::ostream &output, std::shared_ptr<MState> st, int spacer) {
    if (!st->States.empty()) {
    } else {
    }
}

void CSimObjectV2::DumpStateExit(std::ostream &output, std::shared_ptr<MState> st, int spacer) {
}


void CSimObjectV2::DumpHistoryFromTransition(std::ostream& output, std::shared_ptr<MState> from, std::shared_ptr<MState> to, int spacer) {
    std::string stateclass;
    std::string filler;
    auto  up = to->parent;

    while (up) {
        switch (up->type) {
        case eElementType::SimStatemachine:
            //
            // Prepending the name of ourselfes.
            stateclass = basename+stateclass;
            up = MElementRef();
            break;
        case eElementType::State:
            stateclass = up->name+stateclass;
            up = up->parent;
            break;
        default:
            up = MElementRef();
            break;
        }
    }

    filler.assign(spacer, ' ');
//    output << filler << "    if (" << tostatefnc << "_state_idx >= (int)e" << basename << "State::LastState) {\n";
//    output << filler << "        " << tostatefnc << "_state_idx = " << stateidx << ";\n";
//    output << filler << "    }\n";


#if 0

    if (to->parent->type==eSimStatemachine) {
        std::string tostatefnc=GetStateFncName(to);
        MState *istate=GetInitialTargetState(to);
        int stateidx=0;

        if (istate != 0) {
            stateidx=GetStateIndex(istate);
        }

        if ((to->shallowHistory!=0)) {// && (from->parent!=to->parent)) {

            std::string uidx=std::string("IDA_")+helper::toupper(tostatefnc+"_state_idx");

            output << filler << "    /*\n";
            output << filler << "     * This is the shallowHistory functionality of a composite state.\n";
            output << filler << "     */\n";
            output << filler << "    if (" << tostatefnc << "_state_idx >= (int)e" << basename << "State::LastState) {\n";
            output << filler << "        " << tostatefnc << "_state_idx = " << stateidx << ";\n";
            output << filler << "    }\n";
        } else {
            if (!to->States.empty()) {
                output << filler << "        ((" << name << "*)obj)->" << tostatefnc << "_state_idx = " << stateidx << ";\n";
            }
        }
    } else if (to->parent->type == eState) {  //  Check for a composite state as parent.
        MState* tp=(MState*)to->parent;

        if ((to->shallowHistory!=0) && (from->parent != to->parent)) {
            std::string tostatefnc=GetStateFncName(to);
            output << "        /*\n";
            output << "         * This is the shallowHistory functionality.\n";
            output << "         */\n";
            output << "        if (" << tostatefnc << "_state_idx >= (int)e" << basename << "State::LastState) {\n";
            output << "        }\n";
            //
            //  This is a transition from an outer state into an inner state.
            DumpHistoryFromTransition(output, from, tp, spacer);
        } else {
        }
    }
#endif
}

std::string CSimObjectV2::GetStateClass(void) {
    std::string retval;

    return retval;
}




void CSimObjectV2::ChangeState(std::ostream& output, std::shared_ptr<MState> to, int spacer, std::string statevar) {
    std::string       filler;
    std::shared_ptr<CSimStatemachine> sm;
    std::shared_ptr<CState>           cs;
    std::shared_ptr<MElement>  down;

    filler.assign(spacer, ' ');
    std::shared_ptr<MElement>   up = to;

    while (up) {
        //
        //  Here we check what kind of object we need.
        switch (up->type) {
        case eElementType::SimStatemachine:
            //
            //  Check for a history.
            sm = std::dynamic_pointer_cast<CSimStatemachine>(up);
            output << filler << "state = " << sm->stateclass << "::" << helper::normalize(down->name) << ";\n";
            //
            // Prepending the name of ourselfes.
            up = std::shared_ptr<MElement>();
            break;
        case eElementType::State:
            cs = std::dynamic_pointer_cast<CState>(up);
            if (!cs->States.empty()) {
                if (up == to) {
                    down = cs->GetInitialState();
                }
                if (down) {
                    output << filler << cs->statevar << " = " << cs->stateclass << "::" << helper::normalize(down->name) << ";\n";
                }
            }
            down = up;
            up = up->parent;
            break;
        case eElementType::FinalState:
            down = up;
            up   = up->parent;
            break;
        default:
            up = std::shared_ptr<MElement>();
            break;
        }
    }
}

void CSimObjectV2::DumpTransitionAction(std::ostream& output, std::shared_ptr<MTransition> trans, int spacer) {
    std::string filler;

    filler.assign(spacer, ' ');

    for (auto & i : trans->actions) {
        auto a = std::dynamic_pointer_cast<MAction>(*i);
        a->DumpComment(output, spacer+4);
        output << filler << "    " << a->name << ";\n";
    }
}
void CSimObjectV2::DumpTransition(std::ostream& output, std::shared_ptr<MTransition> trans, int spacer, std::string statevar) {
    std::string filler;

    if (trans->events.empty()) {
        //
        //  Dump the comment.
        trans->DumpComment(output, spacer+4);
        //
        //  Generate Guard
        if (!trans->guard.empty()) {
            filler.assign(spacer, ' ');
            output << filler << "    if (" << trans->guard << ") {\n";
            spacer+=IndentSize;
        }
        //
        //  First we create all history read-backs.
        DumpHistoryFromTransition(output, std::dynamic_pointer_cast<MState>(*trans->from), std::dynamic_pointer_cast<MState>(*trans->to), spacer);
        //
        //  Then we create the update functions.
        ChangeState(output, std::dynamic_pointer_cast<MState>(*trans->to), spacer+4, statevar);
        //
        //  Dump old state exit
        DumpStateExit(output, std::dynamic_pointer_cast<MState>(*trans->from), spacer);
        //
        // Dump transition action
        DumpTransitionAction(output, trans, spacer);
        //
        // Dump new state entry;
        DumpStateEntry(output, std::dynamic_pointer_cast<MState>(*trans->to), spacer);
        //
        //  Terminate guard
        if (!trans->guard.empty()) {
            if (spacer >=IndentSize) {
                spacer-=IndentSize;
            } else {

            }
            filler.assign(spacer, ' ');
            output << filler << "    }\n";

        }
    } else {
    }

}


void CSimObjectV2::DumpStatemachine(std::ostream &output, std::string statevar) {
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);
        output << "    bool retval = true;\n\n";
        //
        //  Use the object state as the state var.
        output << "    switch(static_cast<uint64_t>(state)) {\n";
        //
        //  Add the state calls here.
        for (auto & st : sm->states) {
#if 0
            //
            //  First thing to do in a state is to run the do-actions.
            std::vector<MAction*>::iterator     doact;

            for (doact=state->DoActions.begin(); doact!=state->DoActions.end(); ++doact) {
                (*doact)->DumpComment(output, 4);
                output << "        " << (*doact)->name << ";\n";
            }
#endif
            //
            //  Now check if it is a composite state.
            //  Then we run its transition processing.
            if (st->type == eElementType::State) {
                output << "    case e" << basename << "State::" << helper::normalize(st->name) << ":\n";
    #if 0
                if ((!state->States.empty() || (state->DoActions.empty()))) {
                    output << "        " << helper::tolower(helper::normalize(state->name)) << "_state(aCycle);\n";
                } else {
                    std::vector<MTransition*>::iterator ot;
                    for (ot = (*st)->Outgoing.begin(); ot != (*st)->Outgoing.end(); ++ot) {
                        DumpTransition(output, *ot, 4, statevar);
                    }
                }
    #endif
                output << "        " << helper::tolower(helper::normalize(st->name)) << "_state(aCycle);\n";
                output << "        break;\n";
            } else if (st->type == eElementType::FinalState) {
                output << "    case e" << basename << "State::" << helper::normalize(st->name) << ":\n";
                output << "        state  = UINT64_MAX;\n"
                          "        retval = false;\n";
                output << "        break;\n";
            } else if (st->type == eElementType::PseudoState) {
                auto ps = std::dynamic_pointer_cast<CPseudoState>(*st);

                if (helper::tolower(ps->kind) == "initial") {
                    output << "    case e" << basename << "State::" << helper::normalize(st->name) << ":\n";
                    //
                    //  From an initial state we may have multiple outgoing transitions.
                    for (auto & ot : ps->Outgoing) {
                        DumpTransition(output, std::dynamic_pointer_cast<MTransition>(*ot), 4, statevar);
                    }
                    output << "        break;\n";
                }
            }
        }
        output << "    default:\n"
                  "         retval = false;\n"
                  "         break;\n"
                  "    }\n"
                  "    return retval;\n";
    } else {
        output << "   bool nextupdate = false;\n";
        output << "// User-Defined-Code: " << id << "-update\n"
                  "// End-Of-UDC: " << id << "-update\n";
        output << "   return nextupdate;\n";
    }
}

void CSimObjectV2::DumpCompositeStatemachine(std::ostream &output, std::shared_ptr<MState> aState) {

    if (aState->type == eElementType::State) {
        auto instate = std::dynamic_pointer_cast<CState>(aState);

        output << "    switch(static_cast<uint64_t>(" << instate->statevar << ")) {\n";

        for (auto & st : instate->States) {
            auto state = std::dynamic_pointer_cast<CState>(*st);

            if ((st->type != eElementType::PseudoState) || (state->isInitial())) {
                output << "    case " << instate->stateclass << "::" << helper::normalize(state->name) << ":\n";
                //
                //  First thing to do in a state is to run the do-actions.
                for (auto & doact : state->DoActions) {
                    doact->DumpComment(output, 4);
                    output << "    " << doact->name << ";\n";
                }
                //
                //  Now check if it is a composite state.
                //  Then we run its transition processing.
                if (!state->States.empty()) {
                    output << "        " << helper::tolower(helper::normalize(state->name)) << "_state(aCycle);\n";
                }

                for (auto & ot : state->Outgoing) {
                    DumpTransition(output, std::dynamic_pointer_cast<MTransition>(*ot), 4, instate->statevar);
                }
                output << "        break;\n";
            }
        }
        output << "    default:\n"
                  "         break;\n"
                  "    }\n";
    }
}

void CSimObjectV2::DumpDefaultSigHandler(std::ostream& src) {
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : DefaultSigHandler()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "bool " << name << "::DefaultSigHandler(std::shared_ptr<tSig> sig) {\n";
    src << "    bool retval = false;\n\n"
           "    switch (sig->id) {\n"
           "// User-Defined-Code: default-sig-handler\n"
           "// End-Of-UDC: default-sig-handler\n"
           "    default:\n"
           "        simlog().Write(LogLevel::Warn, \"Did not process signal. Object-Type: ";
    src << basename << ": Signal :%8.8lX:\", sig->id);\n"
           "        break;\n"
           "    }\n"
           "    return (retval);\n"
           "}\n\n";

}

void CSimObjectV2::DumpDefaultMsgHandler(std::ostream& src) {
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : DefaultMsgHandler()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "tMsg* " << name << "::DefaultMsgHandler(std::shared_ptr<tMsg> msg) {\n";
    src << "    tMsg* retval = 0;\n\n"
           "    switch (msg->id) {\n"
           "// User-Defined-Code: default-msg-handler\n"
           "// End-Of-UDC: default-msg-handler\n"
           "    default:\n"
           "        simlog().Write(LogLevel::Warn, \"Did not process message. Object-Type: ";
    src << basename << ": Msg-Id :%8.8lX:\", msg->id);\n"
           "        break;\n"
           "    }\n"
           "    return retval;\n"
           "}\n\n";
}


void CSimObjectV2::DumpNewProcessMsg(std::ostream& src) {
    std::set<std::string> done;
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : process_msg()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src <<  "tMsg* " << name << "::Process(tMsgPtr msg) {\n"
            "    tMsg *retmsg=0;\n\n";

    src <<  "//\n//  A quick out.\n"
            "    if (msg == nullptr) return nullptr;\n\n"
            "    switch (msg->id) {\n";

    DumpProcessMsgSwitch(src, done);
    DumpProcessSigSwitch(src, done);

    if (HaveOperation("deletereq")) {
        src << "    case IDM_DELETEREQ:\n";
        src << "        thisobj->process(std::static_pointer_cast<tMsgDeleteReq>(msg));\n";
        src << "        break;\n";
    }
    if (HaveOperation("deletereply")) {
        src << "    case IDM_DELETEREPLY:\n";
        src << "        thisobj->process( std::static_pointer_cast<tMsgDeleteReply>(msg));\n";
        src << "        break;\n";
    }
    src << "    default:\n";
    src << "        if (((msg->type == MSG_TYPE_REPLY) || (msg->type == MSG_TYPE_INDICATION)) && (parent.valid()) && (*parent != this)) {\n";
    src << "            retmsg = parent->Process(msg);\n";
    src << "        } else {\n";
    src << "            retmsg = DefaultMsgHandler(msg);\n";
    src << "        }\n";
    src << "        break;\n";
    src << "    }\n";

    src << "    return retmsg;\n";
    src << "}\n";
}


void CSimObjectV2::DumpCreateObject(std::ostream& src) {
    std::set<std::string>  done;
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : create_" << lower_basename << "_obj()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "static tSimObj* create_" << lower_basename << "_obj(objectid_t  oid, uint64_t droptime) {\n";
    src << "    " << name << "* new" << lower_basename << " = new " << name << ";\n\n";

    src << "    if (new" << lower_basename << " != 0) {\n"
           "        tSimObj* newobj = new" << lower_basename << ";\n\n"
           "        newobj->type        = IDO_" << upper_basename << ";\n"
           "        newobj->objid       = oid;\n"
           "        newobj->save        = save;\n"
           "        newobj->droptime    = droptime;\n\n"
           "        if (0xc0000000 & oid) {\n"
           "            t_store.insert(std::pair<templateid_t, " << name << "*>(oid, new" << lower_basename << "));\n"
           "        }\n";
    //
    //  Add the state attribute for the toplevel statemachine
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        auto initstate = sm->GetInitialState();

        if (initstate) {
            src << "        new" << lower_basename <<"->state = "<< sm->stateclass << "::" << helper::normalize(initstate->name) << ";\n";
        }

        for (auto & st : sm->states) {
            if (st->type == eElementType::State) {
                DumpStateInit(src, std::dynamic_pointer_cast<MState>(*st), 8);
            }

        }
    }
    DumpInitEmptyObject(src, sharedthis<MElement>(), std::string("new")+lower_basename+"->", std::string("IDA_"), done);

    src << "    } else {\n";
    src << "    }\n";
    src << "    return ((tSimObj*)new" << lower_basename << ");\n";
    src << "}\n";
}

void CSimObjectV2::DumpStateInit(std::ostream &output, std::shared_ptr<MState> aState, int spacer) {
    std::string filler;

    filler.assign(spacer, ' ');

    if (!aState->States.empty()) {
        auto cs = std::dynamic_pointer_cast<CState>(aState);
        auto initstate = cs->GetInitialState();

        if (cs->shallowHistory) {
            src << filler << "new" << lower_basename <<"->" << cs->statevar << " = tMemberValue(oid, IDA_" << helper::toupper(helper::normalize(cs->statevar)) << ", ((uint64_t)(" << cs->stateclass << "::" << helper::normalize(initstate->name) << ")));\n";
        } else {
            if (initstate) {
                src << filler << "new" << lower_basename <<"->" << cs->statevar << " = "<< cs->stateclass << "::" << helper::normalize(initstate->name) << ";\n";
            }
        }

        for (auto & st : cs->States) {
            if (st->type == eElementType::State) {
                DumpStateInit(src, std::dynamic_pointer_cast<MState>(*st), 8);
            }

        }
    }
}

void CSimObjectV2::DumpCreateNewObject(std::ostream& src) {
    std::set<std::string> done;
    src << "// **************************************************************************\n"
           "//\n"
           "//  Method-Name   : create_new_" << lower_basename << "_obj()\n"
           "//\n"
           "//  Generated source code.\n"
           "//\n"
           "// **************************************************************************\n"
           "static tSimObj* create_new_" << lower_basename << "_obj(objectid_t  oid) {\n"
           "    " << name << "* new" << lower_basename << " = new " << name << ";\n\n"
           "    if (new" << lower_basename << " != 0) {\n"
           "        tSimObj* newobj = new" << lower_basename << ";\n\n"
           "        newobj->type        = IDO_" << upper_basename << ";\n"
           "        newobj->objid       = oid;\n"
           "        newobj->save        = save;\n";
    if (ReleaseTimeout != UINT64_MAX) {
        src << "        newobj->droptime = (" << ReleaseTimeout << ");\n";
    }

    //
    //  Add the state attribute for the toplevel statemachine
    if (statemachine) {
        auto sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        auto initstate = sm->GetInitialState();

        if (initstate) {
            src << "        new" << lower_basename <<"->state = "<< sm->stateclass << "::" << helper::normalize(initstate->name) << ";\n";
        }

        for (auto & st : sm->states) {
            if (st->type == eElementType::State) {
                DumpStateInit(src, std::dynamic_pointer_cast<MState>(*st), 8);
            }
        }
    }
    src << "        //\n"
           "        //  Create the object in the db.\n"
           "        stdb_createobj(oid, IDO_" << upper_basename <<  ", newobj->droptime);\n"
           "        //\n"
           "        //  Now fill the attributes with values.\n";

    DumpInitDBObject(src, sharedthis<MElement>(), std::string("new")+lower_basename+"->", std::string("IDA_"), done);

    src << "        //\n"
           "        //  create the attribute data in the DB.\n"
           "    } else {\n"
           "    }\n"
           "    return ((tSimObj*)new" << lower_basename << ");\n"
           "}\n";
}

void CSimObjectV2::DumpSaveFunction(std::ostream & src) {
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : save()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "static int save(tSimObj* obj, uint64_t aCycle, ... ) {\n";
    src << "    int result = 0;\n";
    src << "// User-Defined-Code: " << id << "-save-" << lower_basename << "\n";
    src << "// End-Of-UDC: " << id << "-save-" << lower_basename << "\n";
    src << "    return (result);\n";
    src << "}\n\n";
}

void CSimObjectV2::DumpCopyFromTemplate(std::ostream& src) {
    std::set<std::string> done;
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : copy_from_template()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "static void copy_from_template(tSimObj* obj, templateid_t  tid) {\n";
    src << "    " << name << "* var" << lower_basename << " = ("<< name << "*)obj;\n";
    src << "    std::map<objectid_t, " << name << "*>::iterator found;\n\n";

    src << "    found = t_store.find(tid);\n";
    src << "    if (found != t_store.end()) {\n";
    DumpCopyFromTemplate(src, sharedthis<MElement>(), std::string("var")+lower_basename+"->", std::string("IDA_"),
                         std::string("found->second->"), done);
    src << "    }\n";
    src << "}\n";
}

void CSimObjectV2::DumpCreateNewObjectFromTemplate(std::ostream& src) {
    std::set<std::string> done;
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : create_new_" << lower_basename << "_obj_from_template()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "static tSimObj* create_new_" << lower_basename << "_obj_from_template(templateid_t  tid, objectid_t  oid) {\n";
    src << "    " << name << "* new" << lower_basename << " = 0;\n";
    src << "    std::map<objectid_t, " << name << "*>::iterator found;\n\n";

    src << "    found = t_store.find(tid);\n";
    src << "    if (found != t_store.end()) {\n";
    src << "        new" << lower_basename << " = new " << name << ";\n";

    src << "        if (new" << lower_basename << " != 0) {\n";
    src << "            tSimObj* newobj = new" << lower_basename << ";\n\n";
    src << "            newobj->type        = IDO_" << upper_basename << ";\n";
    src << "            newobj->objid       = oid;\n";
    src << "            newobj->save        = save;\n"
           "            newobj->droptime    = found->second->droptime;\n";
    //
    //  Add the state attribute for the toplevel statemachine
    if (statemachine) {
        auto  sm = std::dynamic_pointer_cast<CSimStatemachine>(*statemachine);

        auto initstate = sm->GetInitialState();

        if (initstate) {
            src << "            new" << lower_basename <<"->state = "<< sm->stateclass << "::" << helper::normalize(initstate->name) << ";\n";
        }

        for (auto & st : sm->states) {
            if (st->type == eElementType::State) {
                DumpStateInit(src, std::dynamic_pointer_cast<MState>(*st), 12);
            }

        }
    }
    src << "            //\n"
           "            //  Create the object in the db.\n"
           "            stdb_createobj(oid, IDO_" << upper_basename << ", newobj->droptime);\n"
           "            //\n";

    DumpInitEmptyObject(src, sharedthis<MElement>(), std::string("    new")+lower_basename+"->", std::string("IDA_"), done);

    //done.clear();
    src << "            //\n";
    src << "            //  Copy data from template.\n";

    DumpCopyTemplate(src, sharedthis<MElement>(), std::string("new")+lower_basename+"->", std::string("IDA_"), std::string("found->second->"), done);

    src << "            //\n";
    src << "        } else {\n";
    src << "        }\n";
    src << "    } else {\n";
    src << "    }\n";
    src << "    return ((tSimObj*)new" << lower_basename << ");\n";
    src << "}\n";
}

void CSimObjectV2::DumpInitNew(std::ostream& src) {
    src << "// **************************************************************************\n";
    src << "//\n";
    src << "//  Method-Name   : create_new_" << lower_basename << "_obj_from_template()\n";
    src << "//\n";
    src << "//  Generated source code.\n";
    src << "//\n";
    src << "// **************************************************************************\n";
    src << "static tSimObj* create_new_" << lower_basename << "_obj_from_template(templateid_t  tid, objectid_t  oid) {\n";
}

void CSimObjectV2::DumpMessageProcessingProto(std::ostream & src, std::set<std::shared_ptr<MElement>> aDoneList) {
    auto  i = GetImportIncoming();

    for (auto & mi : i) {
        auto mic = std::dynamic_pointer_cast<CMessage>(mi);

        if (mic->m_implementation) {
            if (aDoneList.find(mic->m_implementation) == aDoneList.end()) {
                aDoneList.insert(mic->m_implementation);
                auto s = mic->m_implementation->sharedthis<CClassBase>();

                if ((mic->isMessage()) && (mic->mtype != enumMessageType::mReply)) {
                    if (!HaveProcessOperation("process", "std::shared_ptr<" + s->name + ">")) {
                        src << "    virtual tMsg* process(std::shared_ptr<" << s->name << "> msg);\n";
                    }
                } else if (mic->isSignal()) {
                    if (!HaveProcessOperation("process", "std::shared_ptr<" + s->name + ">")) {
                        src << "    virtual void process(std::shared_ptr<" << s->name << "> sig);\n";
                    }
                } else {
                    //
                    // Dont know what to do with that.
                }
            }
        }
    }
    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpMessageProcessingProto(src, aDoneList);
        }
    }
}

void CSimObjectV2::DumpMessageForwards(std::ostream & file) {
    //
    //  We want to be namespace aware as in CxxClasses.
    NameSpaceNode nstree;
    //
    //  Fetch all incoming messages.
    auto i = GetImportIncoming();
    //
    //  Here we create the forward declarations for the incoming messages
    //  Even the imported classes are used for that as well.
    std::set<std::shared_ptr<MMessage>> haveit;
    //
    //  In the mainviewport we have always two signals that are not modeled.
    if (MainViewPort) {
        file << "class tSignalStartCycle;\n"
                "class tSignalEndCycle;\n";
    }
    //  Get along the incoming messages.
    for (auto & mi : i) {
        //
        //  Check if we have the message already processed.
        if (haveit.find(mi) == haveit.end()) {
            haveit.insert(mi);
            auto mic = std::dynamic_pointer_cast<CMessage>(mi);
            //
            //  If there is a signature attached we need to use this.
            //  Operation signatures are ignored for now. Maybe later this comes to play.
            if ((mic->m_implementation) && (((mic->isMessage()) && (mic->mtype != enumMessageType::mReply)) ||
                (mic->isSignal()))) {
                nstree.add(mic->m_implementation->sharedthis<CClassBase>()->mNameSpace.get(), mic->m_implementation->sharedthis<CClassBase>());
            } else {
            }
        }
    }
    nstree.dump(file);
}

#if 0
void CSimObjectV2::DumpMessageIncludes(std::ostream &src, std::set<std::string>& aDoneList, std::set<std::string>& aIncludesDone) {
    std::vector<MMessage*>           i = GetImportIncoming();
    std::vector<MMessage*>           o = GetImportOutgoing();
    std::vector<MMessage*>::iterator mi=i.begin();
    std::vector<MMessage*>::iterator mo=o.begin();
    std::set<MElement*>              ldone;  // local done list.

    for (; mi!=i.end(); ++mi) {
        if ((*mi)->type == eElementType::SimMessage) {
            CSimMessage   *s=(CSimMessage*)(*mi);
            CMessageClass *sc=(CMessageClass*)s->GetClass();
            if (sc != 0) {
                if (aDoneList.find(sc->name)==aDoneList.end()) {
                    std::list<MElement*>::const_iterator inc;

                    for (inc=sc->GetNeededHeader().begin(); inc != sc->GetNeededHeader().end(); ++inc) {
                        ((CClassBase*)*inc)->DumpNeededIncludes(src, this, aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming Simulation-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if ((*mi)->type == eElementType::JSONMessage) {
            CJSONMessage  *s=(CJSONMessage*)(*mi);
            CMessageClass *sc=(CMessageClass*)s->GetClass();
            if (sc != 0) {
                if (aDoneList.find(sc->name)==aDoneList.end()) {
                    src << "#include \"" << sc->name << ".h\"\n";
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming JSON-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if ((*mi)->type == eElementType::SimSignal) {
            CSimSignal  *s=(CSimSignal*)(*mi);
            CSignalClass *sc=(CSignalClass*)s->GetClass();
            if (sc != 0) {
                if (aDoneList.find(sc->name)==aDoneList.end()) {
                    std::list<MElement*>::const_iterator inc;

                    for (inc=sc->GetNeededHeader().begin(); inc != sc->GetNeededHeader().end(); ++inc) {
                        ((CClassBase*)*inc)->DumpNeededIncludes(src, this, aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming Simulation-Signal:" << s->name << ": in SimObject:" << name << ":\n";
            }
        }
    }


    for (; mo!=o.end(); ++mo) {
        if ((*mo)->type == eElementType::SimMessage) {
            CSimMessage   *s=(CSimMessage*)(*mo);
            CMessageClass *sc=(CMessageClass*)s->GetClass();
            if (sc != 0) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    std::list<MElement*>::const_iterator inc;

                    for (inc=sc->GetNeededHeader().begin(); inc != sc->GetNeededHeader().end(); ++inc) {
                        ((CClassBase*)*inc)->DumpNeededIncludes(src, this, aIncludesDone, ldone);
                        if ((aIncludesDone.find((*inc)->name)==aIncludesDone.end())   && ((*inc)->type != eElementType::ExternClass)) {
//                            src << "#include \"" << (*inc)->name << ".h\"\n";
                            aIncludesDone.insert((*inc)->name);
                        }
                    }
                    aDoneList.insert(sc->name);
                }
            } else {
                std::cerr << "No Class definition found for outgoing Simulation-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if ((*mo)->type == eElementType::JSONMessage) {
            CJSONMessage  *s=(CJSONMessage*)(*mo);
            s->FillClass();
            CMessageClass *sc=(CMessageClass*)s->GetClass();
            if (sc != 0) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    std::list<MElement*>::const_iterator inc;

                    for (inc=sc->GetNeededHeader().begin(); inc != sc->GetNeededHeader().end(); ++inc) {
                        ((CClassBase*)*inc)->DumpNeededIncludes(src, this, aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                }
            } else {
                std::cerr << "No Class definition found for outgoing JSON-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if ((*mo)->type == eElementType::SimSignal) {
            CSimSignal  *s=(CSimSignal*)(*mo);
            CSignalClass *sc=(CSignalClass*)s->GetClass();
            if (sc != 0) {
                if (aDoneList.find(sc->name)==aDoneList.end()) {
                    std::list<MElement*>::const_iterator inc;

                    for (inc=sc->GetNeededHeader().begin(); inc != sc->GetNeededHeader().end(); ++inc) {
                        ((CClassBase*)*inc)->DumpNeededIncludes(src, this, aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for outgoing Simulation-Signal:" << s->name << ": in SimObject:" << name << ":\n";
            }
        }
    }

    //
    //
    for (std::list<CClassBase*>::iterator baselist=Base.begin(); baselist!=Base.end(); ++baselist) {
        if ((*baselist)->type == eElementType::SimObject) {
            ((CSimObjectV2*)(*baselist))->DumpMessageIncludes(src, aDoneList, aIncludesDone);
        }
    }
}
#endif

void CSimObjectV2::DumpMessageProcessingFunctions(std::shared_ptr<CSimObjectV2> where, std::set<std::shared_ptr<MElement>> aDoneList) {
    auto  i = GetImportIncoming();

    if (MainViewPort) {
        where->src << "/* **************************************************************************\n";
        where->src << " *\n";
        where->src << " *  Method-Name   : process()\n";
        where->src << " *\n";
        where->src << " *  Partial generated source code.\n";
        where->src << " *\n";
        where->src << " * *************************************************************************/\n";
        where->src << "void " << name << "::process(std::shared_ptr<tSignalStartCycle> msg) {\n";
        where->src << "// User-Defined-Code: " << helper::tolower(name) << "-startcycle\n";
        where->src << "// End-Of-UDC:" << helper::tolower(name) << "-startcycle\n";
        where->src << "}\n";
        where->src << "/* **************************************************************************\n";
        where->src << " *\n";
        where->src << " *  Method-Name   : process()\n";
        where->src << " *\n";
        where->src << " *  Partial generated source code.\n";
        where->src << " *\n";
        where->src << " * *************************************************************************/\n";
        where->src << "void " << name << "::process(std::shared_ptr<tSignalEndCycle> msg) {\n";
        where->src << "// User-Defined-Code: " << helper::tolower(name) << "-endcycle\n";
        where->src << "// End-Of-UDC:" << helper::tolower(name) << "-endcycle\n";
        where->src << "}\n";

    }
    for (auto & mi : i) {
        auto mic = std::dynamic_pointer_cast<CMessage>(mi);

        if (mic->m_implementation) {
            if (aDoneList.find(mic->m_implementation) == aDoneList.end()) {
                aDoneList.insert(mic->m_implementation);
                auto s = mic->m_implementation->sharedthis<CClassBase>();

                if ((mic->isMessage()) && (mic->mtype != enumMessageType::mReply)) {
                    if (!HaveProcessOperation("process", "std::shared_ptr<" + s->name + ">")) {
                        where->src << "/* **************************************************************************\n";
                        where->src << " *\n";
                        where->src << " *  Method-Name   : process(std::shared_ptr<" << s->name << ">)\n";
                        where->src << " *\n";
                        where->src << " *  Partial generated source code.\n";
                        where->src << " *\n";
                        where->src << " * *************************************************************************/\n";
                        where->src << "tMsg* " << where->name << "::process(std::shared_ptr<" << s->name << "> msg) {\n";
                        where->src << "    tMsg* retval = nullptr;\n";
                        where->src << "// User-Defined-Code: " << where->id << "-process-" << s->name << "\n";
                        where->src << "// End-Of-UDC: " << where->id << "-process-" << s->name << "\n";
                        if (this == where.get()) {
                            DumpEventTransition(where->src, s->name);
                        }
                        where->src << "    return (retval);\n";
                        where->src << "}\n";
                    }
                } else if (mic->isSignal()) {
                    if (!HaveProcessOperation("process", "std::shared_ptr<" + s->name + ">")) {
                        where->src << "/* **************************************************************************\n";
                        where->src << " *\n";
                        where->src << " *  Method-Name   : process(std::shared_ptr<" << s->name << ">)\n";
                        where->src << " *\n";
                        where->src << " *  Partial generated source code.\n";
                        where->src << " *\n";
                        where->src << " * *************************************************************************/\n";
                        where->src << "void " << where->name << "::process(std::shared_ptr<" << s->name << "> sig) {\n";
                        where->src << "// User-Defined-Code: " << where->id << "-process-" << s->name << "\n";
                        where->src << "// End-Of-UDC: " << where->id << "-process-" << s->name << "\n";
                        if (this == where.get()) {
                            DumpEventTransition(where->src, s->name);
                        }
                        where->src << "}\n";
                    }
                } else {
                    //
                    // Dont know what to do with that.
                }
            }
        }
    }
    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpMessageProcessingFunctions(where, aDoneList);
        }
    }
}

void CSimObjectV2::DumpProcessSigSwitch(std::ostream &src, std::set<std::string> &aDoneList) {
    auto     i = GetImportIncoming();
    if (MainViewPort) {
        src << "    case IDS_STARTCYCLE:\n";
        src << "        process(std::static_pointer_cast<tSignalStartCycle>(msg));\n";
        src << "        break;\n";
        src << "    case IDS_ENDCYCLE:\n";
        src << "        process(std::static_pointer_cast<tSignalEndCycle>(msg));\n";
        src << "        break;\n";

    }
    for (auto & mi : i) {
        auto mic = std::dynamic_pointer_cast<CMessage>(mi);

        if (mic->m_implementation) {
            auto s = mic->m_implementation->sharedthis<CClassBase>();

            if (mic->isSignal()) {
                src << "    case IDS_" << helper::toupper(s->name) << ":\n";
                src << "        process(std::static_pointer_cast<" << s->name << ">(msg));\n";
                src << "        break;\n";
            }
        }
    }
    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpProcessSigSwitch(src, aDoneList);
        }
    }
}

void CSimObjectV2::DumpProcessMsgSwitch(std::ostream &src, std::set<std::string> &aDoneList) {
    auto     i = GetImportIncoming();

    for (auto & mi : i) {
        auto mic = std::dynamic_pointer_cast<CMessage>(mi);

        if (mic->m_implementation) {
            auto s = mic->m_implementation->sharedthis<CClassBase>();

            if ((mic->isMessage()) && (mic->mtype != enumMessageType::mReply)) {
                src << "    case IDM_" << helper::toupper(s->name) << ":\n";
                src << "        retmsg = process(std::static_pointer_cast<" << s->name << ">(msg));\n";
                src << "        break;\n";
            }
        }
    }
    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpProcessMsgSwitch(src, aDoneList);
        }
    }
#if 0
    for (auto & mi : i) {
        if (mi->type == eElementType::SimMessage) {
            auto s = std::dynamic_pointer_cast<CSimMessage>(mi);
            auto sc= std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

            if ((sc) && (sc->direction != "Reply")) {
                if (aDoneList.find(sc->upper_basename) == aDoneList.end()) {
                    src << "    case IDM_" << sc->upper_basename << ":\n";
                    src << "        retmsg = process(std::static_pointer_cast<" << sc->name << ">(msg));\n";
                    src << "        break;\n";
                    aDoneList.insert(sc->upper_basename);
                }
            }
        } else if (mi->type == eElementType::JSONMessage) {
            auto s = std::dynamic_pointer_cast<CJSONMessage>(mi);

            if ((s->Class) && (s->Class->type == eElementType::SimMessageClass)) {
                auto sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

                if ((sc != 0) && (sc->direction != "Reply")) {
                    if (aDoneList.find(sc->upper_basename) == aDoneList.end()) {
                        src << "    case IDM_" << sc->upper_basename << ":\n";
                        src << "        retmsg = process(std::static_pointer_cast<" << sc->name << ">(msg));\n";
                        src << "        break;\n";
                        aDoneList.insert(sc->upper_basename);
                    }
                }
            } else {
                std::cerr << "No class defined for : " << s->name << "\n";
            }
        }
    }
    for (auto& baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpProcessMsgSwitch(src, aDoneList);
        }
    }
#endif
}

std::list<std::string>  CSimObjectV2::GetStateVars(std::shared_ptr<MElement> check, std::string prefix) {
    std::list<std::string>         retval;
    std::list<std::string>         more;

    switch (check->type) {
    case eElementType::SimStatemachine:
        for(auto & sti : std::dynamic_pointer_cast<MStatemachine>(check)->states) {
            auto state = std::dynamic_pointer_cast<MState>(*sti);

            if (!state->States.empty()) {
                retval.push_back(helper::normalize(sti->name)+"State");
                more = GetStateVars(*sti, prefix+ sti->name);
                if (!more.empty()) {
                    retval.insert(retval.end(), more.begin(), more.end());
                }
            }
        }
        break;
    case eElementType::State:
        for(auto & sti : std::dynamic_pointer_cast<MState>(check)->States) {
            auto state = std::dynamic_pointer_cast<MState>(*sti);

            if (!state->States.empty()) {
                retval.push_back(helper::normalize(sti->name)+"State");
                more = GetStateVars(*sti, prefix+ sti->name);
                if (!more.empty()) {
                    retval.insert(retval.end(), more.begin(), more.end());
                }
            }
        }
        break;
    default:
        break;
    }
    return retval;
}

bool CSimObjectV2::HaveProcessOperation(std::string opname, std::string msg) {
    bool retval = false;
    auto  op = GetImportOperation();

    for (auto & oi : op) {
        if (oi->name == opname) {
            auto operation = std::dynamic_pointer_cast<MOperation>(oi);
            std::string pname;

            for (auto & pi : operation->Parameter) {
                auto para = std::dynamic_pointer_cast<MParameter>(*pi);

                if (para->Direction != "return") {
                    if (para->Classifier) {
                        pname = para->Classifier->FQN();
                    } else  {
                        pname = para->ClassifierName;
                    }
                    if (pname == msg) {
                        retval = true;
                    }
                }
            }
        }
    }
    return retval;
}

std::string CSimObjectV2::GetInitOperation() {
    std::string retval;

    for (auto & i : Operation) {
        if (i->HasTaggedValue("InitOperation")) {
            retval = i->name;
            break;
        }
    }
    return retval;
}
