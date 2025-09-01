//
// Copyright 2024 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#ifndef CMODULECLASS_H
#define CMODULECLASS_H

#include <iostream>
#include <list>
#include "ccxxclass.h"

class CClassBase;
class CAttribute;
class COperation;
class CAssociationEnd;
class MModel;
class MElement;
class CCollaboration;

class CModuleClass : public CCxxClass
{
public:
    CModuleClass() = default;
    CModuleClass(const std::string&aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {if (!mIsInterface) {has_src=".cpp";} has_hdr=".h";type = eElementType::ModuleClass;mClassifierType = "module";}
    ~CModuleClass() override = default;
    //
    //  Virtuals from MElement
    virtual void SetFromTags(const std::string& name, const std::string&value);
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);

    void DumpClassDecl(std::ostream& file, int indent);

#if 0
    std::string GetBaseClasses(void);

    void CollectNeededModelHeader(std::shared_ptr<MElement> e, HeaderList& aHeaderList) ;

    void CollectFromBase(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromAttribute(const TypeNode& aNode, HeaderList& aHeaderList, bool aIsTemplateReference = false);
    void CollectFromParameter(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromSharedAssoc(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromCompositeAssoc(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromTemplateBinding(std::shared_ptr<CClassBase> aClass, HeaderList& aHeaderList);

//    void CollectExtraHeader(std::shared_ptr<MElement> e);
    void CollectSelfContainedHeader();

    void CollectForwards() ;

    void DumpPackageOperationDecl(std::ostream& hdr);
    void DumpOperationDecl(std::ostream& hdr, int indent);
    void DumpInlineOperations(std::ostream& hdr);
    //void DumpQtConnectorDecl(std::ostream& hdr);
    void DumpAttributeDecl(std::ostream& hdr, int indent);
    void DumpPackageAttributeDecl(std::ostream& hdr);

    void DumpStaticAttributeDefinition(std::ostream& src);
    void DumpPackageAttributeDefinition(std::ostream&src);
    void DumpPackageOperationDefinition(std::ostream &src, bool aStatic) ;
    void DumpOperationDefinition(std::ostream& src);
    void DumpAliases(std::ostream& file, int a_indent, eVisibility a_vis = vPackage);
    //void DumpQtConnectorDefinition(std::ostream& src);

    void DumpCollaboration(std::ostream& src, std::shared_ptr<CClassBase> aCollab);
    static bool IsCardinalType(const std::string&);

    bool NeedSimIfc(void);
    //bool HaveAnyContent(void);
    bool HasAnyVirtuals(void);
    //bool HasCTOR(void);
    bool HasDTOR(void);
    bool hasOneOfFive(void);
    uint8_t getOneOfFive(void) ;

    std::shared_ptr<COperation> findBySignature(const std::string& aSignature);

    void DumpMessageForwards(std::ostream& hdr);
    void DumpMessageProcessingProto(std::ostream& hdr);
    //void DumpMessageIncludes(std::ostream& src, std::set<std::string>& aDoneList, std::set<std::string> &aIncludesDone);
    //void DumpQtConnectorIncludes(std::ostream& src, std::set<std::string>& aDoneList, std::set<std::string> &aIncludesDone);
    void DumpMessageProcessingFunctions(std::shared_ptr<CCxxClass> where, std::set<std::string>& aDoneList);
    //
    //  Serializer specific
    void DumpSerializerDecl(std::ostream& hdr, int indent) const;
    void DumpSerializerDefinition(std::ostream& src);

    void DumpForwards(std::ostream& file);

public:
private:
    void CollectForwards(std::shared_ptr<CClassBase> aClass);
    void CollectForwardRefs(std::shared_ptr<CClassBase> aClass);
    void FillInCollection(std::shared_ptr<MElement> e);
    void FillInCollection(const std::string& aTextType);
    std::string mapVisibility(eVisibility a_vis);
protected:
    std::string                          mClassifierType = "class";
    bool                                 mSerialize = false;
    eByteOrder                           mByteOrder = eByteOrder::Host;
    std::ofstream                        mSysHeader;
    std::list<std::shared_ptr<MElement>> mSelfContainedHeaders;
    std::list<std::shared_ptr<MElement>> mSelfContainedExtras;
    std::list<std::shared_ptr<MElement>> mSelfContainedQt;
#endif
};


#endif // CMODULECLASS_H
