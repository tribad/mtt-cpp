//
// Copyright 2024 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include "cmodulepackage.h"

#include "cexternpackage.h"

#include "cclassbase.h"

#include "mmodel.h"
#include "cmodel.h"

std::string CModulePackage::FQN() const {
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

void CModulePackage::Prepare(void) {
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
    //
    //  Here we create a module-class that will be used as container for the elements that are linked to the module-package.
    auto mc = MClass::construct(id + "-module", "moduleclass", sharedthis<MElement>());

    mc->name = OutputName;
    mc->Prepare();

    for (auto & i : Classes) {
        mc->owned.push_back(i);
        i->parent = mc;
    }
    Classes.clear();

    std::list<MElementRef> mvclasses;
    for (auto c = owned.begin(); c != owned.end(); ) {
        if ((*c)->IsClassBased()) {
            c = owned.erase(c);
        } else {
            ++c;
        }
    }

    Add(mc);
}


void CModulePackage::Dump(std::shared_ptr<MModel> model) {
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
    //  Get all content that is part of this package
    contenttypes.push_back(eElementType::SimObject);
    contenttypes.push_back(eElementType::SimEnumeration);
    contenttypes.push_back(eElementType::CxxClass);
    contenttypes.push_back(eElementType::ModuleClass);
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
            //
            //  These are the class-type elements.
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
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
    //
    // Stop dumping 
    if (gDumpList.find(id) != gDumpList.end()) {
        gDumpStarted = false;
    }
}

void CModulePackage::DumpTestDir(const std::string &aTestDir, const std::list<std::string>& aModules) {
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

void CModulePackage::DumpEAIntro() {
    mExportFile << "<?xml  version='1.0' encoding='windows-1252' ?>\n"
                   "<xmi:XMI xmlns:xmi=\"http://www.omg.org/spec/XMI/20131001\" xmlns:uml=\"http://www.omg.org/spec/UML/20161101\" xmlns:umldi=\"http://www.omg.org/spec/UML/20161101/UMLDI\" xmlns:dc=\"http://www.omg.org/spec/UML/20161101/UMLDC\" xmlns:thecustomprofile=\"http://www.sparxsystems.com/profiles/thecustomprofile/1.0\" xmlns:EAUML=\"http://www.sparxsystems.com/profiles/EAUML/1.0\" xmlns:EAREQ=\"http://www.sparxsystems.com/profiles/EAREQ/1.0\" xmlns:mtt-cpp=\"http://www.sparxsystems.com/profiles/mtt-cpp/1.0\">\n"
                   "<xmi:Documentation exporter=\"Enterprise Architect\" exporterVersion=\"6.5\" exporterID=\"1605\"/>\n"
                   "\t<uml:Model xmi:type=\"uml:Model\" name=\"EA_Model\">" << std::endl;

}



void CModulePackage::DumpEAOutro() {
    mExportFile << 	"\t</uml:Model>\n"
                       "</xmi:XMI>" << std::endl;


}
