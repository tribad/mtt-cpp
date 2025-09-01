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

#include "mmergenode.h"
#include "cmergenode.h"

std::map<std::string, std::shared_ptr<MMergeNode>> MMergeNode::Instances;

MMergeNode::MMergeNode()
{

}

MMergeNode::MMergeNode(const std::string&aId, std::shared_ptr<MElement> aParent) : MNode(aId, aParent) {
    type = eElementType::MergeNode;
    MMergeNode::Instances.insert(std::pair<std::string, std::shared_ptr<MMergeNode>>(aId, sharedthis<MMergeNode>()));
}

std::shared_ptr<MMergeNode> MMergeNode::construct(const std::string&aId, std::shared_ptr<MElement> aParent)
{
    auto retval = new CMergeNode(aId, aParent);

    return retval->sharedthis<MMergeNode>();
}

int MMergeNode::SetLevel(int level) {
    if (Level == -1) {
        Level = level;
    } else {
        if ((level-1) < Level) {
            Level = level -1;
        }
    }
    return Level;
}

MToken MMergeNode::ProcessLevelToken() {
    MToken retval;

    for (auto & in : Incoming) {
        for (auto & token : std::dynamic_pointer_cast<MEdge>(*in)   ->Token) {
            if (Level == -1) {
                Level = token.Level;
            } else {
                if ((token.Level-1) < Level) {
                    Level = token.Level -1;
                }
            }
        }
        std::dynamic_pointer_cast<MEdge>(*in)->Token.clear();
    }
    //
    //  Be sure that we do not create a Level-Token with level -1.
    if (Level > 0) {
        retval = MToken(Level-1);
    } else {
        retval = MToken(0);
    }

    return retval;
}
