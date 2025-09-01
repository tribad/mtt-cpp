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
#include <sys/types.h>
#include <sys/stat.h>

#if defined (__linux__)
#include <unistd.h>
#endif

#include <errno.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <set>

#include "json.h"
#include "parser.h"
#include "massociation.h"
#include "massociationend.h"
#include "mdependency.h"
#include "mgeneralization.h"
#include "mattribute.h"
#include "mparameter.h"
#include "moperation.h"
#include "mlifeline.h"
#include "mwebpagelifeline.h"
#include "mjslifeline.h"
#include "musecase.h"
#include "mclass.h"
#include "mmessage.h"
#include "mjsonmessage.h"
#include "msimmessage.h"
#include "msignal.h"
#include "msimsignal.h"
#include "mpackage.h"
#include "msimstatemachine.h"
#include "mstate.h"
#include "maction.h"
#include "mpseudostate.h"
#include "mfinalstate.h"
#include "mtransition.h"
#include "mevent.h"
#include "mpin.h"
#include "medge.h"
#include "mnode.h"
#include "minitialnode.h"
#include "mfinalnode.h"
#include "mactionnode.h"
#include "mdecisionnode.h"
#include "mmergenode.h"
#include "mjoinnode.h"
#include "mforknode.h"
#include "mactivity.h"
#include "mcollaboration.h"
#include "mobject.h"
#include "mmodel.h"
#include "minteraction.h"

#include "mconnector.h"

#include "stereotypes.h"
#include "helper.h"

void filltransition(std::shared_ptr<MTransition> t, tJSONObject* j) ;
void fillpseudostate(std::shared_ptr<MPseudoState> ps, tJSONObject* j) ;
void fillfinalstate(std::shared_ptr<MFinalState> fs, tJSONObject* j) ;
void fillaction(std::shared_ptr<MAction> act, tJSONObject* j) ;
void fillparameter(std::shared_ptr<MParameter> p, tJSONObject *j) ;
void fillclass(std::shared_ptr<MClass> c, tJSONObject *j) ;
/*
 * This is the JSON-Document of the model.
 */
static tJSON*  root = nullptr;
/*
 * This is the model we construct.
 */
static std::shared_ptr<MModel> model;

static std::list<std::shared_ptr<MClass>> gTypesToComplete;

tJSON* findbyname(tJSONObject* o, std::string name) {
    tJSON*                                  retval=0;
    std::map<std::string, tJSON*>::iterator i;

    if (o != nullptr) {
        i=o->values.find(name);
        if (i != o->values.end()) {
            retval=i->second;
        }
    }
    return (retval);
}

std::list<tJSON*> findbytype(tJSON* node, std::string type)
{
    std::list<tJSON*>                       nodelist;
    //
    //  Check if object has _type attribute
    if (node->type == eJSON::eObject) {
        tJSONObject*                            obj;
        std::map<std::string, tJSON*>::iterator i;

        obj=(tJSONObject*)node;
        i = obj->values.find("_type");
        //
        // Check if found
        if (i != obj->values.end()) {
            std::string nodetype=((tJSONValue*)(i->second))->value;
            if (nodetype == type) {
                nodelist.push_back(node);
            }
        }
        for (i=obj->values.begin(); i != obj->values.end(); ++i) {
            if ((i->second->type == eJSON::eObject) || (i->second->type == eJSON::eArray)) {
                std::list<tJSON*> more=findbytype(i->second, type);
                nodelist.insert(nodelist.end(), more.begin(), more.end());
            }
        }

    } else if (node->type == eJSON::eArray) {
        tJSONArray*                   array;
        std::vector<tJSON*>::iterator i;

        array=(tJSONArray*)node;
        for (i=array->values.begin(); i != array->values.end(); ++i) {
            if (((*i)->type == eJSON::eObject) || ((*i)->type == eJSON::eArray)) {
                std::list<tJSON*> more=findbytype(*i, type);
                nodelist.insert(nodelist.end(), more.begin(), more.end());
            }
        }
    }
    return nodelist;
}

std::string getstringattr(tJSONObject* node, std::string name) {
    std::map<std::string, tJSON*>::iterator i;

    if (node->base.type == eJSON::eObject) {
        i=node->values.find(name);
        if (i!=node->values.end()) {
            std::string s = (*(tJSONValue*)(i->second));
            return s;
        }
    }
    return "";
}

bool getboolean(tJSONObject* node, std::string name, bool defaultvalue = false) ;

bool getboolean(tJSONObject* node, std::string name, bool defaultvalue) {
    std::map<std::string, tJSON*>::iterator i;

    i=node->values.find(name);
    if (i != node->values.end()) {
        tJSONValue* b = ((tJSONValue*)(i->second));
        return (b->value);
    } else {
        return defaultvalue;
    }

    return (false);
}

std::string getstereotype(tJSONObject* node) {
    std::string ref;
    std::map<std::string, tJSON*>::iterator i;

    i=node->values.find("stereotype");
    if (i!=node->values.end()) {
        ref=getstringattr((tJSONObject*)(i->second), "$ref");
    }
    return (ref);
}

eVisibility getvisibility(tJSONObject* j) {
    std::string vis=getstringattr(j, "visibility");

    if (vis == "private") {
        return vPrivate;
    } else if (vis == "public") {
        return vPublic;
    } else if (vis == "package") {
        return vPackage;
    } else if (vis == "protected") {
        return vProtected;
    }
    return vPublic;
}


eNavigable getnavigable(tJSONObject* j) {
    std::string nav = helper::tolower(getstringattr(j, "navigable"));

    if (nav.empty() || (nav == "unspecified")) {
        return eNavigable::unspecified;
    } else if (nav == "navigable") {
        return eNavigable::yes;
    } else if (nav == "notnavigable") {
        return eNavigable::no;
    }
    return eNavigable::unspecified;
}


eAggregation getaggregation(tJSONObject* j) {
    std::string agg=getstringattr(j, "aggregation");

    if (agg == "shared") {
        return aShared;
    } else if (agg == "none") {
        return aNone;
    } else if (agg == "composite") {
        return aComposition;
    } else if (agg == "Aggregation") {
        return aAggregation;
    } else if (agg == "Composition") {
        return aComposition;
    }
    return aNone;
}

