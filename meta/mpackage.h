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
#ifndef MPACKAGE_H
#define MPACKAGE_H

#include <vector>
#include "namespace.h"
#include "melement.h"
#include "mclass.h"
#include "mdependency.h"
#include "mstereotype.h"
#include "mobject.h"

class MPackage : public MElement
{
public:
    MPackage(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr) : MElement(aId, aParent) {
        type=eElementType::Package;
        MPackage::Instances.insert(std::pair<std::string, std::shared_ptr<MPackage>>(aId, sharedthis<MPackage>()));
    }
    virtual ~MPackage() = default;
    static std::shared_ptr<MPackage> construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype = nullptr, std::shared_ptr<MElement> aParent = nullptr);
    static std::shared_ptr<MPackage> construct(const std::string&aId, std::string aPackageType, std::shared_ptr<MElement> aParent = nullptr);
    void Add(std::shared_ptr<MElement> aElement) override;
    void Delete(std::shared_ptr<MElement> aElement) override;
    void Add(std::shared_ptr<MClass> c) {
        Classes.emplace_back(c);
        owned.emplace_back(c);
    }
    void Add(std::shared_ptr<MObject> o) {
        Objects.emplace_back(o);
        owned.emplace_back(o);
    }
    void Add(std::shared_ptr<MPackage> p) {
        Packages.emplace_back(p);
        owned.emplace_back(p);
    }
    std::string GetDefaultStereotype();
public:
    static std::map<std::string, std::shared_ptr<MPackage>> Instances;
    std::vector<MElementRef>                                Classes;
    std::vector<MElementRef>                                Packages;
    std::vector<MElementRef>                                Dependency;
    std::vector<MElementRef>                                Objects;
    NameSpace                                               mNameSpace;
};

#endif // MPACKAGE_H
