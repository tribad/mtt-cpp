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
#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#endif

#include "path.h"

#include "helper.h"
#include "main.h"
#include "mdependency.h"
#include "cdependency.h"

#include "cpackagebase.h"
#include "clibrarypackage.h"

#include "cexternpackage.h"

#include "cclassbase.h"

#include "mmodel.h"
#include "cmodel.h"

std::string CLibraryPackage::FQN() const {
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

void CLibraryPackage::Prepare(void) {
    //
    //  Set the defaults for library packages.
    OutputName = name;
    Directory  = "/"+name;
    PrepareBase(tags);
    for (auto t : tags) {
        std::string tagname = helper::tolower(t.first);

        if (tagname == "repotype") {
            if (helper::tolower(t.second) == "svn") {
                RepoType = ERepositoryType::svn;
            } else if (helper::tolower(t.second) == "git") {
                RepoType = ERepositoryType::git;
            } else {
                RepoType = ERepositoryType::none;
            }
        } else if (tagname == "createsubsystem") {
            if (helper::tolower(t.second) == "true") {
                mCreateSubsystem = true;
            } else if (helper::tolower(t.second) == "false") {
                mCreateSubsystem = false;
            } else {
                mCreateSubsystem = true;
            }
        } else if (tagname == "subsystemformat") {
            if (helper::tolower(t.second) == "ea-xmi") {
                mSubsystemFormat = SubsystemFormat::EAXMI;
            } else if (helper::tolower(t.second) == "staruml") {
                mSubsystemFormat = SubsystemFormat::EAXMI;
            } else {
                mSubsystemFormat = SubsystemFormat::None;
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


void CLibraryPackage::Dump(std::shared_ptr<MModel> model) {
    std::string                               path;
    std::list<std::string>                    modules;
    std::list<eElementType>                   contenttypes;
    std::list<std::shared_ptr<MClass>>        content;
    std::list<tConnector<MElement, MElement>> liblist;

    auto cmodel = std::dynamic_pointer_cast<CModel>(model);
    //
    //  Quick exit if generation is limited to a specific id
    if (gDumpList.find(id) != gDumpList.end()) {
        gDumpStarted = true;
    }
    if (!gDumpStarted) {
        return;
    }
    
    DumpBase(cmodel);
    //
    //  Create the export file stuff
    if (mCreateSubsystem) {
        if (mSubsystemFormat == SubsystemFormat::EAXMI) {
            path = cmodel->pathstack.back() + "/." + OutputName +".xmi";
            cmodel->generatedfiles.push_back(tGenFile{path, id, "", "xmi"});
            OpenStream(mExportFile, path);
            DumpEAIntro();
        }
    }
    //
    //  Create the makefile infos.
    path = cmodel->pathstack.back()+"/"+".Makefile";
    cmodel->generatedfiles.push_back(tGenFile {path, id, "##", "mk"});
    OpenStream(makefile, path);
    DumpMakefileHeader(makefile, OutputName+".so");
    //
    //  Get all content that is part of this package
    contenttypes.push_back(eElementType::SimObject);
    contenttypes.push_back(eElementType::SimEnumeration);
    contenttypes.push_back(eElementType::CxxClass);
    contenttypes.push_back(eElementType::SimMessageClass);
    contenttypes.push_back(eElementType::SimSignalClass);
    contenttypes.push_back(eElementType::Struct);
    contenttypes.push_back(eElementType::Enumeration);
    contenttypes.push_back(eElementType::CClass);
    contenttypes.push_back(eElementType::InterfaceClass);

    content = GetAllContent(contenttypes);
        

    for (auto & ci : content) {
        //
        //  If the class is in a different package it should be one of the subpackages.
        if ( ci->parent && (ci->parent != sharedthis<MElement>()) && (ci->parent->IsPackageBased())) {
            std::string pathto = GetPathToPackage(ci->parent);
            if ((ci->type == eElementType::SimObject) || (ci->type == eElementType::CxxClass) || (ci->type == eElementType::CClass)) {
                auto cc = std::dynamic_pointer_cast<CClassBase>(ci);
                //
                //  Do not expect a cpp file for templates.
                if (!cc->isTemplateClass()) {
                    modules.push_back("./" + pathto + "/" + ci->name);
                }
            }

            CPath parentpath(cmodel->pathstack.back() + pathto);

            parentpath.Create();
            cmodel->pathstack.push_back((std::string)parentpath);
            //
            //  Setup the directory for the subpackage.
            ci->Dump(model);
            //
            //  Remove last element from path-stack. Was for the subpackage.
            cmodel->pathstack.pop_back();
        } else {
            if ((ci->type == eElementType::SimObject) || (ci->type == eElementType::CxxClass) || (ci->type == eElementType::CClass)) {
                auto cc = std::dynamic_pointer_cast<CClassBase>(ci);
                //
                //  Do not expect a cpp file for templates.
                if (!cc->isTemplateClass()) {
                    modules.push_back(ci->name);
                }
            }
            ci->Dump(model);
        }
    }
    for (auto & p : Packages) {
        p->Dump(model);
    }
    //
    //  Dump the export
    if (mCreateSubsystem) {
        if (mSubsystemFormat == SubsystemFormat::EAXMI) {
           DumpEA(cmodel, mExportFile);
           DumpEAExtension(cmodel, mExportFile);
        }
    }
    makefile << "PROJ=" << OutputName+"\n\n";
    DumpMakefileSource(makefile, modules);
    DumpMakefileObjects(makefile, modules);
    //
    //  If we dump the test dir as well add the dir to the phony list.
    if (!cmodel->TestEnvironment.empty()) {
        makefile << ".PHONY: test\n\n";
    }
    //
    //  Find some CXX-Flags to generate into the Makefile.
    std::string cxxFlags;
    if (HasTaggedValue("CxxFlags")) {
        cxxFlags = GetTaggedValue("CxxFlags");
    }
    makefile << "CXXFLAGS+=-std=" << m_cxxstandard << " -fPIC -g $(INCLUDEPATH) " << cxxFlags.c_str() << "\n\n";
    makefile << "\n";

    //
    //  First we add the libs.
    liblist = GetLibraryDependency();

    makefile << "STATICLIBS+=";
    for (auto& di : liblist) {
        if ((di.getConnector()->HasStereotype("static")) || (di.getConnector()->HasStereotype("StaticLinkage"))) {
            if (di.getElement()) {
                auto package = std::dynamic_pointer_cast<CPackageBase>(di.getElement());
                if (di.getElement()->type == eElementType::LibraryPackage) {
                    makefile << "\\\n" << "     -l" << package->OutputName ;
                } else if (di.getElement()->type == eElementType::ExternPackage) {
                    makefile << "\\\n" << "     -l" << package->OutputName ;
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
                } else if (di.getElement()->type == eElementType::ExternPackage) {
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
        if (di.getElement() != nullptr) {
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

                if (package->Directory != "./") {
                    makefile << "\\\n" << "     -I" << package->Directory;
                } else {
                    if (!package->OutputPath.empty()) {
                        makefile << "\\\n" << "     -I" << package->OutputPath;
                    }
                }
            }
        }
    }
    makefile << "\n\n";

    //
    //  Check if we need to do anything.
    if (!modules.empty()) {
        makefile << "VERSION:=$(shell if [ ! -e VERSION ] ; then echo \"1\" > VERSION; fi; cat VERSION;)\n";
        makefile << "OLDREVISION:=$(shell if [ ! -e REVISION ] ; then echo \"1\" > REVISION; fi; cat REVISION;)\n";
        switch (RepoType) {
        case ERepositoryType::git:
            makefile << "REVISIONNR:=$(shell git rev-list --count HEAD -- .)\n";
            break;
        case ERepositoryType::svn:
            break;
        default:
            makefile << "##\n## No repository so we handle revisions by hand.\n";
            makefile << "REVISIONNR:=$(OLDREVISION)\n";
            break;
        }
        makefile << "ifneq ($(OLDREVISION), $(REVISIONNR))\n";
        makefile << "    BUILDNR:=$(shell echo \"1\" > BUILD;echo $(REVISIONNR) > REVISION;if [ -e lib$(PROJ).so ] ; then touch lib$(PROJ).so; fi;cat BUILD)\n";
        makefile << "else\n";
        makefile << "    BUILDNR:=$(shell if [ ! -e BUILD ] ; then echo \"1\" > BUILD; fi; cat BUILD;)\n";
        makefile << "endif\n"
                    "\nNEWBUILDNR:=$(shell echo $$(( $(BUILDNR) +1 )))\n\n";

        makefile << "all : lib$(PROJ).so lib$(PROJ).a BUILD\n\n";
        makefile << "lib$(PROJ).so : $(OBJ)\n";
        if (!modules.empty()) {
            makefile << "\tg++ -shared $(CXXFLAGS) $(LDFLAGS) $(LIBPATH) $(OBJ) -Wl,-Bstatic -Wl,--start-group $(STATICLIBS) -Wl,--end-group -Wl,-Bdynamic -Wl,--start-group $(DYNLIBS) -Wl,--end-group -o $@\n\n";
        }
        makefile << "lib$(PROJ).a : $(OBJ)\n";
        if (!modules.empty()) {
            makefile << "\tar cr $@ $(OBJ)\n\n";
        }
        makefile << "BUILD : lib$(PROJ).so\n";
        makefile << "\tcp lib$(PROJ).so lib$(PROJ).so.$(VERSION).$(REVISIONNR).$(NEWBUILDNR)\n";
        makefile << "\t$(shell echo $(NEWBUILDNR) > BUILD)\n\n"
                    "\tBUILDNR=$(shell cat BUILD)\n\n";

        if (!modules.empty()) {
            makefile << "depend : $(SRC)\n";
            makefile << "\tg++ $(CXXFLAGS) -M $(SRC) > depend\n\n";
        }

        makefile << "clean:\n";
        makefile << "\trm -f lib$(PROJ).a\n";
        makefile << "\trm -f lib$(PROJ).so*\n";
        makefile << "\trm -f depend\n";
        makefile << "\trm -f $(OBJ)\n\n";

        makefile << "install:\n"
                    "\trm -f $(LIBINSTALLDIR)/lib$(PROJ).*\n"
                    "\tcp lib$(PROJ).a $(LIBINSTALLDIR)\n"
                    "\tcp lib$(PROJ).so.$(VERSION).$(REVISIONNR).$(BUILDNR) $(LIBINSTALLDIR)\n"
                    "\tln -s $(LIBINSTALLDIR)/lib$(PROJ).so.$(VERSION).$(REVISIONNR).$(BUILDNR) $(LIBINSTALLDIR)/lib$(PROJ).so\n\n";

        if (!cmodel->TestEnvironment.empty()) {
            DumpTestDir(cmodel->pathstack.back()+"/test", modules);
        }
        makefile << "-include depend\n\n";
    } else {
        makefile << "all :\n";
        makefile << "\t$(info No modules to compile)\n\n";
        makefile << "clean :\n";
        makefile << "\t$(info Nothing to clean)\n\n";
        makefile << "install :\n";
        makefile << "\t$(info Nothing to install)\n";
    }
    //
    //  close files that have been done.
    makefile.close();
    DumpEAOutro();
    mExportFile.close();
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
    //
    // Stop dumping 
    if (gDumpList.find(id) != gDumpList.end()) {
        gDumpStarted = false;
    }
}

void CLibraryPackage::DumpTestDir(const std::string &aTestDir, const std::list<std::string>& aModules) {
    //
    //  Setup test target
    makefile << "test:\n"
                "\t$(MAKE) -C test\n\n";

    int err;
    struct stat dirstat;

    err=stat(aTestDir.c_str(), &dirstat);
    if ((err == -1) && (errno==ENOENT)) {
        helper::mkdir(aTestDir.c_str(), 0777);
    }

    std::set<std::string> dirlist;
    //
    //  create the directories for to hold the object files.
    for (auto & i : aModules) {
        CPath fname(i);

        std::string dir = fname.Directory();
        std::string dirpath = aTestDir+"/"+dir;

        if ((!dir.empty()) && (dirpath != "./")) {
            if (dirlist.find(dirpath) == dirlist.end()) {
                dirlist.insert(dirpath);
                err=stat(dirpath.c_str(), &dirstat);
                if ((err == -1) && (errno==ENOENT)) {
                    helper::mkdir(dirpath.c_str(), 0777);
                }
            }
        }
    }

    std::string testmakefilename = aTestDir+"/Makefile";
    std::ofstream  testmakefile(testmakefilename);

    testmakefile << "PROJ=" << OutputName << "Test\n\n";

    testmakefile << "SRC=";

    for (auto i : aModules) {
        CPath fname(i);

        testmakefile << "\\\n    " << "." << i << ".cpp";
        testmakefile << "\\\n    " << fname.Directory() << "/test_" << fname.Base() << ".cpp";
    }
    testmakefile << "\n\n";

    testmakefile << "OBJS=";

    for (auto i : aModules) {
		CPath fname(i);

        testmakefile << "\\\n    " << i << ".o";
        testmakefile << "\\\n    " << fname.Directory() << "/test_" << fname.Base() << ".o";
    }
    testmakefile << "\n\n";

    testmakefile << "all : test\n\n"
                    "test : $(PROJ)\n"
                    "\t$(PROJ)\n\n"
                    "$(PROJ) : $(OBJS)\n"
                    "\t$(CXX) -g -pthread -lgtest $(OBJS) -o $(PROJ)\n\n";


    for (auto i : aModules) {
		CPath fname(i);

        testmakefile << i << ".o : ." << i << ".cpp\n"
                        "\t$(CXX) -c -g -I.. $< -o $@\n\n";

        testmakefile << fname.Directory() << "/test_" << fname.Base() << ".o : " << fname.Directory() << "/test_" << fname.Base() << ".cpp\n"
                        "\t$(CXX) -c -g -I.. $< -o $@\n\n";
    }
}

void CLibraryPackage::DumpEAIntro() {
    mExportFile << "<?xml  version='1.0' encoding='windows-1252' ?>\n"
                   "<xmi:XMI xmlns:xmi=\"http://www.omg.org/spec/XMI/20131001\" xmlns:uml=\"http://www.omg.org/spec/UML/20161101\" xmlns:umldi=\"http://www.omg.org/spec/UML/20161101/UMLDI\" xmlns:dc=\"http://www.omg.org/spec/UML/20161101/UMLDC\" xmlns:thecustomprofile=\"http://www.sparxsystems.com/profiles/thecustomprofile/1.0\" xmlns:EAUML=\"http://www.sparxsystems.com/profiles/EAUML/1.0\" xmlns:EAREQ=\"http://www.sparxsystems.com/profiles/EAREQ/1.0\" xmlns:mtt-cpp=\"http://www.sparxsystems.com/profiles/mtt-cpp/1.0\">\n"
                   "<xmi:Documentation exporter=\"Enterprise Architect\" exporterVersion=\"6.5\" exporterID=\"1605\"/>\n"
                   "\t<uml:Model xmi:type=\"uml:Model\" name=\"EA_Model\">" << std::endl;

}



void CLibraryPackage::DumpEAOutro() {
    mExportFile << 	"\t</uml:Model>\n"
                       "</xmi:XMI>" << std::endl;


}
