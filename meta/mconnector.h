//
// Copyright 2020 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#ifndef MCONNECTOR_H
#define MCONNECTOR_H

#include <vector>
#include "melement.h"
#include "massociation.h"

class MConnector : public MElement
{
public:
    MConnector() = default;
    MConnector(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr) : MElement(aId, aParent) {type=eElementType::Connector;}
    static std::shared_ptr<MConnector> construct(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);

    void AddEnd(std::shared_ptr<MAssociationEnd> e) {ends.emplace_back(e);}
    std::shared_ptr<MAssociationEnd> ThisEnd(const std::string& ref);
    std::shared_ptr<MAssociationEnd> OtherEnd(const std::string& ref);
    std::shared_ptr<MAssociationEnd> OtherEnd(std::shared_ptr<MAssociationEnd> thisend);
public:
    static std::map<std::string, std::shared_ptr<MConnector>> Instances;
    std::vector<MElementRef>                                  ends;
};

#endif // MCONNECTOR_H
