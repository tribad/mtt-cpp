//
// Copyright 2018 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include <sstream>
#include "melement.h"
#include "medge.h"
#include "cedge.h"
#include "mnode.h"
#include "mparameter.h"
#include "mpin.h"
#include "cpin.h"
#include "mactionnode.h"
#include "mactivity.h"
#include "cactivity.h"
#include "mactionnode.h"
#include "cactionnode.h"
#include "mdecisionnode.h"
#include "cdecisionnode.h"
#include "mmergenode.h"
#include "cmergenode.h"

#include "mattribute.h"
#include "cattribute.h"

#include "massociationend.h"
#include "cassociationend.h"

#include "mclass.h"
#include "cclassbase.h"

#include "csignalclass.h"
#include "cmessageclass.h"

std::string CActivity::FQN() const {
    return name;
}

void CActivity::Prepare(void) {
    if (!prepDone) {
        for (auto & ie : Edges) {
            auto edge = std::dynamic_pointer_cast<CEdge>(*ie);
            //
            //  Get the source element node or pin
            auto s = *edge->Source;
            //
            //  Outgoings contains all edges outgoing from node and pin
            //  So checking for node based source.
            if (s && s->IsNodeBased()) {
                auto node = std::dynamic_pointer_cast<MNode>(s);
                //
                //  Putting empty or else guarded edges to the end
                //  All others to the front. This way we get the edges that are
                //  coded with the if (<guard) statement first.
                if ((edge->Guard.empty()) || (edge->Guard == "else")) {
                    node->Outgoing.push_back(ie);
                } else {
                    node->Outgoing.insert(node->Outgoing.begin(), ie);
                }
            } else if (s && (s->type == eElementType::Pin)) {
                auto nodeparent = std::dynamic_pointer_cast<MNode>(*s->parent);
                if ((edge->Guard.empty()) || (edge->Guard == "else")) {
                    nodeparent->Outgoing.push_back(ie);
                } else {
                    nodeparent->Outgoing.insert(nodeparent->Outgoing.begin(), ie);
                }
            }
            auto t = *std::dynamic_pointer_cast<CEdge>(*ie)->Target;
            if (t && t->IsNodeBased()) {
                auto node = std::dynamic_pointer_cast<MNode>(t);
                node->Incoming.push_back(ie);
            } else if (t && (t->type == eElementType::Pin)) {
                auto nodeparent = std::dynamic_pointer_cast<MNode>(*t->parent);
                nodeparent->Incoming.push_back(ie);
            }
        }
        for (auto & mi : Nodes) {
            mi->Prepare();
        }
        prepDone = true;
    }
}

std::vector<std::shared_ptr<MNode>> CActivity::GetInitialNode() {
    std::vector<std::shared_ptr<MNode>> retval;

    //
    //  Go along the nodes.
    for (auto & ni : Nodes) {
        auto node = std::dynamic_pointer_cast<MNode>(*ni);
        //
        //  Check that we have no incoming edges but outgoings.
        if ((node->Incoming.empty() && node->InputPins.empty()) &&
            (!node->Outgoing.empty() || !node->OutputPins.empty())) {
            //
            //  The initial node gets processed somehow special.
            if (node->type == eElementType::InitialNode) {
                //
                //  Check that the outgoing is not empty.
                //  Initials cannot have pins.
                if (!node->Outgoing.empty()) {
                    //
                    //  We expect multiple outgoing edges on an initial node.
                    //  if they have guards its somehow special. But ignored here.
                    for (auto & o : node->Outgoing) {
                        auto edge = std::dynamic_pointer_cast<MEdge>(*o);
                        //
                        //  Set a token in all targets if not already done.
                        auto t = std::dynamic_pointer_cast<MNode>(*edge->Target);
                        //
                        //  The outgoing edges targets are all initials.
                        retval.push_back(t);
                        t->Level = 0;
                    }
                    node->Level = 0;
                }
            } else {
                //
                //  If not an initial node we got the start.
                node->Level = 0;
                retval.push_back(node);
            }
        }
    }
    return retval;
}

void CActivity::Dump(std::shared_ptr<MModel> aModel) {

}

