//
// Copyright 2020 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include "helper.h"
#include "mdependency.h"
#include "cdependency.h"

#include "cpackagebase.h"
#include "chttpifcpackage.h"

#include "cexternpackage.h"

#include "cclassbase.h"

#include "mmodel.h"
#include "cmodel.h"

std::string CHttpIfcPackage::FQN() const {
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

void CHttpIfcPackage::Prepare(void) {
    //
    //  Set the defaults for library packages.
    OutputName = helper::normalize(name);
    Directory  = "/"+name;
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


void CHttpIfcPackage::Dump(std::shared_ptr<MModel> model) {
    std::string                               path;
    std::list<std::string>                    modules;
    std::vector<MClass*>::iterator            i;
    std::list<eElementType>                   contenttypes;
    std::list<std::shared_ptr<MClass>>        content;
    std::list<tConnector<MElement, MElement>> liblist;

    auto cmodel = std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel);
    path=cmodel->pathstack.back()+"/"+".Makefile";
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
    contenttypes.push_back(eElementType::HtmlPageClass);
    content = GetAllContent(contenttypes);
    for (auto & ci : content) {
        //
        //  If the class is in a different package it should be one of the subpackages.
        if ( ci->parent && (ci->parent != sharedthis<MElement>()) && (ci->parent->IsPackageBased())) {
            std::string pathto = GetPathToPackage(ci->parent);

            if ((ci->type == eElementType::SimObject) || (ci->type == eElementType::CxxClass) ||
                    (ci->type == eElementType::CClass) || (ci->type == eElementType::HtmlPageClass)) {
                modules.push_back("./" + pathto + "/" + ci->name);
            }
            //
            //  Setup the directory for the subpackage.
            std::dynamic_pointer_cast<CPackageBase>(*ci->parent)->DumpBase(cmodel);
            ci->Dump(model);
            //
            //  Remove last element from path-stack. Was for the subpackage.
            cmodel->pathstack.pop_back();
        } else {
            if ((ci->type == eElementType::SimObject) || (ci->type == eElementType::CxxClass) ||
                    (ci->type == eElementType::CClass) || (ci->type == eElementType::HtmlPageClass)) {
                modules.push_back(ci->name);
            }
            ci->Dump(model);
        }
    }
    //
    //  Before dumping the modeled classes dump the interface source.
    std::string httpifc = cmodel->pathstack.back()+"/";

    DumpHttpIfc(httpifc, modules);
    cmodel->generatedfiles.push_back(tGenFile {httpifc+".__httpifc.cpp", id, "//", "src"});
    cmodel->generatedfiles.push_back(tGenFile {httpifc+".__httpifc.h", id, "//", "hdr"});
    //
    //  Dump the makefile
    makefile << "PROJ=" << OutputName+"\n\n";
    DumpMakefileSource(makefile, modules);
    DumpMakefileObjects(makefile, modules);
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

    makefile << "STATICLIBS+=\\\n";
    if (!cmodel->AppCoreVersion.empty()) {
        makefile << "    -lhttpifc-" << cmodel->AppCoreVersion << "\\\n";
        makefile << "    -lhtmlbase-" << cmodel->AppCoreVersion << "\\\n";
    }

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

    makefile << "LIBPATH+=\\\n";
    if (HasTaggedValue("LibPath")) {
        makefile << "    -L" << GetTaggedValue("LibPath") << "\\\n";
    }

    if (!cmodel->AppCoreVersion.empty()) {
        makefile << "     -L/usr/local/lib64/" << "\\\n     -L/usr/lib64/";
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
        else {

        }
    }
    makefile << "\n\n";

    makefile << "INCLUDEPATH+=\\\n";
    //
    // prepend the appcore header pathes.
    if (!cmodel->AppCoreVersion.empty()) {
        makefile << "     -I/usr/local/include/appcore/" << cmodel->AppCoreVersion << "\\\n     -I/usr/include/appcore/" << cmodel->AppCoreVersion;
    }

    for (auto& di : liblist) {
        if (di.getElement()) {
            if (di.getElement()->type == eElementType::ExternPackage) {
                std::string dir = std::dynamic_pointer_cast<CPackageBase>(di.getElement())->Directory;

                if (dir != "/.") {
                    makefile << "\\\n" << "     -I" << dir;
                }
            }
        }
        else {

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
        makefile << "endif\n";
        makefile << "\n\n";

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
        makefile << "\tcp lib$(PROJ).so lib$(PROJ).so.$(VERSION).$(REVISIONNR).$(BUILDNR)\n";
        makefile << "\t$(shell echo $$(( $(BUILDNR) +1 )) > BUILD)\n\n";

        if (!modules.empty()) {
            makefile << "depend : $(SRC)\n";
            makefile << "\tg++ $(CXXFLAGS) -M $(SRC) > depend\n\n";
        }

        makefile << "clean:\n";
        makefile << "\trm -f lib$(PROJ).a\n";
        makefile << "\trm -f lib$(PROJ).so*\n";
        makefile << "\trm -f depend\n";
        makefile << "\trm -f $(OBJ)\n\n";

        makefile << "-include depend\n\n";
    } else {
        makefile << "all :\n";
        makefile << "\t$(info No modules to compile)\n";
    }
    makefile.close();
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
}

std::list<std::shared_ptr<MElement>> CHttpIfcPackage::CollectPages(std::shared_ptr<MPackage> aPack) {
    std::list<std::shared_ptr<MElement>> retval;
    auto pack = aPack;

    if (pack) {
        for (auto & cc : pack->Classes) {
            if ((cc->type == eElementType::HtmlPageClass) && (cc->HasTaggedValue("URI"))) {
                retval.emplace_back(cc);
            }
        }
        for (auto & pp : pack->Packages) {
            //
            //  We check only untyped packages.
            if (pp->type == eElementType::Package) {
                std::list<std::shared_ptr<MElement>> more = CollectPages(std::dynamic_pointer_cast<MPackage>(*pp));
                retval.insert(retval.end(), more.begin(), more.end());
            }
        }
    }
    return retval;
}

void CHttpIfcPackage::DumpHttpIfc(const std::string& aPath, std::list<std::string>& aModulelist) {
    std::ofstream ifc(aPath+".__httpifc.cpp");
    std::list<std::shared_ptr<MElement>> htmlpages = CollectPages(sharedthis<MPackage>());

    ifc <<  "// *************************************************************************************************************\n"
            "//\n"
            "//  Modul-Name     : __httpifc.cpp\n"
            "//\n"
            "//  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n"
            "//\n"
            "// *************************************************************************************************************\n"
            "#include <httpcontentifc.h>\n"
            "#include <map>\n"
            "#include <string>\n"
            "#include <libxml/tree.h>\n"
            "#include <httprequest.h>\n"
            "#include <httpresponse.h>\n"
            "#include <msg.h>\n"
            "#include <vector>\n"
            "#include <stdint.h>\n"
            "#include <httpsimulationifc.h>\n"
            "#include <CHtmlPage.h>\n"
            "#include \"__httpifc.h\"\n\n";
    std::set<std::string> doneincludes;
    std::set<std::shared_ptr<MElement>>   donelist;

    doneincludes.insert("CHtmlPage.h");

    for (auto & h : htmlpages) {
        if (h->IsClassBased()) {
            std::dynamic_pointer_cast<CClassBase>(h)->DumpExtraIncludes(ifc, doneincludes, donelist);
        }
    }

    for (auto & h : htmlpages) {
        if (h->IsClassBased()) {
            std::dynamic_pointer_cast<CClassBase>(h)->DumpNeededIncludes(ifc, std::dynamic_pointer_cast<CClassBase>(h), doneincludes, donelist);
        }
    }
    ifc <<
            "std::map<std::string, CHtmlPage*> CIfc::pagemap;\n\n"
            "CIfc::CIfc(xmlNode* param)  {\n"
            "    (void)param;\n";
        for (auto & h : htmlpages) {
            ifc << "    pagemap.insert(std::pair<std::string, CHtmlPage*>(\"" << h->GetTaggedValue("URI") << "\", new " << h->name << "));\n";
        }
    ifc <<  "}\n\n";
    ifc <<
            "CIfc::~CIfc()  {\n"
            "}\n\n"
            "bool CIfc::DoYouHandleURI(const char* uri) {\n"
            "    bool retval = false;\n"
            "    std::map<std::string, CHtmlPage*>::iterator pi=CIfc::pagemap.find(uri);\n\n"

            "    if (pi != CIfc::pagemap.end()) {\n"
            "        retval = true;\n"
            "    }\n"
            "    return  (retval);\n"
            "}\n\n"
            "tHttpResponse* CIfc::HandleURI(tHttpRequest* req) {\n"
            "    tHttpResponse* retval = 0;\n"
            "    std::map<std::string, CHtmlPage*>::iterator pi=CIfc::pagemap.find(req->uri);\n\n"

            "    if (pi != CIfc::pagemap.end()) {\n"
            "        tHttpResponse* response;\n"
            "\n"
            "        RequestCount++;\n"
            "        response = pi->second->HandleRequest(req);\n"
            "        if (response != 0) {\n"
            "            ResponseCount++;\n"
            "        }\n"
            "        retval = response;\n"
            "    }\n"
            "    return  (retval);\n"
            "}\n"
            "\n"
            "tHttpResponse* CIfc::Process(tHttpRequest* req, std::shared_ptr<tMsg> msg) {\n"
            "    tHttpResponse* retval = 0;\n"
            "    std::map<std::string, CHtmlPage*>::iterator pi=CIfc::pagemap.find(req->uri);\n"
            "\n"
            "    if (pi != CIfc::pagemap.end()) {\n"
            "        tHttpResponse* response;\n"
            "\n"
            "        response = pi->second->Process(req, msg);\n"
            "        if (response != 0) {\n"
            "            ResponseCount++;\n"
            "        }\n"
            "        retval = response;\n"
            "    }\n"
            "    return  (retval);\n"
            "}\n\n"

            "CHttpSimulationIfc* ifc;\n\n"
            "extern \"C\" CHttpContentIfc* getcontentifc(xmlNode* param, CHttpSimulationIfc* simifc) {\n"
            "    CHttpContentIfc* retval = 0;\n\n"
            "    ifc    = simifc;\n"
            "    retval = new CIfc(param);\n\n"
            "    return  (retval);\n"
            "}\n";

    aModulelist.emplace_back("__httpifc");

    std::ofstream hdr(aPath + ".__httpifc.h");

    hdr <<
           "// *************************************************************************************************************\n"
           "//\n"
           "//  Modul-Name     : __httpifc.h\n"
           "//\n"
           "//  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n"
           "//\n"
           "// *************************************************************************************************************\n"
           "#pragma once\n"
           "#ifndef __HTTPIFC_INC\n"
           "#define __HTTPIFC_INC\n"
           "//\n"
           "//  Forwards\n"
           "class CHtmlPage;\n"
           "class tHttpRequest;\n"
           "class tHttpResponse;\n"
           "//\n"
           "//  This is the class\n"
           "class CIfc : public CHttpContentIfc {\n"
           "public:\n"
           "    CIfc(xmlNode* param) ;\n"
           "    virtual ~CIfc() ;\n"
           "    virtual bool DoYouHandleURI(const char* uri) ;\n"
           "    virtual tHttpResponse* HandleURI(tHttpRequest* req) ;\n"
           "    virtual tHttpResponse* Process(tHttpRequest* req, std::shared_ptr<tMsg> msg) ;\n"
           "public:\n"
           "    static std::map<std::string, CHtmlPage*> pagemap;\n"
           "};\n"
           "\n"
           "#endif  // __HTTPIFC_INC\n";

}
