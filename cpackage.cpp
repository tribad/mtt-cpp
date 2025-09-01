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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include <iostream>
#include "main.h"
#include "cpackage.h"
#include "cmodel.h"

std::string CPackage::FQN() const {
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

void CPackage::Prepare(void) {
    std::map<std::string, std::string>::iterator i;
    std::vector<MClass*>::iterator               ci;
    std::vector<MDependency*>::iterator          di;

    PrepareBase(tags);

    for (auto& o : owned) {
        o->Prepare();
    }
}

void CPackage::Dump(std::shared_ptr<MModel> model) {
    std::string path;
    auto     cmodel = std::dynamic_pointer_cast<CModel>(model);
    //
    //  Quick exit if generation is limited to a specific id
    if (gDumpList.find(id) != gDumpList.end()) {
        gDumpStarted = true;
    }
    if (!gDumpStarted) {
        return;
    }

    DumpBase(cmodel);
    for (auto& o : owned) {
        o->Dump(model);
    }
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
    //
    // Stop dumping 
    if (gDumpList.find(id) != gDumpList.end()) {
        gDumpStarted = false;
    }
}
