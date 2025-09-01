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
#ifndef CJSONMESSAGE_H
#define CJSONMESSAGE_H

#include "mclass.h"

class CAttribute;
class CAssociationEnd;

class CJSONMessage : public MJSONMessage
{
public:
    CJSONMessage() = default;
    CJSONMessage(const std::string& aId, std::shared_ptr<MElement> e) : MJSONMessage(aId, e) {Class = 0;};
    virtual ~CJSONMessage() = default;
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);
    //
    //  Others
    void FillClass(void);
    std::shared_ptr<MClass> GetClass();
    void DumpIncoming(std::ofstream& ifc);
    void DumpJSONIncomingArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, int space);
    void DumpOutgoing(std::ofstream& ifc);
    void DumpStruct(std::ofstream& ifc, std::shared_ptr<MElement> e, std::string sname, std::string prefix, bool first=false, int space=0);
    void DumpArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first=false, int space=0);
    void DumpValue(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first=false, int space=0);
    void DumpArray(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first=false, int space=0);
    void DumpValue(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first=false, int space=0);
public:
    std::shared_ptr<MClass> Class;
};

#endif // CJSONMESSAGE_H
