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
#ifndef CMESSAGECLASS_H
#define CMESSAGECLASS_H

#include "cclassbase.h"
#include "ccxxclass.h"

class CMessageClass : public CCxxClass
{
public:
    CMessageClass();
    CMessageClass(const std::string& aId, std::shared_ptr<MElement> e);
    virtual void SetFromTags(const std::string& name, const std::string&value);
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);
    //void CollectNeededModelHeader(std::shared_ptr<MElement> e) ;

    void DumpJSONIncoming(std::ofstream& ifc);
    void DumpJSONIncomingDeclaration(std::ofstream& ifc);
    void DumpJSONIncomingArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, int space);
    void DumpJSONIncomingArray(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, int space);
    void DumpJSONOutgoing(std::ofstream& ifc);
    void DumpJSONOutgoingDeclaration(std::ofstream& ifc);
    void DumpJSONStruct(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<MElement> e, const std::string& sname, const std::string& prefix, bool first=false, int space=0);
    void DumpJSONArray(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAttribute> a, const std::string& prefix, bool first=false, int space=0);
    void DumpJSONValue(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAttribute> a, const std::string& prefix, bool first=false, int space=0);
    void DumpJSONArray(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAssociationEnd> a, const std::string& prefix, bool first=false, int space=0);
    void DumpJSONValue(std::ofstream& ifc, const std::string& a_stream , const std::shared_ptr<CAssociationEnd> a, const std::string& prefix, bool first=false, int space=0);

    void toJSONBuddy(std::ofstream& ifc, const std::string& a_stream);

    void DumpFromJSONArray(std::ofstream& ifc, const std::string& a_stream, const std::shared_ptr<CAttribute> a, const std::string& prefix, bool first=false, int space = 0 );
    void DumpFromJSONArray(std::ofstream& ifc, const std::string& a_stream, const std::shared_ptr<CAssociationEnd> a, const std::string& prefix, bool first=false, int space = 0 );

    void fromJSONBuddy(std::ofstream& ifc);
public:
    std::string direction;
    std::string msgtype;
    std::string basename;
    std::string lower_name;
    std::string upper_name;
    std::string lower_basename;
    std::string upper_basename;
};

#endif // CMESSAGECLASS_H
