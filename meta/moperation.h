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
#ifndef MOPERATION_H
#define MOPERATION_H

#include <vector>
#include "melement.h"

class MParameter;
class MActivity;

class MOperation : public MElement
{
public:
    MOperation() = default;
    MOperation(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr) : MElement(aId, aParent) {type=eElementType::Operation;;}
    virtual ~MOperation() = default;
    static std::shared_ptr<MOperation> construct(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
public:
    static std::map<std::string, std::shared_ptr<MOperation>> Instances;
    std::vector<MElementRef>                  Parameter;
    bool                                      isStatic;
    bool                                      isAbstract;
    bool                                      isQuery        = false;
    bool                                      isPure         = false;
    bool                                      hasConstReturn = false;
    MElementRef                               mException;

    std::string                               Specification;
    MElementRef                               Activity;
};

#endif // MOPERATION_H