std::list<std::shared_ptr<MElement>> CActivity::CreateBlock(std::shared_ptr<MElement> aStart) {
    std::list<std::shared_ptr<MElement>> retval;
    //
    //  We have two starting points. On Edges or on Actions.
    if (aStart) {
        std::shared_ptr<MElement> s = aStart;
        //
        //  Go along the edges until a break.
        while (s) {
            if (s->type == eElementType::Edge) {
                auto edge = std::dynamic_pointer_cast<MEdge>(s);
                auto node = edge->GetTargetNode();

                if ((node && (node->Outgoing.size() > 1u)) || (node && (node->Incoming.size() > 1u))) {
                    mBlock.push_back(CreateBlock(node));
                    retval.push_back(edge);
                    break;
                } else {
                    retval.push_back(edge);
                    s = node;
                }

            } else if (s->IsNodeBased()) {
                auto node = std::dynamic_pointer_cast<MNode>(s);

                if (node->Outgoing.size() == 1u) {
                    //
                    //  As we do have only one outgoing but not more
                    //  than one incoming. This is only the next step
                    //  in the block
                    if (node->Incoming.size() > 1u) {
                        std::list<std::shared_ptr<MElement>> temp;

                        temp = CreateBlock(node->Outgoing[0]);
                        temp.push_front(node);
                        mBlock.push_back(temp);
                        break;
                    } else {
                        retval.push_back(node);
                        s = node->Outgoing[0];
                    }
                } else if (node->Outgoing.size() > 1u) {
                    retval.push_back(node);
                    for (auto & o : node->Outgoing) {
                        mBlock.push_back(CreateBlock(*o));
                    }
                    //
                    //  Stop the block
                    break;
                } else {
                    //
                    //  ToDo: This is the final node.
                    break;
                }
            } else {
                //
                // ToDo: Give error cause here something goes wrong.
            }
        }
    }

    return retval;
}

