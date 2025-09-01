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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include <iostream>
#include "helper.h"
#include "cphppackage.h"

#include "cphpclass.h"
#include "cmodel.h"

void CPHPPackage::UpdateParent() {
    for (auto x : Classes) {
        x->parent = id;
    }
    for (auto x : Packages) {
        x->parent = id;
    }
    for (auto x : Dependency) {
        x->parent = id;
    }
    for (auto x : Objects) {
        x->parent = id;
    }
}

std::string CPHPPackage::FQN() const {
    return GetNameSpace();
}

void CPHPPackage::Prepare(void) {
    //
    //  Check for the autoloader tag.
    //  Its done before any other because it is a
    //  switch-on for some that may not be handled correctly.
    if (HasTaggedValue("autoloader")) {
        if (!HasTaggedValue("directory")) {
            AddTag("Directory", "");
        }
        if (!HasTaggedValue("namespace")) {
            AddTag("NameSpace", "");
        }
    }
    //
    //  Setup some default tags.
    PrepareBase(tags);
    //
    //  Add some special PHP tags processing.
    if (HasTaggedValue("strict")) {
        std::string s = GetTaggedValue("strict");
        if (s.empty()) {
            Strict = true;
        } else {
            if (helper::tolower(s) == "true") {
                Strict = true;
            } else if (helper::tolower(s) == "false") {
                Strict = false;
            } else {
                std::cerr << "Invalid value for tagged value \"Strict\": " << s << "in package " << name << std::endl;
            }
        }
    }
    for (auto  & ci : Classes) {
        if (ci->type == eElementType::PHPClass) {
            auto php = std::dynamic_pointer_cast<CPHPClass>(*ci);
            php->Strict = Strict;
            //
            //  If we have the autoloader we do not want the modelincludes.
            if(HasTaggedValue("AutoLoader")) {
                php->ModelIncludes = false;
            }
        }
        ci->Prepare();
    }
    for (auto & di : Dependency) {
        di->Prepare();
    }

    for (auto & pi : Packages) {
        //
        //  We need to convert simple packages into PHP packages because in
        //  PHP subdirs are not subsystems or the like. They are PHP packages.
        if (pi->type == eElementType::Package) {
#if 0   ///  Somewhere in the code there is a better solution for this problem.
            CPHPPackage* php = new CPHPPackage;
            //
            //  Move the data inclusive the makefile ofstream into the new PHP package.
            *((CPackageBase*)(php)) = std::move(*((CPackageBase*)(*pi)));
            //
            //  Drop the old pointer stored in the vector
            delete *pi;
            //
            //  Set the pointer in the vector to the new PHP package
            *pi = php;
            //
            //  Update the parents
            ((CPHPPackage*)(*pi))->UpdateParent();
            //
            //  We are losing the Namespace delimiter
            ((CPHPPackage*)(*pi))->NameSpaceDelimiter = "\\";
            //
            //  We are losing the element-type too.
            //  So restore it.
            ((CPHPPackage*)(*pi))->type = eElementType::PHPPackage;
#endif
        }
        //
        //  We may have other packages than PHP-packages below this point.
        //  So we check it.
        if (pi->type == eElementType::PHPPackage) {
            //
            //  Before preparation of one package down. We set the strict flag
            std::dynamic_pointer_cast<CPHPPackage>(*pi)->Strict = Strict;
            //
            //  If we have the autoloader tag set we set it in the PHP-package as well.
            if (HasTaggedValue("autoloader")) {
                pi->AddTag("AutoLoader", "");
            }
        }
        //
        //  Prepare one down anyway what package type it is.
        pi->Prepare();
    }
}

void CPHPPackage::Dump(std::shared_ptr<MModel> model) {
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel);
    for (auto & ci : Classes) {
        ci->Dump(model);
    }
    for (auto & p : Packages) {
        p->Dump(model);
    }
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();

}