mMessageType getmsgtype(tJSONObject* j) {
    std::string mtype=getstringattr(j, "messageSort");

    if (mtype == "reply") {
        return mReply;
    } else if (mtype == "asynchSignal") {
        return mAsyncSignal;
    } else if (mtype == "asynchCall") {
        return mAsyncCall;
    } else if (mtype == "synchCall") {
        return mSyncCall;
    } else if (mtype == "createMessage") {
        return mCreateMessage;
    } else if (mtype == "deleteMessage") {
        return mDeleteMessage;
    }
    return mSyncCall;
}

eActionKind getactionkind(tJSONObject* j) {
    std::string kind=getstringattr(j, "kind");

    if (kind == "opaque") {
        return eActionKind::Opaque;
    } else if (kind == "create") {
        return eActionKind::Create;
    } else if (kind == "destroy") {
        return eActionKind::Destroy;
    } else if (kind == "read") {
        return eActionKind::Read;
    } else if (kind == "write") {
        return eActionKind::Write;
    } else if (kind == "insert") {
        return eActionKind::Insert;
    } else if (kind == "delete") {
        return eActionKind::Delete;
    } else if (kind == "sendSignal") {
        return eActionKind::SendSignal;
    } else if (kind == "acceptSignal") {
        return eActionKind::AcceptSignal;
    } else if (kind == "triggerEvent") {
        return eActionKind::TriggerEvent;
    } else if (kind == "acceptEvent") {
        return eActionKind::AcceptEvent;
    } else if (kind == "structured") {
        return eActionKind::Structured;
    } else if (kind == "timerEvent") {
        return eActionKind::TimerEvent;
    } else {
        return eActionKind::Opaque;
    }
}

std::string getreference(tJSONObject* j, const std::string& name ) {
    std::string ref;
    std::map<std::string, tJSON*>::iterator i;

    i = j->values.find(name);
    if ((i!=j->values.end()) && (i->second->type==eJSON::eObject)) {
        ref=getstringattr((tJSONObject*)(i->second), "$ref");
    }
    return (ref);
}

static std::string findnamespace(std::shared_ptr<MPackage> p) {
    std::string result;

    if(p->parent && p->parent->IsPackageBased()) {
        result = findnamespace(std::dynamic_pointer_cast<MPackage>(*p->parent));

        std::string ns = p->GetTaggedValue("namespace");
        if (!ns.empty()) {
            result += ns + "::";
        }
    }

    return result;
}

static std::string  findnamespace(std::shared_ptr<MClass> c) {
    std::string result;

    if (c->parent && c->parent->IsClassBased()) {
        result = findnamespace(std::dynamic_pointer_cast<MClass>(*c->parent));  // enclosed classes not supported yet.
    } else if(c->parent && c->parent->IsPackageBased()) {
        result = findnamespace(std::dynamic_pointer_cast<MPackage>(*c->parent));
    }
    return result;
}

void filltags(std::shared_ptr<MElement> e, tJSONObject* j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   tags=(tJSONArray*)(findbyname(j, "tags"));

    if (tags != nullptr) {
        for (i=tags->values.begin(); i!= tags->values.end(); ++i) {
            std::string name=getstringattr((tJSONObject*)(*i), "name");
            std::string value=getstringattr((tJSONObject*)(*i), "value");
            e->AddTag(name, value);
        }
    }
}

void filltemplateparameters(std::shared_ptr<MElement> e, tJSONObject* j) {
    tJSONArray*                   parameters=(tJSONArray*)(findbyname(j, "templateParameters"));

    if (parameters != nullptr) {
        auto c = std::dynamic_pointer_cast<MClass>(e);
        int  pos = 0;
        for (auto & i : parameters->values) {
            std::string id       = getstringattr((tJSONObject*)(i), "_id");
            std::string name     = getstringattr((tJSONObject*)(i), "name");
            std::string parent   = getreference((tJSONObject*)(i), "_parent");
            std::string ptype    = getstringattr((tJSONObject*)(i), "parameterType");
            std::string defvalue = getstringattr((tJSONObject*)(i), "defaultValue");

            auto p = MParameter::construct(id, e);

            p->name           = name;
            p->ClassifierName = ptype;
            p->defaultValue   = defvalue;
            p->mPosition      = pos;

            c->mClassParameter.emplace(pos++, p);
        }
    }
}


void fillelement(std::shared_ptr<MElement> e, tJSONObject* j) {
    filltags(e, j);
    e->name                 = getstringattr(j, "name");
    e->visibility           = getvisibility(j);
    e->comment              = getstringattr(j, "documentation");

    auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(j)));
    if (stereotype) {
        e->stereotypes.insert(std::pair<std::string, std::shared_ptr<MStereotype>>(stereotype->name, stereotype));
    }
}


void filllifeline(std::shared_ptr<MLifeLine> l, tJSONObject* j) {
    //
    //  Fill with tagged values.
    fillelement(l, j);
    //
    //  Set the Source Reference if we find one.
    std::string ref=getreference(j, "represent");

    if (!ref.empty()) {
        l->role = ref ;
    } else {
    }
}

void fillmsg(std::shared_ptr<MMessage> m, tJSONObject *j)
{
    //
    //  Fill with tagged values.
    fillelement(m, j);

    m->mtype             = getmsgtype(j);
    m->isConcurrent      = getboolean(j, "isConcurrentIteration");
    m->m_guard           = getstringattr(j, "guard");
    //
    //  Set the Source Reference if we find one.
    std::string ref=getreference(j, "source");

    if (!ref.empty()) {
        m->source = ref ;
    } else {
    }
    //
    //  Set the Target Reference if we find one.
    ref=getreference(j, "target");

    if (!ref.empty()) {
        m->target = ref ;
    } else {
    }
    //
    // The signature if we are connected to a method.
    ref=getreference(j, "signature");

    if (!ref.empty()) {
        m->signature = ref ;
    } else {
    }
    //
    // The connector in between lifelines.
    ref=getreference(j, "connector");

    if (!ref.empty()) {
        m->connector = ref ;
    } else {
    }

}


void fillassocend(std::shared_ptr<MAssociationEnd> e, tJSONObject *j)
{
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   owned=(tJSONArray*)(findbyname(j, "ownedElements"));
    //
    //  Fill with tagged values.
    fillelement(e, j);
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (owned != nullptr) {

    }
    e->Aggregation       = getaggregation(j);
    e->Multiplicity      = getstringattr(j, "multiplicity");
    e->Navigable         = getnavigable(j);
    e->defaultValue      = getstringattr(j, "defaultValue");
    //
    //  Set the owning attribute reference.
    std::string owner = getreference(j, "ownerAttribute");

    if (!owner.empty()) {
        e->mOwner = owner;
    }
    //
    //  Set the Classifier Reference if we find one.
    std::string ref=getreference(j, "reference");

    if (!ref.empty()) {
        e->Classifier = ref ;
    } else {

    }
}