void CActivity::DumpCxx(std::ostream &src) {
    auto il = GetInitialNode();


    for (auto & i : il) {
        mBlock.push_back(CreateBlock(i));
    }

    int blockcount = 0;
    for (auto & b : mBlock) {
        if (!b.empty()) {
            src << "//\n"
                   "//  " << ++blockcount << "\n";

            for (auto & be : b) {
                if (be->type == eElementType::Edge) {
                    auto  ed = std::dynamic_pointer_cast<MEdge>(be);

                    src << "// " ;
                    if (ed->GetSource() != nullptr) {
                        src << ed->GetSource()->name;
                    }
                    src << " edge ";
                    if (ed->GetTargetNode() != nullptr) {
                        src << ed->GetTargetNode()->name << std::endl;
                    }
                } else {
                    src << "//  "  << be->name << std::endl;
                }
            }
        }
    }

#if 0
    //
    //  Reset the parser infos in all nodes/edges and Pins
    //  This is needed for activities that are used in different operations/behaviour
    for (std::vector<MNode*>::iterator x=Nodes.begin(); x != Nodes.end(); ++x) {
        (*x)->Level = 0;
        for (std::vector<MPin*>::iterator y = (*x)->InputPins.begin(); y != (*x)->InputPins.end(); ++y) {
            ((CPin*)(*y))->Token.clear();
        }
    }
    for (std::vector<MEdge*>::iterator x = Edges.begin(); x != Edges.end(); ++x) {
        ((CEdge*)(*x))->Level = 0;
    }
    //
    //  Search all Initial Nodes.
    bool didIt = true;
    //
    //  Going through all active nodes. We do not know what nodes has a complete segment.
    //  But we run along the edges until one segment completes.
    while (!activNodes.empty() && (didIt)) {
        std::vector<MNode*>::iterator a;

        for (a = activNodes.begin(); a != activNodes.end(); ++a) {
            if ((*a)->HasAllToken()) {
                //
                //  Each execution path has its own token.
                DumpCxx(src, *a);
                didIt = true;
                break;
            }
        }
        if (!didIt) {
            std::cerr << "Cannot complete activity\n";
            break;
        }
    }
    //
    //  Create a list of segments.
    //  A segment starts and ends with an edge.
    //  They are later used to generate the code from.
    //  A Segment starts on an initial node or on a node that has more than one outgoing edge.
    //  The segment ends if an edge ends on a node where more than one incoming edges are detected.
    std::vector<MNode*> starts = GetInitialNode();
    //
    //  Starting on each initial node.
    for (auto s : starts) {
        //
        //  Going along each of the outgoings. They build each a segment.
        for (auto o : s->Outgoing) {
            segments.push_back(CreateSegment(static_cast<CEdge*>(o)));
        }
    }
    //
    //  Going along the segments.
    std::list<CSegment>::iterator si = segments.begin();
    while (si != segments.end()) {
        CSegment s = *si;
        // Within each segment all actions are dumped
        // if the starting node has all tokens set.
        CEdge* start = *(s.path.begin());
        //
        //  Check if it is nodebased
        if ((start->Source != nullptr) && (start->Source->IsNodeBased())) {
            MNode* sn = static_cast<MNode*>(start->Source);
            //
            // Check if ready for dumping.
            if (sn->HasAllToken()) {
                //
                //  Checking for guards and the order of dumping multiple
                //  outgoings.
                if (!start->Guard.empty()) {
                    CActionNode* an = static_cast<CActionNode*>(sn);

                    if (start->Guard == "else") {
                        //
                        //  The else part can only be dumped if all other guards have been
                        //  already dumped
                        if ((an->DumpState+1u) == an->Outgoing.size()) {
                            src << "// } else {\n";
                            //
                            //  Dump the nodes withing this segment.
                            Dump(src, *si);
                        } else {
                            continue;
                        }
                    } else {
                        //
                        //  a non else guard. Checking if we are the first guard.
                        if (an->DumpState == 0) {
                            src << "// if (" << start->Guard << ") {\n";
                        } else {
                            src << "// } else if (" << start->Guard << ") {\n";
                        }
                        //
                        //  Dump the nodes withing this segment.
                        Dump(src, *si);
                        an->DumpState++;
                    }
                } else {
                    //
                    //  Ok dump all nodes within this segment.
                    Dump(src, *si);
                }
                si = segments.erase(si);
            } else {
                ++si;
            }
        } else {
            ++si;
        }
    }
    //
    //  Get the lowest level nodes.
    std::vector<MNode*> starts = GetInitialNode();
    //
    //  Fill the levels, by sending a level token into the tree.
    for (auto st : starts) {
        MToken lt(0, st);

        SendToken(st, lt);
    }
    //
    //  Check if all nodes have a level.
    //  Not beeing so indicates an incomplete tree that would not give
    //  Some usefull code. So we would not create code.
    bool incomplete = false;
    for (auto n : Nodes) {
        if (n->Level == -1) {
            incomplete = true;
        }
    }
    if (!incomplete) {
        //
        //  Create a copy of the nodes.
        activNodes = Nodes;
        //
        //  Create a multimap on the levels.
        std::multimap<int, MNode*> mn;

        for (std::vector<MNode*>::iterator an = activNodes.begin(); an != activNodes.end(); ++an) {
            mn.insert(std::pair<int, MNode*>((*an)->Level, *an));
        }
        //
        //  Now take the zero level nodes and get some nodes to dump.
        for (auto an : mn) {
            std::list<MElement*> path;

            if (an.second->Level == 0) {
                path = GetDumpNodes(an.second);
                for (auto p : path) {
                    src << "// " << p->name << std::endl;
                    if (p->type == eElementType::Edge) {
                        src << "// level: " << ((MEdge*)(p))->Level << std::endl;
                    } else {
                        src << "// level: " << ((MNode*)(p))->Level << std::endl;
                    }
                }
            } else {

            }

        }
#if 0
        for (auto an : mn) {
            src << "// " << an.second->name << std::endl;
            src << "// level: " << an.second->Level << std::endl;
            for (auto ie : an.second->Incoming) {

                if (ie->Level != an.second->Level) {
                    src << "//  Have different Levels. Node: " << an.second->Level << " Edge: " << ie->Level << std::endl;
                }
            }
        }
#endif

    } else {
        src << "// " << std::endl;
        src << "// Cannot create activity because of some nodes are not reachable." << std::endl;
    }
#endif
}
//
//  This method is called with start nodes.
std::list<std::shared_ptr<MElement>> CActivity::GetDumpNodes(std::shared_ptr<MNode> start) {
    std::list<std::shared_ptr<MElement>> retval;
    std::list<std::shared_ptr<MElement>> collection;

    //
    //
    if (start->Outgoing.size() > 1) {

        for (auto & e : start->Outgoing) {
            std::list<std::shared_ptr<MElement>> more;
            auto edge = std::dynamic_pointer_cast<MEdge>(*e);

            more = GetDumpNodes(edge);
            //
            //  If a path along one of the edges is incomplete we abort the dump.
            if (more.empty()) {
                collection.clear();
                break;
            }
            collection.insert(collection.end(), more.begin(), more.end());
        }
        retval = collection;
    }
    return retval;
}
//
//  This method is called on outgoing edges.
std::list<std::shared_ptr<MElement>> CActivity::GetDumpNodes(std::shared_ptr<MEdge> start) {
    std::list<std::shared_ptr<MElement>> retval;

    return retval;
}

