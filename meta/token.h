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
#ifndef TOKEN_H
#define TOKEN_H
#include <list>

#include "melement.h"

class MNode;

enum class MTokenType {
    Level,
    Path,
    Empty
};

class MToken : public MElement {
public:
    MToken() {type = eElementType::Token; Level = 0; tType = MTokenType::Empty;}
    MToken(int level) {type = eElementType::Token; Level = level; tType = MTokenType::Level;}
    MToken(int level, std::shared_ptr<MElement> start) {type = eElementType::Token; Level = level; tType = MTokenType::Level;path.emplace_back(start);}
    virtual ~MToken() = default;
    virtual std::string FQN(void) const {return std::string("");}
    virtual void Prepare(void) {}
    virtual void Dump(std::shared_ptr<MModel>) {}
public:
    MTokenType             tType;
    int                    Level;
    std::list<MElementRef> path;
};

//
//  We need a list pathes that may end somewhere.
//  For operations with different parameter input pathes this
//  makes it possible to create them in the right order.

class tToken {
public:
    std::shared_ptr<MNode> validate();
    void clear() {path.clear();}
    std::list < std::list<MElementRef> > path;
};

#endif // TOKEN_H
