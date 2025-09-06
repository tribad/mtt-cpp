//
// Copyright 2016 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#ifndef MOBJECT_H
#define MOBJECT_H

#include "melement.h"
#include "mlink.h"
#include "mattribute.h"

class MObject : public MElement
{
public:
    MObject();
    MObject(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
    static std::shared_ptr<MObject> construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent = nullptr);
    void AddAttribute(std::shared_ptr<MAttribute> a) {
        Slots.emplace_back(a);
        owned.emplace_back(a);
    };
    void AddLink(std::shared_ptr<MLink> l) {
        Links.emplace_back(l);
        owned.emplace_back(l);
    };
public:
    static std::map<std::string, std::shared_ptr<MObject>> Instances;
    std::vector<MElementRef> Links;
    std::vector<MElementRef> Slots;  //  Attributes.
};

#endif // MOBJECT_H
