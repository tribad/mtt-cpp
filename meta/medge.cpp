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
#include "cedge.h"

#include "mnode.h"

std::map<std::string, std::shared_ptr<MEdge>> MEdge::Instances;

MEdge::MEdge()
{
    type = eElementType::Edge;
}

MEdge::MEdge(const std::string&aId, std::shared_ptr<MElement> aParent) : MElement(aId, aParent) {
    type   = eElementType::Edge;
    Level  = 0;
    MEdge::Instances.insert(std::pair<std::string, std::shared_ptr<MEdge>>(aId, sharedthis<MEdge>()));
}

std::shared_ptr<MEdge> MEdge::construct(const std::string&aId,std::shared_ptr<MElement> aParent)
{
    auto retval = new CEdge(aId, aParent);

    return retval->sharedthis<MEdge>();
}


std::shared_ptr<MElement> MEdge::GetSource() {
    std::shared_ptr<MElement> retval;

    if (Source->type == eElementType::ForkNode) {
        retval = std::dynamic_pointer_cast<MEdge>(*(std::dynamic_pointer_cast<MNode>(*Source))->Incoming[0])->GetSource();
    } else {
        retval = *Source;
    }
    return retval;
}

std::shared_ptr<MNode> MEdge::GetTargetNode() {
    std::shared_ptr<MNode>    node;
    std::shared_ptr<MElement> target = *Target;
    //
    //  If the edge is directly connected to a node
    if ((target) && target->IsNodeBased()) {
        node = std::dynamic_pointer_cast<MNode>(target);
    } else if ((target) && target->type == eElementType::Pin) {
        node = std::dynamic_pointer_cast<MNode>(*target->parent);
    }
    return node;
}

void MEdge::AddToken(const MToken& token) {
    Token.push_back(token);
    Level = token.Level;
}
