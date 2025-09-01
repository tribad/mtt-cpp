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
#include "cpackagebase.h"
#include "cjspackage.h"

#include "mmodel.h"
#include "cmodel.h"

std::string CJSPackage::FQN() const {
    return name;
}

void CJSPackage::Prepare(void) {
    OutputName=name;
    PrepareBase(tags);
    for (auto & i : Classes) {
        i->Prepare();
    }

}

void CJSPackage::Dump(std::shared_ptr<MModel> model) {
    std::string path;
    std::list<std::string> modules;

    auto  cmodel=std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel);

    for (auto & i : Classes) {
        if (i->type == eElementType::JSClass) {
            modules.push_back(i->name);
        }
        i->Dump(model);
    }
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
}
