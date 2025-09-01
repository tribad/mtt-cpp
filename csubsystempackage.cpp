//
// Copyright 2023 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#include "path.h"
#include "helper.h"
#include "cmodel.h"
#include "cdependency.h"
#include "csubsystempackage.h"

std::string CSubsystemPackage::FQN() const {
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

void CSubsystemPackage::Prepare(void) {
    PrepareBase(tags);
    for (auto & t : tags) {
        std::string tagname = helper::tolower(t.first);

        if (tagname == "repotype") {
            if (helper::tolower(t.second) == "svn") {
                RepoType = ERepositoryType::svn;
            } else if (helper::tolower(t.second) == "git") {
                RepoType = ERepositoryType::git;
            } else {
                RepoType = ERepositoryType::none;
            }
        }
    }
    for (auto & i : Classes) {
        i->Prepare();
    }
    for (auto & di : Dependency) {
        di->Prepare();
    }
    for (auto & p : Packages) {
        p->Prepare();
    }
}

void CSubsystemPackage::Dump(std::shared_ptr<MModel> model) {
    std::ofstream document;
    CPath path;
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel);

//    path = cmodel->pathstack.back() + OutputName + ".adoc";
//    OpenStream(document, path);

    for (auto& o : owned) {
        o->Dump(model);
    }
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
}