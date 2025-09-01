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
#ifndef MNODE_H
#define MNODE_H

#include <vector>
#include <string>

#include "melement.h"
#include "medge.h"

class MEdge;
class MPin;

class MNode : public MElement
{
public:
    MNode();
    MNode(const std::string&aId, std::shared_ptr<MElement> aParent);
    static std::shared_ptr<MNode> construct(const std::string&aId, std::shared_ptr<MElement> aParent);

    tToken* Process(tToken* token);
    virtual  tToken CreatePath();
    bool HasAllToken();
    int OutgoingSum() { return (int)(Outgoing.size()+OutputPins.size());}
    int IncomingSum() { return (int)(Incoming.size()+InputPins.size());}
    virtual int SetLevel(int level) ;
    virtual MToken ProcessToken();
    virtual MToken ProcessLevelToken();
public:
    static std::map<std::string, std::shared_ptr<MNode>> Instances;
    int                                  Level;
    bool                                 prepDone;
    MElementRef                          Target;      //  Some node.
    std::vector<MElementRef>             Incoming;    //  These are edges.
    std::vector<MElementRef>             Outgoing;    //  These are edges.
    std::vector<MElementRef>             InputPins;   //  These pins carry incoming tokens.
    std::vector<MElementRef>             OutputPins;  //  Outgoing pathes.
    tToken                               Token;       //  These are tokens from Edges ending on the node.
    bool                                 CheckPoint;  //  Only on CheckPoints code can be generated.
};

#endif // MNODE_H
