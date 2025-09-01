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
#include "cmodelpackage.h"

#include "cmodel.h"

std::string CModelPackage::FQN() const {
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

void CModelPackage::Prepare(void) {
    OutputName=name;
    PrepareBase(tags);

    for ( auto & i : Classes) {
        i->Prepare();
    }
    for (auto & di : Dependency) {
        di->Prepare();
    }
    for (auto & p : Packages) {
        p->Prepare();
    }
}

void CModelPackage::Dump(std::shared_ptr<MModel> model) {
    std::string             path;
    std::list<std::string>  modules;
    auto                    cmodel = std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel);
#if 0
    path=cmodel->pathstack.back()+"/"+".Makefile";
    cmodel->generatedfiles.push_back(std::pair <std::string, std::string>(path, "##"));
    OpenStream(makefile, path);
    DumpMakefileHeader(makefile, OutputName+".so");
#endif
    for (auto & i : Classes) {
        if ((i->type == eElementType::SimObject) || (i->type == eElementType::CxxClass) || (i->type == eElementType::CClass)) {
            modules.push_back(i->name);
        }
        i->Dump(model);
    }
    for (auto & p : Packages) {
        p->Dump(model);
    }
#if 0
    makefile << "PROJ=" << OutputName+"\n\n";
    DumpMakefileSource(makefile, modules);
    DumpMakefileObjects(makefile, modules);
    makefile << "CXXFLAGS+=-std=gnu++0x -fPIC -g\n\n";
    makefile << "\n";
    makefile << "all : $(PROJ)\n\n";
    makefile << "$(PROJ) : lib$(PROJ).so lib$(PROJ).a\n\n";
    makefile << "lib$(PROJ).so : $(OBJ)\n";
    if (!modules.empty()) {
        makefile << "\tg++ -shared $(CXXFLAGS) $(OBJ) -o $@\n\n";
    }
    makefile << "lib$(PROJ).a : $(OBJ)\n";
    if (!modules.empty()) {
        makefile << "\tar cr $@ $(OBJ)\n\n";
    }
    if (!modules.empty()) {
        makefile << "depend : $(SRC)\n";
        makefile << "\tg++ $(CXXFLAGS) -M $(SRC) > depend\n\n";
    }

    makefile << "clean:\n";
    makefile << "\trm -f $(PROJ)\n";
    makefile << "\trm -f depend\n";
    makefile << "\trm -f $(OBJ)\n\n";

    makefile << "-include depend\n\n";

    makefile.close();
#endif
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
}
