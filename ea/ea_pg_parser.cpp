//
// Copyright 2019 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef WIN32
#include <direct.h>
#endif

#include <errno.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include "helper.h"
#include "ea_pg_parser.h"
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
#include "mobject.h"
#include "mmodel.h"

#include "mstereotype.h"

static MModel* model = 0;

#if 0
void filltransition(MTransition* t, tJSONObject* j) ;
void fillpseudostate(MPseudoState* ps, tJSONObject* j) ;
void fillfinalstate(MFinalState* fs, tJSONObject* j) ;
void fillaction(MAction* act, tJSONObject* j) ;
void fillparameter(MParameter* p, tJSONObject *j) ;
/*
 * This is the JSON-Document of the model.
 */
tJSON*  root=0;
/*
 * This is the model we construct.
 */


tJSON* findbyname(tJSONObject*o, std::string name) {
    tJSON*                                  retval=0;
    std::map<std::string, tJSON*>::iterator i;

    if (o!=0) {
        i=o->value.find(name);
        if (i!=o->value.end()) {
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
        i=obj->value.find("_type");
        //
        // Check if found
        if (i!=obj->value.end()) {
            tJSONString* nodetype=(tJSONString*)(i->second);
            if (nodetype->value == type) {
                nodelist.push_back(node);
            }
        }
        for (i=obj->value.begin(); i != obj->value.end(); ++i) {
            if ((i->second->type == eJSON::eObject) || (i->second->type == eJSON::eArray)) {
                std::list<tJSON*> more=findbytype(i->second, type);
                nodelist.insert(nodelist.end(), more.begin(), more.end());
            }
        }

    } else if (node->type == eJSON::eArray) {
        tJSONArray*                   array;
        std::vector<tJSON*>::iterator i;

        array=(tJSONArray*)node;
        for (i=array->value.begin(); i != array->value.end(); ++i) {
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

    if (node->type == eJSON::eObject) {
        i=node->value.find(name);
        if ((i!=node->value.end()) && (i->second->type==eJSON::eString)) {
            tJSONString *s=(tJSONString*)(i->second);
            if (s!=0) {
                return (s->value);
            }
        }
    }
    return ("");
}

bool getboolean(tJSONObject* node, std::string name, bool defaultvalue = false) ;

bool getboolean(tJSONObject* node, std::string name, bool defaultvalue) {
    std::map<std::string, tJSON*>::iterator i;

    i=node->value.find(name);
    if ((i!=node->value.end()) && (i->second->type==eJSON::eBoolean)) {
        tJSONBoolean *b=(tJSONBoolean*)(i->second);
        return (b->value);
    } else {
        return defaultvalue;
    }

    return (false);
}

std::string getstereotype(tJSONObject* node) {
    std::string ref;
    std::map<std::string, tJSON*>::iterator i;

    i=node->value.find("stereotype");
    if (i!=node->value.end()) {
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
    return mUnknownMessage;
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

    i=j->value.find(name);
    if ((i!=j->value.end()) && (i->second->type==eJSON::eObject)) {
        ref=getstringattr((tJSONObject*)(i->second), "$ref");
    }
    return (ref);
}

void filltags(MElement* e, tJSONObject* j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   tags=(tJSONArray*)(findbyname(j, "tags"));

    if (tags != 0) {
        for (i=tags->value.begin(); i!= tags->value.end(); ++i) {
            std::string name=getstringattr((tJSONObject*)(*i), "name");
            std::string value=getstringattr((tJSONObject*)(*i), "value");
            e->AddTag(name, value);
        }
    }
}


void fillelement(MElement* e, tJSONObject* j) {
    filltags(e, j);
    e->name                 = getstringattr(j, "name");
    e->visibility           = getvisibility(j);
    e->comment              = getstringattr(j, "documentation");

    MStereotype* stereotype = model->StereotypeById(getstereotype((tJSONObject*)(j)));
    if (stereotype != 0) {
        e->stereotypes.insert(std::pair<std::string, MStereotype*>(stereotype->name, stereotype));
    }
}


void filllifeline(MLifeLine* l, tJSONObject* j) {
    //
    //  Fill with tagged values.
    fillelement(l, j);
    //
    //  Set the Source Reference if we find one.
    std::string ref=getreference(j, "represent");

    if (ref.size()!=0) {
        l->role_ref = ref ;
    } else {
    }
}

void fillmsg(MMessage* m, tJSONObject *j)
{
    //
    //  Fill with tagged values.
    fillelement(m, j);

    m->mtype             = getmsgtype(j);
    m->isConcurrent      = getboolean(j, "isConcurrentIteration");
    //
    //  Set the Source Reference if we find one.
    std::string ref=getreference(j, "source");

    if (ref.size()!=0) {
        m->source_ref = ref ;
    } else {
    }
    //
    //  Set the Target Reference if we find one.
    ref=getreference(j, "target");

    if (ref.size()!=0) {
        m->target_ref = ref ;
    } else {
    }
}


void fillassocend(MAssociationEnd* e, tJSONObject *j)
{
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   owned=(tJSONArray*)(findbyname(j, "ownedElements"));
    //
    //  Fill with tagged values.
    fillelement(e, j);
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (owned!=0) {

    }
    e->Aggregation       = getaggregation(j);
    e->Multiplicity      = getstringattr(j, "multiplicity");
    e->Navigable         = getboolean(j, "navigable", true);
    e->defaultValue      = getstringattr(j, "defaultValue");
    //
    //  Set the Classifier Reference if we find one.
    std::string ref=getreference(j, "reference");

    if (ref.size()!=0) {
        e->ClassifierRef = ref ;
        MReference::Add(ref);
    } else {

    }
}

void filledge(MEdge* p, tJSONObject* j) {
    fillelement(p, j);

    p->SourceRef = getreference(j, "source");
    p->TargetRef = getreference(j, "target");
    p->Guard     = getstringattr(j, "guard");
}

void fillnode(MNode* p, tJSONObject* j) {
    fillelement(p, j);
    p->TargetRef = getreference(j, "target");
}

void fillactionnode(MActionNode* p, tJSONObject* j) {
    tJSONArray*                   ipins      = (tJSONArray*)(findbyname(j, "inputs"));
    tJSONArray*                   opins      = (tJSONArray*)(findbyname(j, "outputs"));
    std::vector<tJSON*>::iterator i;

    p->SubRef             = getreference(j, "subactivity");
    p->isLocallyReentrant = getboolean(j, "isLocallyReentrant");
    p->isSynchronous      = getboolean(j, "isSynchronous");
    p->Language           = getstringattr(j, "language");
    p->Body               = getstringattr(j, "body");
    p->Kind               = getactionkind(j);

    if (ipins!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=ipins->value.begin(); i!= ipins->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");
            MPin* newpin = MPin::construct(id, p);

            fillparameter(newpin, (tJSONObject*)(*i));
            newpin->Direction = "in";
            p->InputPins.push_back(newpin);
        }
    }
    if (opins!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=opins->value.begin(); i!= opins->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");
            MPin* newpin = MPin::construct(id, p);

            fillparameter(newpin, (tJSONObject*)(*i));
            newpin->Direction = "out";
            p->OutputPins.push_back(newpin);
        }
    }
}

void fillactivity(MActivity* p, tJSONObject* j) {
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
    if (nodes!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=nodes->value.begin(); i!= nodes->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");
            MNode* newnode= 0;

            if (o_type == "UMLInitialNode") {
                MInitialNode* node = MInitialNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLFinalNode") {
                MFinalNode* node = MFinalNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLAction") {
                MActionNode* node = MActionNode::construct(id, p);

                fillactionnode(node, (tJSONObject*)(*i));
                newnode = node;
            } else if (o_type == "UMLDecisionNode") {
                MDecisionNode* node = MDecisionNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLMergeNode") {
                MMergeNode* node = MMergeNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLJoinNode") {
                MJoinNode* node = MJoinNode::construct(id, p);

                newnode = node;
            } else if (o_type == "UMLForkNode") {
                MForkNode* node = MForkNode::construct(id, p);

                newnode = node;
            } else {
                MNode* node = MNode::construct(id, p);

                newnode = node;
            }
            if (newnode != 0) {
                fillnode(newnode, (tJSONObject*)(*i));
                p->Nodes.push_back(newnode);
            }
        }
    }
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (edges!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=edges->value.begin(); i!= edges->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            MEdge* edge = MEdge::construct(id, p);

            filledge(edge, (tJSONObject*)(*i));

            p->Edges.push_back(edge);
        }
    }
}

