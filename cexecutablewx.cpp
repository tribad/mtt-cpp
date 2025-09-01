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
#include <helper.h>
#include "cpackagebase.h"
#include "cexecutablewx.h"

#include "mmodel.h"
#include "cmodel.h"

#include "cclassbase.h"

#include "path.h"

void CExecutableWx::Dump(std::shared_ptr<MModel> model) {
    std::string                               path;
    std::list<std::string>                    modules;
    std::vector<std::shared_ptr<MClass>>::iterator i;
    std::list<tConnector<MElement, MElement>>    liblist;
    std::list<eElementType>                      contenttypes;
    std::list<std::shared_ptr<MClass>>           content;
    std::list<std::shared_ptr<MClass>>::iterator ci;

    auto cmodel = std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel);
    path=cmodel->pathstack.back()+"/"+".Makefile";
    cmodel->generatedfiles.push_back( tGenFile {path, id, "##", "mk"} );
    OpenStream(makefile, path);
    DumpMakefileHeader(makefile, OutputName);
    //
    //  Get all content that is part of this package
    contenttypes.push_back(eElementType::Enumeration);
    contenttypes.push_back(eElementType::Union);
    contenttypes.push_back(eElementType::CxxClass);
    contenttypes.push_back(eElementType::CClass);
    contenttypes.push_back(eElementType::Struct);
    contenttypes.push_back(eElementType::InterfaceClass);
    contenttypes.push_back(eElementType::WxFormsClass);

    content = GetAllContent(contenttypes);

    for (ci=content.begin(); ci!=content.end(); ci++) {
        auto ccb = std::dynamic_pointer_cast<CClassBase>(*ci);
        //
        //  If the class is in a different package it should be one of the subpackages.
        if ( ((*ci)->parent) && ((*ci)->parent != sharedthis<MElement>()) && ((*ci)->parent->IsPackageBased())) {
            std::string pathto = GetPathToPackage((*ci)->parent);
            if (((*ci)->type == eElementType::SimObject) || ((*ci)->type == eElementType::CxxClass) ||
                ((*ci)->type == eElementType::CClass) || ((*ci)->type == eElementType::WxFormsClass)) {
                modules.push_back(CPath("./"+pathto+"/"+ccb->getFileName()));
            }
            //
            //  Setup the directory for the subpackage.
            std::dynamic_pointer_cast<CPackageBase>(*(*ci)->parent)->DumpBase(cmodel);
            (*ci)->Dump(model);
            //
            //  Remove last element from path-stack. Was for the subpackage.
            cmodel->pathstack.pop_back();
        } else {
            if (((*ci)->type == eElementType::SimObject) || ((*ci)->type == eElementType::CxxClass) ||
                ((*ci)->type == eElementType::CClass) || ((*ci)->type == eElementType::WxFormsClass)) {
                modules.push_back(CPath("./"+ccb->getFileName()));
            }
            (*ci)->Dump(model);
        }
    }
    makefile << "PROJ=" << OutputName+"\n\n";
    DumpMakefileSource(makefile, modules);
    DumpMakefileObjects(makefile, modules);
    //
    //  Find some CXX-Flags to generate into the Makefile.
    std::string cxxFlags;
    if (HasTaggedValue("CxxFlags")) {
        cxxFlags = GetTaggedValue("CxxFlags");
    }
    makefile << "CXXFLAGS+=$(shell wx-config --debug=yes --cxxflags) -std=" << m_cxxstandard << " -fPIC -g $(INCLUDEPATH) " << cxxFlags.c_str() << "\n\n";

    //
    //  Find some CXX-Flags to generate into the Makefile.
    makefile << "LDFLAGS+=$(shell wx-config --debug=yes --libs std,richtext,aui,propgrid) ";

    std::string ldFlags;
    if (HasTaggedValue("LdFlags")) {
        ldFlags = GetTaggedValue("LdFlags");
         makefile << ldFlags;
    }
    makefile  << std::endl << std::endl;
    //
    //  First we add the libs.
    liblist = GetLibraryDependency();

    makefile << "STATICLIBS+=";
    for (auto& di : liblist) {
        if ((di.getConnector()->HasStereotype("static")) || (di.getConnector()->HasStereotype("StaticLinkage"))) {
            if (di.getElement()) {
                auto package = std::dynamic_pointer_cast<CPackageBase>(di.getElement());

                if (di.getElement()->type == eElementType::LibraryPackage) {
                    makefile << "\\\n" << "     -l" << package->OutputName;
                }
                else if (di.getElement()->type == eElementType::ExternPackage) {
                    makefile << "\\\n" << "     -l" << package->OutputName;
                }
            }
        }
    }
    makefile << "\n\n";
    makefile << "DYNLIBS+=";
    for (auto& di : liblist) {
        if (!(di.getConnector()->HasStereotype("static")) && !(di.getConnector()->HasStereotype("StaticLinkage"))) {
            if (di.getElement()) {
                auto package = std::dynamic_pointer_cast<CPackageBase>(di.getElement());

                if (di.getElement()->type == eElementType::LibraryPackage) {
                    makefile << "\\\n" << "     -l" << package->OutputName;
                }
                else if (di.getElement()->type == eElementType::ExternPackage) {
                    makefile << "\\\n" << "     -l" << package->OutputName;
                }
            }
        }
    }
    makefile << "\n\n";
    //
    //  Than we add the pathes.

    makefile << "LIBPATH+=";
    if (HasTaggedValue("LibPath")) {
        makefile << "-L" << GetTaggedValue("LibPath") << "\\\n";
    }
    for (auto& di : liblist) {
        if (di.getElement()) {
            if (di.getElement()->type == eElementType::LibraryPackage) {
                std::string libpath = GetPathToPackage(di.getElement());
                if (!libpath.empty()) {
                    makefile << "\\\n" << "     -L" << libpath;
                }
            }
        }
    }
    makefile << "\n\n";

    makefile << "INCLUDEPATH+=";
    for (auto& di : liblist) {
        if (di.getElement()) {
            if (di.getElement()->type == eElementType::ExternPackage) {
                auto package = std::dynamic_pointer_cast<CPackageBase>(di.getElement());

                std::string dir = package->Directory;
                if (dir != "./") {
                    makefile << "\\\n" << "     -I" << dir;
                } else {
                    if (!package->OutputPath.empty()) {
                        makefile << "\\\n" << "     -I" << package->OutputPath;
                    }
                }
            }
        }
    }
    makefile << "\n\n";

    makefile << "all : $(PROJ)\n\n";
    makefile << "$(PROJ) : $(OBJ) depend\n";
    if (!modules.empty()) {
        makefile << "\tg++ $(CXXFLAGS) $(LIBPATH) $(OBJ) -Wl,-Bstatic -Wl,--start-group $(STATICLIBS) -Wl,--end-group -Wl,-Bdynamic -Wl,--start-group $(DYNLIBS) -Wl,--end-group $(LDFLAGS) -o $@\n\n";
    }
    if (!modules.empty()) {
        makefile << "depend : $(SRC)\n";
        makefile << "\tg++ $(CXXFLAGS) -M $(SRC) > depend\n\n";
    }

    makefile << "clean:\n";
    makefile << "\trm -f $(PROJ)\n";
    makefile << "\trm -f depend\n";
    makefile << "\trm -f $(OBJ)\n\n";
    makefile << "install:\n"
                "\trm -f $(BININSTALLDIR)/$(PROJ)\n"
                "\tcp $(PROJ) $(BININSTALLDIR)\n\n";

    makefile << "-include depend\n\n";

    makefile.close();
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
}