void CActivity::FillLevel(std::shared_ptr<MNode> node, std::shared_ptr<MToken> level) {
    auto n = node;
    //
    //  Set the level and have a look on the number of Outgoings.
    n->SetLevel(level->Level);
    //
    //  We have a follow up node on the same level.
    while (n->Outgoing.size() == 1) {
        std::shared_ptr<MElement> t = std::dynamic_pointer_cast<MEdge>(*n->Outgoing[0])->Target;

        if (t->IsNodeBased()) {
            n = std::dynamic_pointer_cast<MNode>(t);
        } else if (t->type == eElementType::Pin) {
            n = std::dynamic_pointer_cast<MNode>(*t->parent);
        }
        n->SetLevel(level->Level);
    }
    //
    //  On more than one outgoing edge we go recursive.
    if (n->Outgoing.size() > 1) {
        //
        // go along the outgoing edges.
        for (auto & o : n->Outgoing) {
            auto ot = std::dynamic_pointer_cast<MEdge>(*o)->Target;

            if (ot->IsNodeBased()) {
                //
                //  Forks stay on their level.
                if (ot->type == eElementType::ForkNode) {
                    FillLevel(std::dynamic_pointer_cast<MNode>(*ot), level);
                } else {
                    FillLevel(std::dynamic_pointer_cast<MNode>(*ot), level);  ///   This is wrong. TODO
                }
            }
        }
    }
}
//
//  This is only for starting nodes, they have no incoming edges.
void CActivity::SendToken(std::shared_ptr<MNode> node, MToken& token) {

    for (auto & e : node->Outgoing) {
        token.path.push_back(e);
        SendToken(std::dynamic_pointer_cast<MEdge>(*e), token);
    }
}
//
//
void CActivity::SendToken(std::shared_ptr<MEdge> edge, MToken& token) {
    //
    //  Add the token to the edge.
    edge->AddToken(token);
    //
    //
    auto node = edge->GetTargetNode();
    //
    //
    if (node->HasAllToken()) {
        MToken pt = node->ProcessToken();
        SendToken(node, pt);
    }
}

void CActivity::Dump(std::ostream &src, CActionBlock &segment) {
#if 0
    CActionNode* an;
    for (el = segment.mBlock.begin(); el != segment.mBlock.end(); ++el) {
        an = static_cast<CActionNode*>((*el)->Source);

        DumpCxx(src, an);
    }
    if (!segment.mBlock.empty()) {
        CEdge* ed = segment.mBlock.back();

        an = static_cast<CActionNode*>(ed->Target);

        DumpCxx(src, an);
        ed->AddToken(MToken());
    }
#endif
}