void fillparameter(MParameter* p, tJSONObject *j) {
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

    if (ref.size()==0) {
        p->ClassifierName = getstringattr(j, "type");
    } else {
        p->ClassifierRef = ref;
        MReference::Add(ref);
    }
    p->Classifier=0;
}

void filloperation(MOperation* o, tJSONObject *j) {
    tJSONArray*                   owned=(tJSONArray*)(findbyname(j, "ownedElements"));
    tJSONArray* parameters=(tJSONArray*)(findbyname(j, "parameters"));
    std::vector<tJSON*>::iterator i;
    //
    //  Fill with tagged values.
    fillelement(o, j);

    o->isStatic       = getboolean(j, "isStatic");
    o->isAbstract     = getboolean(j, "isAbstract");
    if (parameters != 0) {
        //
        //  *i points to an tJSONObject. For all parameters we create the
        //  model elements.
        for (i=parameters->value.begin(); i!= parameters->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLParameter") {
                MParameter *p=MParameter::construct(id, o);

                fillparameter(p, (tJSONObject*)(*i));
                o->Parameter.push_back(p);
            }
        }
    }
    if (owned!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->value.begin(); i!= owned->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLActivity") {
               MActivity* newact=MActivity::construct(id, o);

               fillactivity(newact,  (tJSONObject*)(*i));
               o->Activity = newact;
            }
        }
    }
}