void filledge(std::shared_ptr<MEdge> p, tJSONObject* j) {
    fillelement(p, j);

    p->Source = getreference(j, "source");
    p->Target = getreference(j, "target");
    p->Guard     = getstringattr(j, "guard");
}

void fillnode(std::shared_ptr<MNode> p, tJSONObject* j) {
    fillelement(p, j);
    p->Target = getreference(j, "target");
}

void fillactionnode(std::shared_ptr<MActionNode> p, tJSONObject* j) {
    tJSONArray*                   ipins      = (tJSONArray*)(findbyname(j, "inputs"));
    tJSONArray*                   opins      = (tJSONArray*)(findbyname(j, "outputs"));
    std::vector<tJSON*>::iterator i;

    p->Sub                = getreference(j, "subactivity");
    p->isLocallyReentrant = getboolean(j, "isLocallyReentrant");
    p->isSynchronous      = getboolean(j, "isSynchronous");
    p->Language           = getstringattr(j, "language");
    p->Body               = getstringattr(j, "body");
    p->Kind               = getactionkind(j);

    if (ipins != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=ipins->values.begin(); i!= ipins->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");
            auto newpin = MPin::construct(id, p);

            fillparameter(newpin, (tJSONObject*)(*i));
            newpin->Direction = "in";
            p->InputPins.emplace_back(newpin);
        }
    }
    if (opins != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=opins->values.begin(); i!= opins->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");
            auto newpin = MPin::construct(id, p);

            fillparameter(newpin, (tJSONObject*)(*i));
            newpin->Direction = "out";
            p->OutputPins.emplace_back(newpin);
        }
    }
}

void fillactivity(std::shared_ptr<MActivity> p, tJSONObject* j) {
//    tJSONArray*                   owned      = (tJSONArray*)(findbyname(j, "ownedElements"));
    tJSONArray*                   nodes      = (tJSONArray*)(findbyname(j, "nodes"));
    tJSONArray*                   edges      = (tJSONArray*)(findbyname(j, "edges"));
    std::vector<tJSON*>::iterator i;

    fillnode(p, j);

    p->isReadOnly        = getboolean(j, "isReadOnly");
    p->isReentrant       = getboolean(j, "isReentrant");
    p->isSingleExecution = getboolean(j, "isSingleExecution");

    //
    //  The id should be set already. Now check whether we have owned elements.
    if (nodes != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=nodes->values.begin(); i!= nodes->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");
            std::shared_ptr<MNode> newnode;

            if (o_type == "UMLInitialNode") {
                auto node = MInitialNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLFinalNode") {
                auto node = MFinalNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLAction") {
                auto node = MActionNode::construct(id, p);

                fillactionnode(node, (tJSONObject*)(*i));
                newnode = node;
            } else if (o_type == "UMLDecisionNode") {
                auto node = MDecisionNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLMergeNode") {
                auto node = MMergeNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLJoinNode") {
                auto node = MJoinNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLForkNode") {
                auto node = MForkNode::construct(id, p);

                newnode = node;
            } else {
                auto node = MNode::construct(id, p);

                newnode = node;
            }
            if (newnode) {
                fillnode(newnode, (tJSONObject*)(*i));
                p->Nodes.emplace_back(newnode);
            }
        }
    }
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (edges != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=edges->values.begin(); i!= edges->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            auto edge = MEdge::construct(id, p);

            filledge(edge, (tJSONObject*)(*i));

            p->Edges.emplace_back(edge);
        }
    }
}

void fillparameter(std::shared_ptr<MParameter> p, tJSONObject *j) {
    //
    //  Fill with tagged values.
    fillelement(p, j);

    p->defaultValue   = getstringattr(j, "defaultValue");
    p->isLeaf         = getboolean(j, "isLeaf");
    p->Multiplicity   = getstringattr(j, "multiplicity");
    p->Direction      = getstringattr(j, "direction");
    p->isReadOnly     = getboolean(j, "isReadOnly");
    //
    //  get the classifier reference if one is there.
    //  Taking the type name if not.
    std::string ref = getreference(j, "type");

    if (ref.empty()) {
        p->ClassifierName = getstringattr(j, "type");
    } else {
        p->Classifier = ref;
    }
}

void fillcollaboration(std::shared_ptr<MCollaboration> c, tJSONObject* j) {
    (void)c;
    (void)j;
}

void filloperation(std::shared_ptr<MOperation> o, tJSONObject *j) {
    tJSONArray*                   owned=(tJSONArray*)(findbyname(j, "ownedElements"));
    tJSONArray* parameters=(tJSONArray*)(findbyname(j, "parameters"));
    std::vector<tJSON*>::iterator i;
    //
    //  Fill with tagged values.
    fillelement(o, j);

    o->isStatic       = getboolean(j, "isStatic");
    o->isAbstract     = getboolean(j, "isAbstract");
    o->isQuery        = getboolean(j, "isQuery");
    o->Specification  = getstringattr(j, "specification");

    if (parameters != nullptr) {
        //
        //  *i points to an tJSONObject. For all parameters we create the
        //  model elements.
        for (i=parameters->values.begin(); i!= parameters->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLParameter") {
                auto p = MParameter::construct(id, o);

                fillparameter(p, (tJSONObject*)(*i));
                o->Parameter.emplace_back(p);
            }
        }
    }
    if (owned != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i = owned->values.begin(); i != owned->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLActivity") {
               auto newact = MActivity::construct(id, o);

               fillactivity(newact,  (tJSONObject*)(*i));
               o->Activity = newact;
            } else if (o_type == "UMLCollaboration") {
                //std::string id=getstringattr((tJSONObject*)(*i), "_id");

                auto newclass = MClass::construct(id, "Collaboration",  o);
                fillclass(newclass, (tJSONObject*)(*i));
                o->mCollaboration.emplace_back(newclass);
            }

        }
    }
}


void fillassoc(std::shared_ptr<MAssociation> a, tJSONObject *j)
{
    tJSONObject*                  ends;
    //
    //  Fill with tagged values.
    filltags(a, j);
    a->name = getstringattr(j, "name");
    a->visibility = getvisibility(j);
    //
    //  The id should be set already. Readin the ends
    ends=(tJSONObject*)findbyname(j, "end1");
    if (ends != nullptr) {
        std::string     id      = getstringattr(ends, "_id");
        auto newend = MAssociationEnd::construct(id, a);

        fillassocend(newend, ends);
        a->AddEnd(newend);
        model->Add(newend);
    }
    ends=(tJSONObject*)findbyname(j, "end2");
    if (ends != nullptr) {
        std::string     id      = getstringattr(ends, "_id");
        auto newend = MAssociationEnd::construct(id, a);

        fillassocend(newend, ends);
        a->AddEnd(newend);
        model->Add(newend);
    }
    a->comment = getstringattr(j, "documentation");
}

