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
#include "typenode.h"

#include "cclassbase.h"
#include "cprimitivetype.h"

void CPrimitiveType::SetFromTags(const std::string& name, const std::string&value)
{

}

std::string CPrimitiveType::FQN() const {
    return mTypeTree.getFQN();
}

void CPrimitiveType::Prepare(void) {
    if (!PrepDone) {
        PrepareBase();
        PrepDone = true;
    }
}

void CPrimitiveType::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}

bool CPrimitiveType::isAlias() {
    bool retval = false;

    if (!Base.empty()) {
        if (Base.size() > 1) {
            std::cerr << "Primitive: '" << name << "' has more than one base class.\n";
        } else {
            auto cb = *(Base.begin());

            if (cb.getElement()) {
                retval = true;
            }
        }
    } else {
        if (!mTypeTree.isCompositeType()) {
            if (Supplier.empty() && !mAlias.empty()) {
                retval = true;
            } else {
                for (auto & u : Supplier) {
                    if ((u.getConnector()->type == eElementType::Dependency) && (u.getConnector()->HasStereotype("use")) && (u.getElement()->IsClassBased())) {
                        retval = true;
                    }
                }
            }
        }
    }
    return retval;
}

std::shared_ptr<CClassBase> CPrimitiveType::getAliasBase() {
    std::shared_ptr<CClassBase> retval;

    if (!Base.empty()) {
        if (Base.size() > 1) {
            std::cerr << "Primitive: '" << name << "' has more than one base class.\n";
        } else {
            retval = Base.begin()->getElement();

            if (retval && (retval->IsClassBased())) {
                if (retval->type == eElementType::PrimitiveType) {
                    auto pr = std::dynamic_pointer_cast<CPrimitiveType>( retval);
                    retval = pr->getAliasBase();
                }
            }
        }
    }

    return retval;
}