void fillassoc(MAssociation* a, tJSONObject *j)
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
    if (ends!=0) {
        std::string     id      = getstringattr(ends, "_id");
        MAssociationEnd *newend = MAssociationEnd::construct(id, a);

        fillassocend(newend, ends);
        a->AddEnd(newend);
        model->Add(newend);
    }
    ends=(tJSONObject*)findbyname(j, "end2");
    if (ends!=0) {
        std::string     id      = getstringattr(ends, "_id");
        MAssociationEnd *newend = MAssociationEnd::construct(id, a);

        fillassocend(newend, ends);
        a->AddEnd(newend);
        model->Add(newend);
    }
    a->comment = getstringattr(j, "documentation");
}

void fillattribute(MAttribute* a, tJSONObject *j) {
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

    if (ref.size()==0) {
        a->ClassifierName = getstringattr(j, "type");
    } else {
        a->ClassifierRef = ref;
        MReference::Add(ref);
    }
}

void fillslot(MAttribute* a, tJSONObject *j) {
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

    if (ref.size()==0) {
        a->ClassifierName = getstringattr(j, "type");
    } else {
        a->ClassifierRef = ref;
        MReference::Add(ref);
    }
}

void fillnormalstate(MState* ns, tJSONObject* j) {
    tJSONArray* regions = (tJSONArray*)(findbyname(j, "regions"));
    tJSONArray* doact   = (tJSONArray*)(findbyname(j, "doActivities"));

    std::vector<tJSON*>::iterator i;

    ns->name           = getstringattr(j, "name");
    ns->visibility     = getvisibility(j);
    ns->comment        = getstringattr(j, "documentation");

    if (doact != 0) {
        for (i=doact->value.begin(); i!= doact->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLOpaqueBehavior") {
                MAction *act=MAction::construct(id, ns);

                fillaction(act, (tJSONObject*)(*i));
                ns->DoActions.push_back(act);
            }
        }
    }
    //
    //  Process composite states
    if (regions != 0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=regions->value.begin(); i!= regions->value.end(); ++i) {
            std::vector<tJSON*>::iterator x;
            tJSONArray*                   vertices=(tJSONArray*)(findbyname((tJSONObject*)(*i), "vertices"));
            tJSONArray*                   transitions=(tJSONArray*)(findbyname((tJSONObject*)(*i), "transitions"));

            if (vertices != 0) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=vertices->value.begin(); x!= vertices->value.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    if (o_type == "UMLPseudostate") {
                        MPseudoState *ps=MPseudoState::construct(id, ns);

                        fillpseudostate(ps, (tJSONObject*)(*x));
                        ns->States.push_back(ps);
                    } else if (o_type == "UMLFinalState") {
                        MFinalState *fs=MFinalState::construct(id, ns);

                        fillfinalstate(fs, (tJSONObject*)(*x));
                        ns->States.push_back(fs);
                    } else if (o_type == "UMLState") {
                        MState *nos=MState::construct(id, ns);

                        fillnormalstate(nos, (tJSONObject*)(*x));
                        ns->States.push_back(nos);
                    }
                }
            }
            if (transitions != 0) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=transitions->value.begin(); x!= transitions->value.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    MTransition* newtransition=MTransition::construct(id, ns);

                    if (newtransition != 0) {
                        filltransition(newtransition, (tJSONObject*)(*x));
                        ns->Transitions.push_back(newtransition);
                    }
                }
            }
        }
    }
}

