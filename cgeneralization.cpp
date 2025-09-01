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
#include "melement.h"
#include "mparameter.h"
#include "cgeneralization.h"
#include "mclass.h"

std::string CGeneralization::FQN() const {
    return name;
}

void CGeneralization::Prepare(void) {
    std::shared_ptr<MClass> b = std::dynamic_pointer_cast<MClass>(*base);
    std::shared_ptr<MClass> d = std::dynamic_pointer_cast<MClass>(*derived);

    if (base && d && (base->IsClassBased())) {
        b->mSubClass.emplace_back(d);
    }
    if (derived && b && (derived->IsClassBased())) {
        d->mSubClass.emplace_back(b);
    }
    for (auto & tb : mTemplateParameter) {
#if 1
        auto param = std::dynamic_pointer_cast<MParameter>(*tb.second);
        //
        //  mActual is only used for initialization as we can not determine if the reference
        //  goes to a template Parameter or a classifier.
        //  As the EA does it differently we check if mActual is set at all.
        if (param->mActual) {
            if (param->mActual->IsClassBased()) {
                param->Classifier = param->mActual;
                param->mActual    = MElementRef();
            } else if (param->mActual->type == eElementType::Parameter) {
                auto tp = std::dynamic_pointer_cast<MParameter>(*param->mActual);

                param->Classifier = tp->Classifier;
                param->mActual    = MElementRef();
            }
        }
#endif
    }
}

void CGeneralization::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}