void fillconnector(std::shared_ptr<MConnector> a, tJSONObject *j) {
    tJSONObject*                  ends;
    //
    //  Fill with tagged values.
    filltags(a, j);
    a->name = getstringattr(j, "name");
    a->visibility = getvisibility(j);
    //
    //  The id should be set already. Readin the ends
    ends=(tJSONObject*)findbyname(j, "end1");
    if (ends != nullptr) {
        std::string     id      = getstringattr(ends, "_id");
        auto newend = MAssociationEnd::construct(id, a);

        fillassocend(newend, ends);
        a->AddEnd(newend);
        model->Add(newend);
    }
    ends=(tJSONObject*)findbyname(j, "end2");
    if (ends != nullptr) {
        std::string     id      = getstringattr(ends, "_id");
        auto newend = MAssociationEnd::construct(id, a);

        fillassocend(newend, ends);
        a->AddEnd(newend);
        model->Add(newend);
    }
    a->comment = getstringattr(j, "documentation");
}

void fillattribute(std::shared_ptr<MAttribute> a, tJSONObject *j) {
    //
    //  Fill with tagged values.
    filltags(a, j);
    a->name           = getstringattr(j, "name");
    a->visibility     = getvisibility(j);
    a->comment        = getstringattr(j, "documentation");
    a->Aggregation    = getaggregation(j);
    a->Multiplicity   = getstringattr(j, "multiplicity");
    a->defaultValue   = getstringattr(j, "defaultValue");
    a->isStatic       = getboolean(j, "isStatic");
    a->isReadOnly     = getboolean(j, "isReadOnly");
    a->isDerived      = getboolean(j, "isDerived");
    //
    //  get the classifier reference if one is there.
    //  Taking the type name if not.
    std::string ref = getreference(j, "type");

    if (ref.empty()) {
        a->ClassifierName = getstringattr(j, "type");
    } else {
        a->Classifier = ref;
    }
    //
    //  Parse all owned elements.
    tJSONArray*                   owned      = (tJSONArray*)(findbyname(j, "ownedElements"));

    if (owned != nullptr) {
        for (auto & i : owned->values) {
            std::string o_type = getstringattr((tJSONObject*)(i), "_type");
            std::string id     = getstringattr((tJSONObject*)(i), "_id");

            if (o_type=="UMLConnector") {
                auto newconnector = MConnector::construct(id, a);

                fillconnector(newconnector, (tJSONObject*)(i));
                a->connector.emplace_back(newconnector);
            }

        }
    }
}

void fillslot(std::shared_ptr<MAttribute> a, tJSONObject *j) {
    //
    //  Fill with tagged values.
    filltags(a, j);
    a->name           = getstringattr(j, "name");
    a->visibility     = getvisibility(j);
    a->comment        = getstringattr(j, "documentation");
    a->Aggregation    = getaggregation(j);
    a->Multiplicity   = getstringattr(j, "multiplicity");
    a->defaultValue   = getstringattr(j, "value");
    a->isStatic       = getboolean(j, "isStatic");
    //
    //  get the classifier reference if one is there.
    //  Taking the type name if not.
    std::string ref = getreference(j, "type");

    if (ref.empty()) {
        a->ClassifierName = getstringattr(j, "type");
    } else {
        a->Classifier = ref;
    }
}

void fillnormalstate(std::shared_ptr<MState> ns, tJSONObject* j) {
    tJSONArray* regions = (tJSONArray*)(findbyname(j, "regions"));
    tJSONArray* doact   = (tJSONArray*)(findbyname(j, "doActivities"));

    std::vector<tJSON*>::iterator i;

    ns->name           = getstringattr(j, "name");
    ns->visibility     = getvisibility(j);
    ns->comment        = getstringattr(j, "documentation");

    if (doact != nullptr) {
        for (i=doact->values.begin(); i!= doact->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLOpaqueBehavior") {
                auto act = MAction::construct(id, ns);

                fillaction(act, (tJSONObject*)(*i));
                ns->DoActions.emplace_back(act);
            }
        }
    }
    //
    //  Process composite states
    if (regions != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=regions->values.begin(); i!= regions->values.end(); ++i) {
            std::vector<tJSON*>::iterator x;
            tJSONArray*                   vertices=(tJSONArray*)(findbyname((tJSONObject*)(*i), "vertices"));
            tJSONArray*                   transitions=(tJSONArray*)(findbyname((tJSONObject*)(*i), "transitions"));

            if (vertices != nullptr) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=vertices->values.begin(); x!= vertices->values.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    if (o_type == "UMLPseudostate") {
                        auto ps = MPseudoState::construct(id, ns);

                        fillpseudostate(ps, (tJSONObject*)(*x));
                        ns->States.emplace_back(ps);
                    } else if (o_type == "UMLFinalState") {
                        auto fs = MFinalState::construct(id, ns);

                        fillfinalstate(fs, (tJSONObject*)(*x));
                        ns->States.emplace_back(fs);
                    } else if (o_type == "UMLState") {
                        auto nos = MState::construct(id, ns);

                        fillnormalstate(nos, (tJSONObject*)(*x));
                        ns->States.emplace_back(nos);
                    }
                }
            }
            if (transitions != nullptr) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=transitions->values.begin(); x!= transitions->values.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    auto newtransition = MTransition::construct(id, ns);

                    if (newtransition) {
                        filltransition(newtransition, (tJSONObject*)(*x));
                        ns->Transitions.emplace_back(newtransition);
                    }
                }
            }
        }
    }
}

void fillfinalstate(std::shared_ptr<MFinalState> fs, tJSONObject* j) {
    fillnormalstate(fs, j);
}

void fillpseudostate(std::shared_ptr<MPseudoState> ps, tJSONObject* j) {
    fillnormalstate(ps, j);
    ps->kind        = getstringattr(j, "kind");
}

void fillevent(std::shared_ptr<MEvent> ev, tJSONObject* j) {
    ev->name       = getstringattr(j, "name");
    ev->visibility = getvisibility(j);
    ev->comment    = getstringattr(j, "documentation");
    ev->kind       = getstringattr(j, "kind");
}