void fillfinalstate(MFinalState* fs, tJSONObject* j) {
    fillnormalstate(fs, j);
}

void fillpseudostate(MPseudoState* ps, tJSONObject* j) {
    fillnormalstate(ps, j);
    ps->kind        = getstringattr(j, "kind");
}

void fillevent(MEvent* ev, tJSONObject* j) {
    ev->name       = getstringattr(j, "name");
    ev->visibility = getvisibility(j);
    ev->comment    = getstringattr(j, "documentation");
    ev->kind       = getstringattr(j, "kind");
}

void fillaction(MAction* act, tJSONObject* j) {
    act->name       = getstringattr(j, "name");
    act->visibility = getvisibility(j);
    act->comment    = getstringattr(j, "documentation");
}

void filltransition(MTransition* t, tJSONObject* j) {
    tJSONArray*                   trigger=(tJSONArray*)(findbyname(j, "triggers"));
    tJSONArray*                   effects=(tJSONArray*)(findbyname(j, "effects"));
    std::vector<tJSON*>::iterator i;

    t->name       = getstringattr(j, "name");
    t->visibility = getvisibility(j);
    t->comment    = getstringattr(j, "documentation");
    t->kind       = getstringattr(j, "kind");
    t->guard      = getstringattr(j, "guard");
    t->from_ref   = getreference(j, "source");
    t->to_ref     = getreference(j, "target");
    if (trigger != 0) {
        //
        //  *i points to an tJSONObject. For all triggers we create the
        //  model elements.
        for (i=trigger->value.begin(); i!= trigger->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLEvent") {
                MEvent *ev=MEvent::construct(id, t);

                fillevent(ev, (tJSONObject*)(*i));
                t->events.push_back(ev);
            }
        }
    }
    if (effects != 0) {
        //
        //  *i points to an tJSONObject. For all triggers we create the
        //  model elements.
        for (i=effects->value.begin(); i!= effects->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLOpaqueBehavior") {
                MAction *act=MAction::construct(id, t);

                fillaction(act, (tJSONObject*)(*i));
                t->actions.push_back(act);
            }
        }
    }
}

