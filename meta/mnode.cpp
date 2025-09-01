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
#include "melement.h"
#include "medge.h"
#include "mnode.h"
#include "cnode.h"

#include "mparameter.h"
#include "mpin.h"

std::map<std::string, std::shared_ptr<MNode>> MNode::Instances;

MNode::MNode()
{
    type     = eElementType::Node;
    Level    = -1;
    prepDone = false;
}

MNode::MNode(const std::string&aId, std::shared_ptr<MElement> aParent) : MElement(aId, aParent) {
    type     = eElementType::Node;
    Level    = -1;
    prepDone = false;
    MNode::Instances.insert(std::pair<std::string, std::shared_ptr<MNode>>(aId, sharedthis<MNode>()));
}

int MNode::SetLevel(int level) {
    if (level > Level) {
        Level = level;
    }
    return Level;
}

std::shared_ptr<MNode> MNode::construct(const std::string&aId, std::shared_ptr<MElement> aParent)
{
    auto retval = new CNode(aId, aParent);

    return retval->sharedthis<MNode>();
}
//
//  This gives even true if there are no incoming edges.
bool MNode::HasAllToken() {
    bool retval = true;

    for (auto & x : Incoming) {
        if (std::dynamic_pointer_cast<MEdge>(*x)->Token.empty()) {
            retval = false;
            break;
        }
    }
    return retval;
}

tToken MNode::CreatePath() {
    tToken path;
    return path;
}

tToken* MNode::Process(tToken* token) {
    tToken* result = 0;
    //
    //  If we have one or none incoming edge we are an initial node.
    //  and not waiting on other tokens.
    if (HasAllToken()) {

        //
        //  If we have more than one incoming edge, we must combine the
        //  the tokens
        if (Incoming.size()>1) {
//            token=MergeToken();
        }
    }
    return result;
}

MToken MNode::ProcessToken() {
    MToken retval;
    //
    //  We check the token on the first incoming edge for the type of token we expect.
    if (!(std::dynamic_pointer_cast<MEdge>(*Incoming[0])->Token.empty())) {
        auto ti = std::dynamic_pointer_cast<MEdge>(*Incoming[0])->Token.begin();

        if (ti != std::dynamic_pointer_cast<MEdge>(*Incoming[0])->Token.end()) {
            switch (ti->tType) {
            case MTokenType::Empty:
                break;
            case MTokenType::Level:
                retval = ProcessLevelToken();
                break;
            default:
                break;
            }
        }
    }
    return retval;

}

MToken MNode::ProcessLevelToken() {
    MToken retval;

    for (auto in : Incoming) {
        for (auto token : std::dynamic_pointer_cast<MEdge>(*in)->Token) {
            if (token.Level > Level) {
                Level = token.Level;
            }
        }
        std::dynamic_pointer_cast<MEdge>(*in)->Token.clear();
    }
    if (Outgoing.size() > 1) {
        retval = MToken(Level+1);
    } else {
        retval = MToken(Level);
    }
    return retval;
}