void fillaction(std::shared_ptr<MAction> act, tJSONObject* j) {
    act->name       = getstringattr(j, "name");
    act->visibility = getvisibility(j);
    act->comment    = getstringattr(j, "documentation");
}

void filltransition(std::shared_ptr<MTransition> t, tJSONObject* j) {
    tJSONArray*                   trigger=(tJSONArray*)(findbyname(j, "triggers"));
    tJSONArray*                   effects=(tJSONArray*)(findbyname(j, "effects"));
    std::vector<tJSON*>::iterator i;

    t->name       = getstringattr(j, "name");
    t->visibility = getvisibility(j);
    t->comment    = getstringattr(j, "documentation");
    t->kind       = getstringattr(j, "kind");
    t->guard      = getstringattr(j, "guard");
    t->from       = getreference(j, "source");
    t->to         = getreference(j, "target");

    if (trigger != nullptr) {
        //
        //  *i points to an tJSONObject. For all triggers we create the
        //  model elements.
        for (i=trigger->values.begin(); i!= trigger->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLEvent") {
                auto ev = MEvent::construct(id, t);

                fillevent(ev, (tJSONObject*)(*i));
                t->events.emplace_back(ev);
            }
        }
    }
    if (effects != nullptr) {
        //
        //  *i points to an tJSONObject. For all triggers we create the
        //  model elements.
        for (i=effects->values.begin(); i!= effects->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLOpaqueBehavior") {
                auto act = MAction::construct(id, t);

                fillaction(act, (tJSONObject*)(*i));
                t->actions.emplace_back(act);
            }
        }
    }
}

void fillstatemachine(std::shared_ptr<MStatemachine> s, tJSONObject* j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   regions=(tJSONArray*)(findbyname(j, "regions"));
    //
    //  Fill with tagged values.
    filltags(s, j);
    if (regions != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=regions->values.begin(); i!= regions->values.end(); ++i) {
            std::vector<tJSON*>::iterator x;
            tJSONArray*                   vertices=(tJSONArray*)(findbyname((tJSONObject*)(*i), "vertices"));
            tJSONArray*                   transitions=(tJSONArray*)(findbyname((tJSONObject*)(*i), "transitions"));

            if (vertices != nullptr) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=vertices->values.begin(); x!= vertices->values.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    if (o_type == "UMLPseudostate") {
                        auto ps = MPseudoState::construct(id, s);

                        fillpseudostate(ps, (tJSONObject*)(*x));
                        s->states.emplace_back(ps);
                    } else if (o_type == "UMLFinalState") {
                        auto fs = MFinalState::construct(id, s);

                        fillfinalstate(fs, (tJSONObject*)(*x));
                        s->states.emplace_back(fs);
                    } else if (o_type == "UMLState") {
                        auto ns = MState::construct(id, s);

                        fillnormalstate(ns, (tJSONObject*)(*x));
                        s->states.emplace_back(ns);
                    }
                }
            }
            if (transitions != nullptr) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=transitions->values.begin(); x!= transitions->values.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    auto newtransition=MTransition::construct(id, s);

                    if (newtransition) {
                        filltransition(newtransition, (tJSONObject*)(*x));
                        s->transitions.emplace_back(newtransition);
                    }
                }
            }
        }
    }
}

void fillusecase(std::shared_ptr<MUseCase> u, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   attributes=(tJSONArray*)(findbyname(j, "attributes"));
    //
    //  Fill with tagged values.
    filltags(u, j);
    //
    //  The id should be set already. Now check whether we have attributes defined.
    if (attributes != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=attributes->values.begin(); i!= attributes->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLAttribute") {
                auto newattr = MAttribute::construct(id, u);

                fillattribute(newattr, (tJSONObject*)(*i));
                u->Attribute.emplace_back(newattr);
            }
        }
    }
    u->visibility = getvisibility(j);
    u->comment    = getstringattr(j, "documentation");
}

void filldependency(std::shared_ptr<MDependency> d, tJSONObject*j) {
    //
    //  Fill with tagged values.
    filltags(d, j);

    d->name              = getstringattr(j, "name");
    d->visibility        = getvisibility(j);
    d->comment           = getstringattr(j, "documentation");

    d->src      = getreference(j, "source");
    d->target   = getreference(j, "target");
}


void fillgeneralization(std::shared_ptr<MGeneralization> d, tJSONObject*j) {
    //
    //  Fill with tagged values.
    filltags(d, j);

    d->name        = getstringattr(j, "name");
    d->visibility  = getvisibility(j);
    d->comment     = getstringattr(j, "documentation");

    d->derived = getreference(j, "source");
    d->base    = getreference(j, "target");


    tJSONArray*                   parameters=(tJSONArray*)(findbyname(j, "parameterSubstitutions"));

    if (parameters != nullptr) {
        auto g = std::dynamic_pointer_cast<MGeneralization>(d);
        int  pos = 0;
        for (auto & i : parameters->values) {
            std::string id     = getstringattr((tJSONObject*)(i), "_id");
            std::string name   = getstringattr((tJSONObject*)(i), "name");
            std::string parent = getreference((tJSONObject*)(i), "_parent");
            std::string actual = getreference((tJSONObject*)(i), "actual");
            std::string formal = getreference((tJSONObject*)(i), "formal");

            auto p = MParameter::construct(id, d);

            p->name    = name;
            p->mActual = actual;
            p->mFormal = formal;

            g->mTemplateParameter.emplace(pos++, p);
        }
    }

}

void fillobject(std::shared_ptr<MObject> o, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   owned      = (tJSONArray*)(findbyname(j, "ownedElements"));
    tJSONArray*                   sl         = (tJSONArray*)(findbyname(j, "slots"));
//    tJSONArray*                   operations = (tJSONArray*)(findbyname(j, "operations"));
//    tJSONArray*                   literals   = (tJSONArray*)(findbyname(j, "literals"));

    std::string                   name = getstringattr(j, "name");
    //
    //  Use of this method is mandatory.
    o->name=(name);
    //
    //  Fill with tagged values.
    filltags(o, j);
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (owned != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->values.begin(); i!= owned->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type=="UMLLink") {
                auto newlink = MLink::construct(id, o);

                fillassoc(newlink, (tJSONObject*)(*i));
                o->Add(newlink);
            }
        }
    }
    if (sl != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=sl->values.begin(); i!= sl->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLSlot") {
                auto newattr = MAttribute::construct(id, o);

                fillslot(newattr, (tJSONObject*)(*i));
                o->Add(newattr);
            }
        }
    }
}

