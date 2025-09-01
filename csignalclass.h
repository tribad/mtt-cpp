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
#ifndef CSIGNALCLASS_H
#define CSIGNALCLASS_H

#include <string>

#include "json.h"
#include "ccxxclass.h"

class CAssociationEnd;

enum class SignalEncoding {
    none,     //  No code is will be generated.
    json,     //  JSON serialization
    protobuf, //  protobuf serialization
    tlv       //  Binary TLV format
};

class CSignalClass : public CCxxClass
{
public:
    CSignalClass() = default;
    CSignalClass(const std::string& aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {has_hdr = ".h";type = eElementType::SimSignalClass;basename=name;}
    virtual void SetFromTags(const std::string& name, const std::string&value);
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);
    void DumpJSON(std::shared_ptr<MModel> aModel);
    void DumpProtobuf(std::shared_ptr<MModel> aModel);
    void DumpTLV(std::shared_ptr<MModel> aModel);
    //void CollectNeededModelHeader(std::shared_ptr<MElement> e) ;
    void DumpJSONIncoming(std::ofstream& ifc);
    void DumpJSONIncomingDeclaration(std::ofstream& ifc);
    void DumpJSONIncomingValues(std::ofstream& ifc, std::shared_ptr<CClassBase> aClass, std::string prefix, std::string jsonvar, int space);
    void DumpJSONIncomingArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, int space);
    void DumpJSONIncomingArray(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, int space);
    void DumpJSONIncomingStruct(std::ofstream& ifc, std::shared_ptr<MElement> aStruct, std::string prefix, int space);

    void DumpJSONOutgoing(std::ofstream& ifc);
    void DumpJSONOutgoingDeclaration(std::ofstream& ifc);
    void DumpJSONStruct(std::ofstream& ifc, std::shared_ptr<MElement> e, std::string sname, std::string prefix, bool first=false, int space=0);
    void DumpJSONArray(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first=false, int space=0);
    void DumpJSONValue(std::ofstream& ifc, std::shared_ptr<CAttribute> a, std::string prefix, bool first=false, int space=0);
    void DumpJSONArray(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first=false, int space=0);
    void DumpJSONValue(std::ofstream& ifc, std::shared_ptr<CAssociationEnd> a, std::string prefix, bool first=false, int space=0);

    void toJSONBuddy(std::ofstream& ifc);
    void fromJSONBuddy(std::ofstream& ifc);


    void DumpProtobufAttributes(std::ofstream& a_pbfile, int a_indentation);
public:
    SignalEncoding m_encoding = SignalEncoding::none;
    std::string    direction;
    std::string    msgtype;
    std::string    basename;
    std::string    lower_name;
    std::string    upper_name;
    std::string    lower_basename;
    std::string    upper_basename;
    int            m_attribute_index = 1;
};

#endif // CSIGNALCLASS_H