void CActivity::DumpCxx(std::ostream &src, std::shared_ptr<MNode> node) {
    //
    //  And start the generation.
    auto start = node;
    while (start) {
        tToken ready;
        //
        // Create a path for a node.
        ready = start->CreatePath();
        //
        //  Remove the node from the list of active nodes
        //  If we can find it there.
        for (auto i = activNodes.begin(); i != activNodes.end(); ++i) {
            if (*i == start) {
                activNodes.erase(i);
                break;
            }
        }
        //
        //  If a segment gets completed we dump it.
        if (!ready.path.empty()) {
            //
            //  Check if all pathes end in the same node.
            auto target = ready.validate();
            //
            //  Validation delivers the target node that
            //  needs to be dumped.
            if (target) {
                switch (target->type) {
                case eElementType::Edge:
                    //DumpCxx(src, (CEdge*)(*ti));
                    break;
                case eElementType::MergeNode:
                    DumpCxx(src, std::dynamic_pointer_cast<CMergeNode>(target));
                    break;
                case eElementType::ActionNode:
                    DumpCxx(src, std::dynamic_pointer_cast<CActionNode>(target));
                    break;
                case eElementType::ForkNode:
                    break;
                default:
                    break;
                }
#if 0
                //
                //  If we have a single outgoing we may concatinate
                //  the pathes.
                if (target->OutgoingCount() > 1) {
                    start = 0;
                    //
                    //  Multiple outgoings.
                    for ( auto t : target->Outgoing) {
                        //
                        // Put the edge as token into the edge.
                        // This is to handle the token thingie all the same
                        std::list<MElement*> newpath;
                        newpath.push_back(t);

                        t->Token.path.push_back(newpath);

                        if (t->Target->IsNodeBased()) {
                            if (start == 0) {
                                start = (MNode*)(t->Target);
                            }
                        } else {
                            std::cerr << "Outgoing to not-node not supported\n";
                            start = 0;
                            break;
                        }
                    }
                } else {
                    if (!target->Outgoing.empty()) {
                        //
                        // Put the edge as token into the edge.
                        // This is to handle the token thingie all the same
                        std::list<MElement*> newpath;
                        newpath.push_back(target->Outgoing[0]);

                        target->Outgoing[0]->Token.path.push_back(newpath);

                        if (target->Outgoing[0]->Target->IsNodeBased()) {
                            start = (MNode*)(target->Outgoing[0]->Target);
                        } else {
                            std::cerr << "Outgoing to not-node not supported\n";
                            start = 0;
                        }
                    } else {
                        //
                        //  Check the outputpins
                        if (!target->OutputPins.empty()) {
                            //
                            // Create a token on the edge.
                            std::list<MElement*> newpath;

                            newpath.push_back(target->OutputPins[0]);

                            target->OutputPins[0]->Token.path.push_back(newpath);

                            if (target->OutputPins[0]->parent->IsNodeBased()) {
                                start = (MNode*)(target->OutputPins[0]->parent);
                            } else {
                                std::cerr << "Output-Pin to not-node not supported\n";
                                start = 0;
                            }
                        } else {
                            //
                            //  No outgoing at all. We are at end.
                            start = 0;
                        }
                    }
                    //
                    //  Set the token if we have new start.
                    if (start != 0) {
                    }
                }
#endif
            }
        } else {
            activNodes.push_back(start);
            start = 0;
        }
    }
}

void CActivity::DumpCxx(std::ostream &src, std::shared_ptr<MEdge> edge) {
    if (!edge->Guard.empty()) {
        if (edge->Guard == "else") {
            src << "} else {\n";
        } else {
            src << "if (" << edge->Guard << ") {\n";
        }
    }
}


void CActivity::DumpCxx(std::ostream &src, std::shared_ptr<CActionNode> node, int level) {
    if (node != 0) {
        switch(node->Kind) {
        case eActionKind::Opaque:
            break;
        case eActionKind::Create:
            DumpCxxCreate(src, node, level);
            break;
        case eActionKind::Destroy:
            break;
        case eActionKind::Read:
            break;
        case eActionKind::Write:
            DumpCxxWrite(src, node, level);
            break;
        case eActionKind::Insert:
            break;
        case eActionKind::Delete:
            break;
        case eActionKind::SendSignal:
            DumpCxxSendSignal(src, node, level);
            break;
        case eActionKind::AcceptSignal:
            break;
        case eActionKind::TriggerEvent:
            break;
        case eActionKind::AcceptEvent:
            break;
        case eActionKind::Structured:
            break;
        case eActionKind::TimerEvent:
            break;
        default:
            break;
        }

        if (!node->comment.empty()) {
            node->DumpComment(src, (level+1)*4, 90,  "//", "//", "");
        }
        if ((node->type != eElementType::ForkNode) && (!node->Body.empty())) {
            std::string filler;

            filler.assign((level+1)*4, ' ');
            std::string l;
            size_t nl_pos=0;
            std::istringstream iss;

            //
            //  Replace \n character combination by real newlines.
            do {
                nl_pos=node->Body.find("\\n", nl_pos);
                if (nl_pos != std::string::npos) {
                    node->Body.replace(nl_pos, 2, "\n");
                }
            } while (nl_pos != std::string::npos);

            //
            //  Remove trailing newlines from the comment.
            while ((node->Body.size()>0) && (node->Body.back()=='\n')) {
                //comment.pop_back();
                node->Body = node->Body.substr(0, comment.size()-1);
            }
            iss.str(node->Body);
            if (node->Body.size() > 0) {
                do {
                    std::getline(iss, l);
                    if (!l.empty()) {
                        src << filler << l << "\n";
                    }
                } while (iss.good());
            }
        }
        if (node->Outgoing.size() > 1) {

        }
    }
}