void fillstatemachine(MStatemachine* s, tJSONObject* j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   regions=(tJSONArray*)(findbyname(j, "regions"));
    //
    //  Fill with tagged values.
    filltags(s, j);
    if (regions != 0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=regions->value.begin(); i!= regions->value.end(); ++i) {
            std::vector<tJSON*>::iterator x;
            tJSONArray*                   vertices=(tJSONArray*)(findbyname((tJSONObject*)(*i), "vertices"));
            tJSONArray*                   transitions=(tJSONArray*)(findbyname((tJSONObject*)(*i), "transitions"));

            if (vertices != 0) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=vertices->value.begin(); x!= vertices->value.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    if (o_type == "UMLPseudostate") {
                        MPseudoState *ps=MPseudoState::construct(id, s);

                        fillpseudostate(ps, (tJSONObject*)(*x));
                        s->states.push_back(ps);
                    } else if (o_type == "UMLFinalState") {
                        MFinalState *fs=MFinalState::construct(id, s);

                        fillfinalstate(fs, (tJSONObject*)(*x));
                        s->states.push_back(fs);
                    } else if (o_type == "UMLState") {
                        MState *ns=MState::construct(id, s);

                        fillnormalstate(ns, (tJSONObject*)(*x));
                        s->states.push_back(ns);
                    }
                }
            }
            if (transitions != 0) {
                //
                //  *x points to an tJSONObject. For all UMLClass objects we create the
                //  model elements.
                for (x=transitions->value.begin(); x!= transitions->value.end(); ++x) {
                    std::string o_type=getstringattr((tJSONObject*)(*x), "_type");
                    std::string id=getstringattr((tJSONObject*)(*x), "_id");

                    MTransition* newtransition=MTransition::construct(id, s);

                    if (newtransition != 0) {
                        filltransition(newtransition, (tJSONObject*)(*x));
                        s->transitions.push_back(newtransition);
                    }
                }
            }
        }
    }
}

void fillusecase(MUseCase*u, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   attributes=(tJSONArray*)(findbyname(j, "attributes"));
    //
    //  Fill with tagged values.
    filltags(u, j);
    //
    //  The id should be set already. Now check whether we have attributes defined.
    if (attributes!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=attributes->value.begin(); i!= attributes->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLAttribute") {
                MAttribute *newattr=MAttribute::construct(id, u);

                fillattribute(newattr, (tJSONObject*)(*i));
                u->Attribute.push_back(newattr);
            }
        }
    }
    u->visibility = getvisibility(j);
    u->comment    = getstringattr(j, "documentation");
}

void filldependency(MDependency* d, tJSONObject*j) {
    //
    //  Fill with tagged values.
    filltags(d, j);

    d->name              = getstringattr(j, "name");
    d->visibility        = getvisibility(j);
    d->comment           = getstringattr(j, "documentation");

    d->src_ref      = getreference(j, "source");
    d->target_ref   = getreference(j, "target");
}


void fillgeneralization(MGeneralization* d, tJSONObject*j) {
    //
    //  Fill with tagged values.
    filltags(d, j);

    d->name        = getstringattr(j, "name");
    d->visibility  = getvisibility(j);
    d->comment     = getstringattr(j, "documentation");

    d->derived_ref = getreference(j, "source");
    d->base_ref    = getreference(j, "target");
}

void fillobject(MObject* o, tJSONObject *j) {
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
    if (owned!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->value.begin(); i!= owned->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type=="UMLLink") {
                MLink* newlink=MLink::construct(id, o);

                fillassoc(newlink, (tJSONObject*)(*i));
                o->Add(newlink);
            }
        }
    }
    if (sl != 0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=sl->value.begin(); i!= sl->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLSlot") {
                MAttribute *newattr=MAttribute::construct(id, o);

                fillslot(newattr, (tJSONObject*)(*i));
                o->Add(newattr);
            }
        }
    }
}