void fillinteraction(std::shared_ptr<MInteraction> in, tJSONObject *j) {
    std::string                   name = getstringattr(j, "name");
    //
    //  Use of this method is mandatory.
    in->name = name;
    //
    //  Fill with tagged values.
    filltags(in, j);
    //
    tJSONArray*                   messages     = (tJSONArray*)(findbyname(j, "messages"));

    if (messages != nullptr) {
        for ( auto & m : messages->values) {
            std::string o_type = getstringattr((tJSONObject*)(m), "_type");
            std::string id     = getstringattr((tJSONObject*)(m), "_id");

            if (o_type=="UMLMessage") {

                auto newmsg = MElementRef(id);

                in->messages.emplace_back(newmsg);
            }

        }
    }

    tJSONArray*                   lifelines    = (tJSONArray*)(findbyname(j, "participants"));

    if (lifelines != nullptr) {
        for (auto & l : lifelines->values) {
            std::string o_type = getstringattr((tJSONObject*)(l), "_type");
            std::string id     = getstringattr((tJSONObject*)(l), "_id");

            if (o_type=="UMLLifeline") {
                auto newll = MElementRef(id);

                in->lifelines.emplace_back(newll);
            }

        }
    }
}

void fillclass(std::shared_ptr<MClass> c, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   owned      = (tJSONArray*)(findbyname(j, "ownedElements"));
    tJSONArray*                   attributes = (tJSONArray*)(findbyname(j, "attributes"));
    tJSONArray*                   operations = (tJSONArray*)(findbyname(j, "operations"));
    tJSONArray*                   literals   = (tJSONArray*)(findbyname(j, "literals"));
    std::string                   name       = getstringattr(j, "name");

    //
    //
    c->name = helper::trim(name);
    //
    // find the namespace from the packages.
    c->mNameSpace = findnamespace(c);

    TypeNode temp = TypeNode::parse(c->name);

    if (temp.isCompositeType()) {
        gTypesToComplete.push_back(c);
    } else {
        gTypesToComplete.push_front(c);
    }

    //
    //  Fill with tagged values.
    filltags(c, j);
    //
    // Now load the template parameters.
    filltemplateparameters(c, j);
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (owned != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i = owned->values.begin(); i != owned->values.end(); ++i) {
            std::string o_type = getstringattr((tJSONObject*)(*i), "_type");
            std::string id     = getstringattr((tJSONObject*)(*i), "_id");

            if (o_type=="UMLAssociation") {
                auto newassoc = MAssociation::construct(id, c);

                fillassoc(newassoc, (tJSONObject*)(*i));
                c->Add(newassoc);
                model->Add(newassoc);
            } else if (o_type == "UMLAttribute") {
                auto newattr = MAttribute::construct(id, c);

                fillattribute(newattr, (tJSONObject*)(*i));
                c->Add(newattr);
            } else if (o_type == "UMLDependency") {
                std::string stname = getstereotype((tJSONObject*)(*i));
                auto stereotype = model->StereotypeById(stname);
                auto newdep = MDependency::construct(id, stereotype, c);

                filldependency(newdep,  (tJSONObject*)(*i));
                //c->Dependency.push_back(newdep);
                model->Add(newdep);

            } else if ((o_type == "UMLGeneralization") || (o_type == "UMLInterfaceRealization") || (o_type == "UMLTemplateBinding")) {
                auto newgen = MGeneralization::construct(id, 0, c);

                if (o_type == "UMLInterfaceRealization") {
                    newgen->mIsRealization = true;
                }
                fillgeneralization(newgen,  (tJSONObject*)(*i));
                //
                //  We do not add the found generalization to the list of generalization in the class.
                //  We may find generalizations that have not assoziation with the class that owns them.
                //
                // c->Generalization.push_back(newgen);
                //
                //  The generalization list will be filled in preparation of the model class from the global instances map.

            } else  if (o_type == "UMLActivity") {
                auto newact = MActivity::construct(id, c);

                fillactivity(newact,  (tJSONObject*)(*i));
                c->Activity.emplace_back(newact);

            } else if (o_type == "UMLStateMachine") {
                switch (c->type) {
                case eElementType::SimObject:
                {
                    auto newstatemachine = MSimStatemachine::construct(id, c);

                    if (newstatemachine) {
                        fillstatemachine(newstatemachine, (tJSONObject*)(*i));
                    }
                    c->statemachine=newstatemachine;
                }
                    break;
                default:
                    break;
                }
            } else if (o_type == "UMLCollaboration") {
                auto newclass = MClass::construct(id, "Collaboration",  c);
                fillclass(newclass, (tJSONObject*)(*i));
                c->mCollaboration.emplace_back(newclass);
            } else if (o_type == "UMLInteraction") {
                //
                //  Here we should find the lifelines and messages.
                auto newinteraction = MInteraction::construct(id, c);
                fillinteraction(newinteraction,  (tJSONObject*)(*i));

                c->mInteraction.emplace_back(newinteraction);

            } else if (o_type == "UMLClass") {
                std::shared_ptr<MClass> newclass;
                auto stereotype=model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                std::string stype;

                if (stereotype) {
                    newclass = MClass::construct(id, stereotype, c);
                } else {
                    stype = c->GetDefaultStereotype();
                    if (!stype.empty()) {
                        newclass = MClass::construct(id, stype, c);
                    } else {
                        if (c->getPackageType() == eElementType::ExternPackage) {
                            newclass = MClass::construct(id, "Extern", c);
                        } else {
                            newclass = MClass::construct(id, "", c);
                        }
                    }
                }
                if (newclass) {
                    if (o_type == "UMLInterface") {
                        newclass->mIsInterface = true;
                    }
                    fillclass(newclass, (tJSONObject*)(*i));
                    c->owned.emplace_back(newclass);
                }
            } else if  (o_type == "UMLInterface") {
                std::shared_ptr<MClass> newclass;
                std::string stype;

                newclass = MClass::construct(id, "Interface", c);
                if (newclass) {
                    newclass->mIsInterface = true;
                    fillclass(newclass, (tJSONObject*)(*i));
                    c->owned.emplace_back(newclass);
                }

            } else if (o_type == "UMLPrimitiveType") {
                if (c->getPackageType() == eElementType::ExternPackage) {
                    std::shared_ptr<MClass> newclass;

                    newclass = MClass::construct(id, "PrimitiveType", c);
                    fillclass(newclass, (tJSONObject*)(*i));
                    c->owned.emplace_back(newclass);
                } else {
                    std::shared_ptr<MClass> newprim;

                    newprim = MClass::construct(id, "PrimitiveType", c);
                    fillclass(newprim, (tJSONObject*)(*i));
                    c->owned.emplace_back(newprim);
                }
            } else if (o_type == "UMLDataType") {
                std::shared_ptr<MClass> newclass;

                newclass = MClass::construct(id, "dataType", c);
                fillclass(newclass, (tJSONObject*)(*i));
                c->owned.emplace_back(newclass);
            }
        }
    }
    //
    //  The id should be set already. Now check whether we have attributes defined.
    if (attributes != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=attributes->values.begin(); i!= attributes->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLAttribute") {
                auto newattr=MAttribute::construct(id, c);

                fillattribute(newattr, (tJSONObject*)(*i));
                c->Add(newattr);
            }
        }
    }
    //
    //  The id should be set already. Now check whether we have literals defined.
    if (literals != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=literals->values.begin(); i!= literals->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLEnumerationLiteral") {
                auto newattr=MAttribute::construct(id, c);

                fillattribute(newattr, (tJSONObject*)(*i));
                c->Add(newattr);
            }
        }
    }
    //
    //  Now check whether we have operations defined.
    if (operations != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=operations->values.begin(); i!= operations->values.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLOperation") {
                auto newop=MOperation::construct(id, c);

                filloperation(newop, (tJSONObject*)(*i));
                c->Operation.emplace_back(newop);
            }
        }
    }
    c->visibility = getvisibility(j);
    c->comment    = getstringattr(j, "documentation");
}