#if 0
void CActivity::DumpCxx(std::ostream &src, MNode* node, int level) {
    std::string filler;

    filler.assign((level+1)*4, ' ');

    while ((node != 0) && (node->HasAllToken())){
        node->Level++;
        src << "// ## " << node->name << std::endl;
        node->DumpComment(src, (level+1)*4);
#if 1
            switch (node->type) {
            case eDecisionNode:
                DumpCxx(src, (CDecisionNode*)node, level);
                break;
            case eMergeNode:
                DumpCxx(src, (CMergeNode*)node, level);
                break;
            case eElementType::ActionNode:
                DumpCxx(src, (CActionNode*)node, level);
                break;
            case eInitialNode:
                break;
            case eFinalNode:
                break;
            default:
                break;
            }

#endif

        //
        //  Go along the outgoing edges.
        std::vector<MEdge* >::iterator oi;
        MEdge*                         elseedge = 0;
        //
        //  If we have more than one outgoing edges and pins.
        //  We go recursive.
        if (node->Outgoing.size() > 1) {
            //
            //  Check the outgoing edges
            for (std::vector<MEdge*>::iterator x = node->Outgoing.begin(); x != node->Outgoing.end(); ++x) {
                //(*x)->Token = true;
                if ((*x)->Target->type == eElementType::Pin) {
                    ((MPin*)((*x)->Target))->Token = (*x)->GetSource();
                    DumpCxx(src, (MNode*)(((*x)->Target->parent)), level+1);
                } else {
                    DumpCxx(src, (MNode*)((*x)->Target), level+1);
                }
            }
            break;
#if 0
            int guardcount = 0;
            //
            //  Go through all outgoing exept else
            for (oi = node->Outgoing.begin(); oi != node->Outgoing.end(); ++oi) {
                if (((*oi)->Guard != "else") && (!(*oi)->Guard.empty())) {
                    if (guardcount > 0) {
                        src << filler << "} else ";
                    }
                    src << filler << "if (" <<  (*oi)->Guard << " ) {\n";
                    guardcount++;
                    DumpCxx(src, (MNode*)((*oi)->Target), level+1);
                } else {
                    elseedge = *oi;
                }
            }
            if (elseedge != 0) {
                if (elseedge->Target->type != eMergeNode) {
                    src <<filler << "} else {\n";
                    DumpCxx(src, (MNode*)((elseedge)->Target), level+1);
                } else {
                    DumpCxx(src, (MNode*)((elseedge)->Target), level);
                }
            }
#endif
        } else if (node->Outgoing.size() == 1) {
            if (node->Incoming.size() > 1) {
                level--;
            }
            //node->Outgoing[0]->Token = true;
            if (node->Outgoing[0]->Target->type == eElementType::Pin) {
                ((MPin*)(node->Outgoing[0]->Target))->Token = node->Outgoing[0]->GetSource();
                node =  (MNode*)((node->Outgoing[0]->Target->parent));
            } else {
                node = (MNode*)(node->Outgoing[0]->Target);
            }
        } else {
            node = 0;
        }
    }
}
#endif
void CActivity::DumpCxx(std::ostream &src, std::shared_ptr<CDecisionNode> node, int level) {
    if (node != 0) {
    }
}

