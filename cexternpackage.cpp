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
#include "mdependency.h"
#include "cdependency.h"

#include "cpackagebase.h"
#include "cexternpackage.h"

#include "clibrarypackage.h"

std::string CExternPackage::FQN() const {
    std::string val;

    if (mNameSpace.empty()) {
    } else {
        val = mNameSpace.getString() + "::";
    }
    if (parent) {
        return (parent->FQN()+val);
    }
    return val;
}

void CExternPackage::Prepare(void) {
    PrepareBase(tags);
    if (OutputName.empty()) {
        OutputName = name;
    }
    for (auto & i : Classes) {
        if (!(i->HasTaggedValue("ExtraInclude"))) {
            if (!ExtraInclude.empty()) {
                i->AddTag("ExtraInclude", ExtraInclude);
            } else {
                //if (i->type != eElementType::PrimitiveType) {
                    i->AddTag("ExtraInclude", name);
                //}
            }
        }
        i->Prepare();
    }
    for (auto & p : Packages) {
        p->Prepare();
    }


}

std::list<tConnector<MElement, MElement>> CExternPackage::GetLibraryDependency() {
    std::list<tConnector<MElement, MElement>> result;
    std::list<tConnector<MElement, MElement>> more;

    for (auto & di : Supplier) {
        if (di.getElement()) {
            if (di.getElement()->type == eElementType::LibraryPackage) {
                more = std::dynamic_pointer_cast<CLibraryPackage>(di.getElement())->GetLibraryDependency() ;
            } else if  (di.getElement()->type == eElementType::ExternPackage) {
                more = std::dynamic_pointer_cast<CExternPackage>(di.getElement())->GetLibraryDependency() ;
            }
            result.push_back(di);
            if (!more.empty()) {
                result.insert(result.end(), more.begin(), more.end());
            } else {
            }

        } else {

        }
    }

    return (result);
}


void CExternPackage::Dump(std::shared_ptr<MModel> model) {
    (void) model;
}
