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
#ifndef MASSOCIATION_H
#define MASSOCIATION_H

#include <vector>

#include "melement.h"
#include "massociationend.h"

class MAssociation : public MElement
{
public:
    MAssociation();
    MAssociation(const std::string&aId, std::shared_ptr<MElement> aParent=0);
    static std::shared_ptr<MAssociation> construct(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
    void AddEnd(std::shared_ptr<MAssociationEnd> e) {ends.emplace_back(e);}
    std::shared_ptr<MAssociationEnd> ThisEnd(const std::string& ref);
    std::shared_ptr<MAssociationEnd> OtherEnd(const std::string& ref);
    std::shared_ptr<MAssociationEnd> OtherEnd(std::shared_ptr<MAssociationEnd> thisend);
public:
    static std::map<std::string, std::shared_ptr<MAssociation>> Instances;
    std::vector<MElementRef>                                    ends;
};

#endif // MASSOCIATION_H
