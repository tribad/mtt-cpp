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
#ifndef CCCLASS_H
#define CCCLASS_H

#include <set>

#include "melement.h"
#include "cclassbase.h"

class CCClass : public CClassBase
{
public:
    CCClass() = default;
    CCClass(const std::string &aId, std::shared_ptr<MElement> e) : CClassBase(aId, e) {has_src=".cpp"; has_hdr=".h";type = eElementType::CClass;}
    virtual ~CCClass() = default;
    //
    //  Virtuals from MElement
    virtual void SetFromTags(const std::string& name, const std::string&value);
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);

    void CollectNeededModelHeader(std::shared_ptr<MElement> e, HeaderList& aHeaderList) ;

    void CollectFromBase(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromAttribute(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromParameter(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromSharedAssoc(const TypeNode& aNode, HeaderList& aHeaderList);
    void CollectFromCompositeAssoc(const TypeNode& aNode, HeaderList& aHeaderList);

    void CollectExtraHeader(std::shared_ptr<MElement> e);

    void DumpOperationDecl(std::ostream& hdr);
    void DumpAttributeDecl(std::ostream& hdr);

    void DumpAttributeDefinition(std::ostream& src, eVisibility vis=vPublic);
    void DumpOperationDefinition(std::ostream& src);

};

#endif // CCCLASS_H