void fillclass(MClass*c, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    tJSONArray*                   owned      = (tJSONArray*)(findbyname(j, "ownedElements"));
    tJSONArray*                   attributes = (tJSONArray*)(findbyname(j, "attributes"));
    tJSONArray*                   operations = (tJSONArray*)(findbyname(j, "operations"));
    tJSONArray*                   literals   = (tJSONArray*)(findbyname(j, "literals"));
    std::string                   name       = getstringattr(j, "name");
    //
    //  Use of this method is mandatory.
    c->SetName(name);
    //
    //  Fill with tagged values.
    filltags(c, j);
    //
    //  The id should be set already. Now check whether we have owned elements.
    if (owned!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->value.begin(); i!= owned->value.end(); ++i) {
            std::string o_type = getstringattr((tJSONObject*)(*i), "_type");
            std::string id     = getstringattr((tJSONObject*)(*i), "_id");

            if (o_type=="UMLAssociation") {
                MAssociation *newassoc=MAssociation::construct(id, c);

                fillassoc(newassoc, (tJSONObject*)(*i));
                c->Add(newassoc);
                model->Add(newassoc);
            } else if (o_type == "UMLAttribute") {
                MAttribute *newattr=MAttribute::construct(id, c);

                fillattribute(newattr, (tJSONObject*)(*i));
                c->Add(newattr);
            } else if (o_type == "UMLDependency") {
                MStereotype* stereotype=model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                MDependency* newdep=MDependency::construct(id, stereotype, c);

                filldependency(newdep,  (tJSONObject*)(*i));
                c->Dependency.push_back(newdep);

            } else if (o_type == "UMLGeneralization") {
                MGeneralization* newgen=MGeneralization::construct(id, 0, c);

                fillgeneralization(newgen,  (tJSONObject*)(*i));
                c->Generalization.push_back(newgen);

            } else  if (o_type == "UMLActivity") {
                MActivity* newact=MActivity::construct(id, c);

                fillactivity(newact,  (tJSONObject*)(*i));
                c->Activity.push_back(newact);

            } else if (o_type == "UMLStateMachine") {
                switch (c->type) {
                case eSimObject:
                {
                    MSimStatemachine* newstatemachine=MSimStatemachine::construct(id, c);

                    if (newstatemachine!=0) {
                        fillstatemachine(newstatemachine, (tJSONObject*)(*i));
                    }
                    c->statemachine=newstatemachine;
                }
                    break;
                default:
                    break;
                }
            }
        }
    }
    //
    //  The id should be set already. Now check whether we have attributes defined.
    if (attributes!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=attributes->value.begin(); i!= attributes->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLAttribute") {
                MAttribute *newattr=MAttribute::construct(id, c);

                fillattribute(newattr, (tJSONObject*)(*i));
                c->Add(newattr);
            }
        }
    }
    //
    //  The id should be set already. Now check whether we have literals defined.
    if (literals!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=literals->value.begin(); i!= literals->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLEnumerationLiteral") {
                MAttribute *newattr=MAttribute::construct(id, c);

                fillattribute(newattr, (tJSONObject*)(*i));
                c->Add(newattr);
            }
        }
    }
    //
    //  Now check whether we have operations defined.
    if (operations!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=operations->value.begin(); i!= operations->value.end(); ++i) {
            std::string o_type=getstringattr((tJSONObject*)(*i), "_type");
            std::string id=getstringattr((tJSONObject*)(*i), "_id");

            if (o_type == "UMLOperation") {
                MOperation *newop=MOperation::construct(id, c);

                filloperation(newop, (tJSONObject*)(*i));
                c->Operation.push_back(newop);
            }
        }
    }
    c->visibility = getvisibility(j);
    c->comment    = getstringattr(j, "documentation");
}