void fillpackage(std::shared_ptr<MPackage> pack, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    //
    //  Fill from tagged values.
    filltags(pack, j);
    //
    //  Setup the owned elements.
    tJSONArray*                   owned=(tJSONArray*)(findbyname(j, "ownedElements"));
    //
    //  Set the id of the package.
    pack->SetId(getstringattr(j, "_id"));
    //
    // Setup the comment field.
    pack->comment = getstringattr(j, "documentation");
#if 0
    //
    //  Check for a namespace tag. And set the namespace of the package.
    if (pack->HasTaggedValue("namespace")) {
        pack->mNameSpace = pack->GetTaggedValue("namespace");
    }
#endif
    if (owned != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->values.begin(); i!= owned->values.end(); ++i) {
            std::string t = getstringattr((tJSONObject*)(*i), "_type");

            if ((t == "UMLClass") || (t == "UMLSignal")) {
                std::shared_ptr<MClass> newclass;
                auto stereotype=model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                std::string id=getstringattr((tJSONObject*)(*i), "_id");
                std::string stype;

                if (stereotype) {
                    newclass = MClass::construct(id, stereotype, pack);
                } else {
                    stype = pack->GetDefaultStereotype();
                    if (!stype.empty()) {
                        newclass = MClass::construct(id, stype, pack);
                    } else {
                        if (pack->type == eElementType::ExternPackage) {
                            newclass = MClass::construct(id, "Extern", pack);
                        } else {
                            if (t == "UMLSignal") {
                                newclass = MClass::construct(id, "Signal", pack);
                            } else {
                                newclass = MClass::construct(id, "", pack);
                            }
                        }
                    }
                }
                if (newclass) {
                    if (t == "UMLInterface") {
                        newclass->mIsInterface = true;
                    }
                    fillclass(newclass, (tJSONObject*)(*i));
                    pack->Add(newclass);
                }
            } else if  (t == "UMLInterface") {
                std::shared_ptr<MClass> newclass;
                std::string id=getstringattr((tJSONObject*)(*i), "_id");
                std::string stype;

                newclass = MClass::construct(id, "Interface", pack);
                if (newclass) {
                    newclass->mIsInterface = true;
                    fillclass(newclass, (tJSONObject*)(*i));
                    pack->Add(newclass);
                }

            } else if (t == "UMLPrimitiveType") {
                if (pack->type == eElementType::ExternPackage) {
                    std::shared_ptr<MClass> newclass;
                    std::string id=getstringattr((tJSONObject*)(*i), "_id");

                    newclass = MClass::construct(id, "PrimitiveType", pack);
                    fillclass(newclass, (tJSONObject*)(*i));
                    pack->Add(newclass);
                } else {
                    std::shared_ptr<MClass> newprim;
                    std::string id=getstringattr((tJSONObject*)(*i), "_id");

                    newprim = MClass::construct(id, "PrimitiveType", pack);
                    if (newprim != 0) {
                        fillclass(newprim, (tJSONObject*)(*i));
                        pack->Add(newprim);
                    }
                }
            } else if (t == "UMLDataType") {
                std::shared_ptr<MClass> newclass;
                std::string id=getstringattr((tJSONObject*)(*i), "_id");

                newclass = MClass::construct(id, "dataType", pack);
                fillclass(newclass, (tJSONObject*)(*i));
                pack->Add(newclass);
            } else if (t=="UMLCollaboration") {
                std::shared_ptr<MClass> newclass;
                std::string id=getstringattr((tJSONObject*)(*i), "_id");

                newclass = MClass::construct(id, "Collaboration",  pack);
                fillclass(newclass, (tJSONObject*)(*i));
                pack->Add(newclass);
            } else if (t=="UMLPackage") {
                std::shared_ptr<MPackage>    newpack;
                std::string id          = getstringattr((tJSONObject*)(*i), "_id");
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));

                newpack=MPackage::construct(id, stereotype, pack);

                if (newpack) {
                    newpack->name=getstringattr((tJSONObject*)(*i), "name");
                    fillpackage(newpack, (tJSONObject*)(*i));
                    pack->Add(newpack);
                }
            } else if (t == "UMLDependency") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                auto newdep     = MDependency::construct(id, stereotype, pack);

                filldependency(newdep,  (tJSONObject*)(*i));
                pack->Dependency.emplace_back(newdep);

            } else if (t == "UMLAssociation") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                auto newassoc  = MAssociation::construct(id, pack);

                fillassoc(newassoc, (tJSONObject *) (*i));
                model->Add(newassoc);
            } else if (t == "UMLObject") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                auto newobj     = MObject::construct(id, stereotype, pack);

                fillobject(newobj,  (tJSONObject*)(*i));
                pack->Add(newobj);

            } else if (t == "UMLEnumeration") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                std::string stype;
                std::shared_ptr<MClass>     nclass;

                if (stereotype != 0) {
                    stype = stereotype->name;
                }
                if (!stype.empty()) {
                    nclass = MClass::construct(id, stype, pack);
                } else {
                    nclass = MClass::construct(id, "Enumeration", pack);
                }
                if (nclass != 0) {
                    fillclass(nclass, (tJSONObject*)(*i));
                    pack->Add(nclass);
                }
            }
        }
    }
}

