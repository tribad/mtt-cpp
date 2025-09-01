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
#ifndef COPERATION_H
#define COPERATION_H

#include <string>
#include <vector>
#include <map>
#include "namespace.h"
#include "melement.h"
#include "mparameter.h"
#include "cparameter.h"

class COperation : public MOperation
{
public:
    COperation() = default;
    COperation(const std::string& aId, std::shared_ptr<MElement> e) : MOperation(aId, e) {}
    virtual ~COperation() = default;
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);
    std::string GetReturnType(const NameSpace& aNameSpace);
    inline std::string GetReturnName(void);
    inline std::string GetReturnDefault(void);
    std::string GetParameterDecl(const NameSpace& aNameSpace);
    std::string GetParameterSignature(const NameSpace& aNameSpace);
    std::string GetParameterDefinition(const NameSpace& aNameSpace);
    std::string GetPHPParameterDefinition(void);
    std::string getHeader(int indent);
    std::string getSourceHeader(int indent);
    /*
     */
    std::string GetCTORInitList(void);
    bool IsCastOperator();
    bool isTemplateOperation() {
        return HasStereotype("template");
    }
    void DumpTemplateOperationPrefix(std::ostream& file, bool strip_default, int a_indent = 0);
private:
    std::string getCtorInitFromSpec();
    std::map <std::string, std::pair<std::string, std::string>> parseCtorInitFromSpec(const std::string& aCtorInit);
public:
    bool qtSlot   = false;
    bool qtSignal = false;
    bool isInline = false;
    std::vector< std::pair <std::string, std::string> > mCtorInitList;
};


inline std::string COperation::GetReturnDefault() {
    std::string retval;

    for (auto & i : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction == "return") {
            retval = para->defaultValue;
            break;
        }
    }
    return retval;
}

inline std::string COperation::GetReturnName() {
    std::string retval;

    for (auto & i : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction == "return") {
            retval = i->name;
            break;
        }
    }
    return retval;
}

#endif // COPERATION_H