void CActivity::DumpCxxSendSignal(std::ostream &src, std::shared_ptr<CActionNode> node, int level) {
#if 0
    std::string filler;

    filler.assign((level+1)*4, ' ');

    if (node) {
        if (node->HasStereotype("SimMessage")) {
            auto mi = CMessageClass::MessageByName.find(node->name);

            if (mi != CMessageClass::MessageByName.end()) {
                auto mc = std::dynamic_pointer_cast<CMessageClass>(mi->second);

                if (mc != 0) {
                    src << filler << mc->name << "* __" << mc->lower_basename << " = new " << mc->name << ";" << std::endl << std::endl;

                }
            }
        }
    }
#endif
}

void CActivity::DumpCxxCreate(std::ostream &src, std::shared_ptr<CActionNode> node, int level) {
    if (node != 0) {
        std::string filler;

        filler.assign((level+1)*4, ' ');
        //
        //  We need an output pin. But only one.

        if (node->Target) {
            src << filler;
            auto pin = std::dynamic_pointer_cast<CPin>(*node->OutputPins[0]);

            if (pin->Classifier) {
                src << pin->Classifier->name;
            } else {
                src << pin->ClassifierName;
            }
            src << "* " << pin->name << " = new " << node->Target->name << ";" << std::endl;
        } else {

        }
    }
}

void CActivity::DumpCxxWrite(std::ostream &src, std::shared_ptr<CActionNode> node, int level) {
    if (node != 0) {
        std::string filler;
        std::string prefix;
        std::string assignment;

        filler.assign((level+1)*4, ' ');
        if (node->InputPins.size() == 1) {
            auto pin = std::dynamic_pointer_cast<CPin>(*node->InputPins[0]);
            //
            //  Search the incoming edge for the input pin.
            auto ie = FindEdge(pin);
            if (ie) {
                src << filler <<  pin->name << " = " << ie->Source->name << ";\n";
            }
        } else {
            auto pin0 = std::dynamic_pointer_cast<CPin>(*node->InputPins[0]);
            auto pin1 = std::dynamic_pointer_cast<CPin>(*node->InputPins[1]);
            //
            // Find the pin that is the structure. It should have type assigned where the other
            // pin name is an attribute in
            std::shared_ptr<MClass> p0;
            std::shared_ptr<MPin>   structure;
            std::shared_ptr<MPin>   property;

            if (pin0->Classifier) {
                if (pin0->Classifier->IsClassBased()) {

                    auto p0 = std::dynamic_pointer_cast<MClass>(*pin0->Classifier);
                    structure = std::dynamic_pointer_cast<MPin>(pin0);
                    property  = std::dynamic_pointer_cast<MPin>(pin1);
                }
            } else if (pin1->Classifier) {
                if (pin1->Classifier->IsClassBased()) {

                    p0 = std::dynamic_pointer_cast<MClass>(*pin1->Classifier);
                    structure = std::dynamic_pointer_cast<MPin>(pin1);
                    property  = std::dynamic_pointer_cast<MPin>(pin0);
                }
            }

            for (auto & a : p0->Attribute) {
                if (a->name == property->name) {
                    src << filler << structure->name;

                    auto se = FindEdge(structure);
                    if (se) {
                        auto s = se->GetSource();
                        if (s->type == eElementType::Pin) {
                            auto parentnode = std::dynamic_pointer_cast<CActionNode>(*s->parent);
                            if (parentnode->type == eElementType::ActionNode) {
                                if (parentnode->Kind == eActionKind::Create) {
                                    src << "->";
                                }
                            } else {
                                src << ".";
                            }
                        } else {
                            src << ".";
                        }
                    }
                    auto ie = FindEdge(property);
                    if (ie) {
                        src << property->name << " = " << ie->Source->name << ";\n";
                    }
                    break;
                }
            }
        }
    }
}

void CActivity::DumpCxx(std::ostream &src, std::shared_ptr<CMergeNode> node, int level) {
    std::string filler;

    filler.assign((level+1)*4, ' ');

    if (node != 0) {
        src << filler << "}\n";
    }
}

std::shared_ptr<MEdge> CActivity::FindEdge(std::shared_ptr<MPin> pin) {
    std::shared_ptr<MEdge> retval;

    for (auto & x : Edges) {
        auto edge = std::dynamic_pointer_cast<MEdge>(*x);
        if ((edge->Source == pin) || (edge->Target == pin)) {
            retval = edge;
            break;
        }
    }
    return retval;
}
