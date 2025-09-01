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
#ifndef CPHPCLASS_H
#define CPHPCLASS_H

#include "cclassbase.h"
#include "moperation.h"
#include "coperation.h"

class CPHPClass : public CClassBase
{
public:
    CPHPClass() = default;
    CPHPClass(const std::string& aId, std::shared_ptr<MElement> e) : CClassBase(aId, e) {Strict = false; ModelIncludes = true; has_src=".php";type = eElementType::PHPClass;}
    virtual ~CPHPClass() = default;
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void SetFromTags(const std::string& name, const std::string&value);
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);

    void CollectNeededModelHeader(std::shared_ptr<MElement> e) ;

    void DumpAttributeDecl(std::ostream& hdr);
    void DumpOperations(std::ostream& outfile);
    void DumpPackageAttributes(std::ostream& outfile);
    void DumpPackageOperations(std::ostream& outfile);
    void DumpOperationBuddy(std::ostream& outfile, std::shared_ptr<COperation> op);

    void DumpUses(std::ostream& outfile);
public:
    bool Strict = false;
    bool ModelIncludes;
};

#endif // CPHPCLASS_H
