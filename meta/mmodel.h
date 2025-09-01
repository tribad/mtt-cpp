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
#ifndef MMODEL_H
#define MMODEL_H

#include <list>
#include <map>
#include <utility>
#include <string>

#include "mstereotype.h"
#include "mclass.h"
#include "mpackage.h"
#include "massociation.h"
#include "massociationend.h"
#include "mattribute.h"

class MModel
{
public:
    MModel() = default;
    virtual ~MModel() = default;
    static std::shared_ptr<MModel> construct();
    void Add(std::shared_ptr<MStereotype> s);
    void Add(std::shared_ptr<MPackage> p);
    void Add(std::shared_ptr<MAssociation> a);
    void Add(std::shared_ptr<MAssociationEnd> e);
    void Add(std::shared_ptr<MElement> e);
    void AddTag(const std::string &name, const std::string& value);
    std::shared_ptr<MStereotype> StereotypeById(const std::string& id);
    std::shared_ptr<MStereotype> StereotypeByName(const std::string& name, const std::string& etype = "");
    void Complete(void);
protected:
    std::list<MElementRef>             Stereotype;
    std::list<MElementRef>             Packages;
    std::list<MElementRef>             Associations;
    std::list<MElementRef>             AssociationEnds;
    std::list<MElementRef>             mDependency;
    std::list<MElementRef>             mGeneralization;
    std::map<std::string, std::string> TaggedValues;
    std::list<MElementRef>             mUnowned;
};

#endif // MMODEL_H
