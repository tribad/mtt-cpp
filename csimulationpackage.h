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
#ifndef CSIMULATIONPACKAGE_H
#define CSIMULATIONPACKAGE_H

#include <string>
#include <list>
#include <set>

class MDependency;
class MClass;
class CCxxClass;
class CClassBase;

class CSimulationPackage : public CPackageBase
{
public:
    CSimulationPackage() = default;
    CSimulationPackage(const std::string& aId, std::shared_ptr<MElement> e) : CPackageBase(aId, e) {type = eElementType::SimulationPackage;}
    ~CSimulationPackage() override = default;
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);

    void DumpExtraIncludes(std::ofstream& ifc, std::set<std::string>& aDoneIncludes);
    void DumpExtraIncludes(std::ofstream& ifc, std::shared_ptr<CClassBase> aClass, std::set<std::string>& aDoneIncludes);
    void DumpExtraIncludes(std::ofstream &ifc, std::string aHeaders, std::set<std::string>& aDoneIncludes);
private:
    std::string        SimulationName;
    std::ofstream      delreq;
    std::ofstream      delreply;
    std::ofstream      delindication;
    std::ofstream      delconfirm;
    std::ofstream      ids_h;
    std::ofstream      ids_sql;
    std::ofstream      crmaps_sql;
    std::ofstream      ids_php;
    std::ofstream      ifc;
    std::ofstream      generated;
    std::list<std::shared_ptr<MClass>> content;
    std::shared_ptr<CCxxClass>         mIfcClass;   //  this is the base for all pathes to be build.
};

#endif // CSIMULATIONPACKAGE_H
