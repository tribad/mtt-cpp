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
#ifndef MEDGE_H
#define MEDGE_H

#include <map>
#include <string>
#include <list>

#include "token.h"

class MElement;

class MEdge : public MElement
{
public:
    MEdge();
    MEdge(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
    static std::shared_ptr<MEdge> construct(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
    std::shared_ptr<MElement> GetSource();
    std::shared_ptr<MNode> GetTargetNode();
    void AddToken(const MToken& token);
public:
    static std::map<std::string, std::shared_ptr<MEdge>> Instances;
    MElementRef                          Source;   // This can be a MNode or a MParameter.
    MElementRef                          Target;   // This can be a MNode or a MParameter.
    std::string                          Guard;
    std::string                          Weight;
    int                                  Level;
    std::list<MToken>                    Token;
};

#endif // MEDGE_H