void fillmodel(tJSONObject* j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   tags  = (tJSONArray*)(findbyname(j, "tags"));
    tJSONArray*                   owned = (tJSONArray*)(findbyname(j, "ownedElements"));

    if (tags != nullptr) {
        for (i=tags->values.begin(); i!= tags->values.end(); ++i) {
            std::string name=getstringattr((tJSONObject*)(*i), "name");
            std::string value=getstringattr((tJSONObject*)(*i), "value");
            model->AddTag(name, value);
        }
    }
    if (owned != nullptr) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->values.begin(); i!= owned->values.end(); ++i) {
            std::string  t          = getstringattr((tJSONObject*)(*i), "_type");
            std::shared_ptr<MPackage>    pack;
            std::string id          = getstringattr((tJSONObject*)(*i), "_id");

            if (t == "UMLPackage") {
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));

                pack=MPackage::construct(id, stereotype);
            } else if (t == "UMLModel") {
                pack = MPackage::construct(id, "UMLModel");
            }
            if (pack) {
                pack->name=getstringattr((tJSONObject*)(*i), "name");
                fillpackage(pack, (tJSONObject*)(*i));
                model->Add(pack);
            }
        }
    }
}

/*
 * This is the only function that gets used by main program
 */
std::shared_ptr<MModel> staruml_modelparser(const char* filename, const char* directory)
{
    model=MModel::construct();
    std::ifstream     infile(filename);
    std::list<tJSON*> nodelist;
    int                err = 0;
    struct stat        dirstat;
    std::string        path;
    //
    //  Set the directory to the current working directory.
    path = helper::getcwd();
    //
    //  Check if infile has been opened
    if (!infile.good()) {
        std::cerr << "Could not open file :" << filename << ": at :" << path << ":\n";
    }
    //
    //  First check for relative directory and create it if needed.
    if (directory[0]!='/') {
        path = path + "/" + directory;
    } else {
        path = directory;
    }
    err=stat(path.c_str(), &dirstat);
    if ((err == -1) && (errno==ENOENT)) {
        helper::mkdir(path, 0777);
    } else if ((err == 0) && (!(S_IFDIR & dirstat.st_mode))) {
        std::cerr << "Not a directory " << path << "\n";
        return (0);
    }
    helper::chdir(path);
    if (model != 0) {
        infile >> std::noskipws;
        root=parse(infile);
        //
        //  If parsing the model succeeded go-on and fill internal meta-model.
        if (root != nullptr) {
            //
            //  collect all defined stereotypes. They are defined a bit outside
            //  of the model.
            nodelist = findbytype(root, "UMLStereotype");
            for (auto & i : nodelist) {
                auto newstereotype = MStereotype::construct(getstringattr((tJSONObject*)(i), "name"), getstringattr((tJSONObject*)(i), "_id"));
                model->Add(newstereotype);
            }
            //
            //  collect all lifelines from sequence diagrams. They are needed to connect the
            //  messages to the classes.
            nodelist=findbytype(root, "UMLLifeline");
            for (auto & i : nodelist) {
                std::shared_ptr<MLifeLine> newlifeline;
                auto stereotype = model->StereotypeById(getstereotype((tJSONObject*)(i)));

                if (stereotype) {
                    if (stereotype->name == "WebPage") {
                        newlifeline = MWebPageLifeLine::construct(getstringattr((tJSONObject*)(i), "_id"));
                    } else if (stereotype->name == "JScript") {
                        newlifeline = MJSLifeLine::construct(getstringattr((tJSONObject*)(i), "_id"));
                    }
                } else {
                    newlifeline=MLifeLine::construct(getstringattr((tJSONObject*)(i), "_id"));
                }
                if (newlifeline) {
                    filllifeline(newlifeline, (tJSONObject*)(i));
                }
            }
            //
            //  collect all usecase from the model to get the Attributes defined in them.
            //  They are needed to connect the messages to the classes.
            nodelist=findbytype(root, "UMLUseCase");
            for (auto & i  : nodelist) {
                std::shared_ptr<MUseCase> newusecase;

                newusecase=MUseCase::construct(getstringattr((tJSONObject*)(i), "_id"));
                fillusecase(newusecase, (tJSONObject*)(i));
            }

            nodelist=findbytype(root, "UMLMessage");
            for (auto & i : nodelist) {
                std::shared_ptr<MMessage>    msg;
                auto stereotype=model->StereotypeById(getstereotype((tJSONObject*)(i)));
                std::string  id=getstringattr((tJSONObject*)(i), "_id");

                msg=MMessage::construct(id);
                if (msg != 0) {
                    msg->name=getstringattr((tJSONObject*)(i), "name");
                    fillmsg(msg, (tJSONObject*)(i));
                    if (stereotype) {
                        msg->stereotypes.emplace(stereotype->name, stereotype);
                    }
                }
            }

            nodelist=findbytype(root, "UMLProfile");
            for (auto & i : nodelist) {
                std::shared_ptr<MPackage> pack;
                std::string id=getstringattr((tJSONObject*)(i), "_id");

                pack=MPackage::construct(id, "UMLProfile");
                if (pack != 0) {
                    pack->name=getstringattr((tJSONObject*)(i), "name");
                    fillpackage(pack, (tJSONObject*)(i));
                    model->Add(pack);
                }
            }
            //
            //  Fill the model attributes.
            fillmodel((tJSONObject*)root);
            model->Complete();
#if 1
            for (auto& g : gTypesToComplete) {
                if (!g->name.empty()) {
                    // find the namespace from the packages.
                    g->mNameSpace = findnamespace(g);
                    //
                    //  Setup the type tree and collect all that have an incomplete setup.
                    g->mTypeTree = TypeNode::parse(g->name);
                    if (!g->mTypeTree.isCompositeType()) {
                        g->mTypeTree.mClassifier = g;
                    }
                    g->mTypeTree.fill(g->mNameSpace);
                    g->add();
                } else {
                    std::cerr << "No name for element-id: " << g->id << std::endl;
                }
            }
#else
            //
            //  Fill the classifiers in composite types.
            for (auto & g : gTypesToComplete) {
                g->mTypeTree.fill("");
                g->add();
            }
#endif
        } else {
        }
    }
    return (model);
}