void fillpackage(MPackage*pack, tJSONObject *j) {
    std::vector<tJSON*>::iterator i;
    //
    //  Fill from tagged values.
    filltags(pack, j);
    tJSONArray*                   owned=(tJSONArray*)(findbyname(j, "ownedElements"));
    //
    //  Set the id of the package.
    pack->SetId(getstringattr(j, "_id"));
    pack->comment = getstringattr(j, "documentation");
    if (owned!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->value.begin(); i!= owned->value.end(); ++i) {
            std::string t = getstringattr((tJSONObject*)(*i), "_type");

            if (t=="UMLClass") {
                MClass *newclass=0;
                MStereotype* stereotype=model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                std::string id=getstringattr((tJSONObject*)(*i), "_id");
                std::string stype;

                if (stereotype != 0) {
                    newclass = MClass::construct(id, stereotype, pack);
                } else {
                    stype = pack->GetDefaultStereotype();
                    if (!stype.empty()) {
                        newclass = MClass::construct(id, stype, pack);
                    } else {
                        if (pack->type == eExternPackage) {
                            newclass = MClass::construct(id, "Extern", pack);
                        } else {
                            newclass = MClass::construct(id, "", pack);
                        }
                    }
                }
                if (newclass != 0) {
                    fillclass(newclass, (tJSONObject*)(*i));
                    pack->Add(newclass);
                }
            } else if (t == "UMLPrimitiveType") {
                if (pack->type == eExternPackage) {
                    MClass *newclass=0;
                    std::string id=getstringattr((tJSONObject*)(*i), "_id");

                    newclass = MClass::construct(id, "Extern", pack);
                    fillclass(newclass, (tJSONObject*)(*i));
                    pack->Add(newclass);
                } else {
                    MClass *newprim=0;
                    std::string id=getstringattr((tJSONObject*)(*i), "_id");

                    newprim = MClass::construct(id, "PrimitiveType", pack);
                    if (newprim != 0) {
                        fillclass(newprim, (tJSONObject*)(*i));
                        pack->Add(newprim);
                    }
                }
            } else if (t == "UMLDataType") {
                MClass *newclass=0;
                std::string id=getstringattr((tJSONObject*)(*i), "_id");

                newclass = MClass::construct(id, "dataType", pack);
                fillclass(newclass, (tJSONObject*)(*i));
                pack->Add(newclass);
            } else if (t=="UMLCollaboration") {
                MClass* newclass=0;
                std::string id=getstringattr((tJSONObject*)(*i), "_id");

                newclass = MClass::construct(id, "Collaboration",  pack);
                fillclass(newclass, (tJSONObject*)(*i));
                pack->Add(newclass);
            } else if (t=="UMLPackage") {
                MPackage    *newpack    = 0;
                std::string id          = getstringattr((tJSONObject*)(*i), "_id");
                MStereotype* stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));

                newpack=MPackage::construct(id, stereotype, pack);

                if (newpack != 0) {
                    newpack->name=getstringattr((tJSONObject*)(*i), "name");
                    fillpackage(newpack, (tJSONObject*)(*i));
                    pack->Add(newpack);
                }
            } else if (t == "UMLDependency") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                MStereotype* stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                MDependency* newdep     = MDependency::construct(id, stereotype, pack);

                filldependency(newdep,  (tJSONObject*)(*i));
                pack->Dependency.push_back(newdep);

            } else if (t == "UMLObject") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                MStereotype* stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                MObject*     newobj     = MObject::construct(id, stereotype, pack);

                fillobject(newobj,  (tJSONObject*)(*i));
                pack->Add(newobj);

            } else if (t == "UMLEnumeration") {
                std::string  id         = getstringattr((tJSONObject*)(*i), "_id");
                MStereotype* stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));
                std::string stype;
                MClass*     nclass=0;

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

    if (tags != 0) {
        for (i=tags->value.begin(); i!= tags->value.end(); ++i) {
            std::string name=getstringattr((tJSONObject*)(*i), "name");
            std::string value=getstringattr((tJSONObject*)(*i), "value");
            model->AddTag(name, value);
        }
    }
    if (owned!=0) {
        //
        //  *i points to an tJSONObject. For all UMLClass objects we create the
        //  model elements.
        for (i=owned->value.begin(); i!= owned->value.end(); ++i) {
            std::string  t          = getstringattr((tJSONObject*)(*i), "_type");
            MPackage*    pack       = 0;
            std::string id          = getstringattr((tJSONObject*)(*i), "_id");

            if (t == "UMLPackage") {
                MStereotype* stereotype = model->StereotypeById(getstereotype((tJSONObject*)(*i)));

                pack=MPackage::construct(id, stereotype);
            } else if (t == "UMLModel") {
                pack = MPackage::construct(id, "UMLModel");
            }
            if (pack != 0) {
                pack->name=getstringattr((tJSONObject*)(*i), "name");
                fillpackage(pack, (tJSONObject*)(*i));
                model->Add(pack);
            }
        }
    }
}
#endif
/*
 * This is the only function that gets used by main program
 */
MModel* ea_pg_modelparser(const char* filename, const char* directory)
{
    model = MModel::construct();
    std::ifstream     infile(filename);
//    std::list<tJSON*> nodelist;
    int                err=0;
    struct stat        dirstat;
    std::string        path;
    //
    //  Set the directory to the current working directory.
	path = helper::getcwd();
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
    } else if ((err==0) && (!S_ISDIR(dirstat.st_mode))) {
        std::cerr << "Not a directory " << path << "\n";
        return (0);
    }
    helper::chdir(path);

	if (model != 0) {
    }
    return (model);
}
