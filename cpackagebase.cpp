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

#ifdef WIN32
#include <direct.h>
#endif

#include <string>
#include <map>
#include <list>

#include "helper.h"

#include "cpackagebase.h"
#include "cpackage.h"
#include "csimulationpackage.h"
#include "cjspackage.h"
#include "clibrarypackage.h"
#include "cexecutablepackage.h"
#include "cexecutablewx.h"
#include "cexternpackage.h"
#include "cmodelpackage.h"
#include "cphppackage.h"
#include "chttpifcpackage.h"

#include "csubsystempackage.h"
#include "cmodulepackage.h"

#include "cmodel.h"

#include "cclassbase.h"

#include "path.h"

std::shared_ptr<MPackage> CPackageBase::construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent)
{
    MPackage* retval;

    if (aStereotype) {
        std::string stereotype = helper::tolower(aStereotype->name);

        if (stereotype == "library") {
            retval = new CLibraryPackage(aId, aParent);
        } else if (stereotype == "application") {
            retval = new CExecutablePackage(aId, aParent);
        } else if ((stereotype == "simulation") || (stereotype == "model")) {
            retval = new CSimulationPackage(aId, aParent);
        } else if (stereotype == "jscript") {
            retval = new CJSPackage(aId, aParent);
        } else if ((stereotype == "extern") || (stereotype == "system")) {
            retval = new CExternPackage(aId, aParent);
        } else if (stereotype == "php") {
            retval = new CPHPPackage(aId, aParent);
        } else if (stereotype == "httpifc") {
            retval = new CHttpIfcPackage(aId, aParent);
        } else if (stereotype == "wxapp") {
            retval = new CExecutableWx(aId, aParent);
        } else if (stereotype == "subsystem") {
            retval = new CSubsystemPackage(aId, aParent);
        } else if (stereotype == "module") {
            retval = new CModulePackage(aId, aParent);
        } else {
            retval = new CPackage(aId, aParent);
        }
    } else {
        retval = new CPackage(aId, aParent);
    }
    return (retval->sharedthis<MPackage>());
}

std::shared_ptr<MPackage> CPackageBase::construct(const std::string&aId, const std::string& aPackageType,  std::shared_ptr<MElement> aParent)
{
    MPackage* retval;
    std::string ptype = helper::tolower(aPackageType);

    if (ptype == "umlmodel") {
        retval = new CModelPackage(aId, aParent);
    } else if (ptype == "umlprofile") {
        retval = new CExternPackage(aId, aParent);
    } else {
        retval = new CPackage(aId, aParent);
    }
    return (retval->sharedthis<MPackage>());
}


//
//  This method first creates a backup file before opening the stream
void CPackageBase::OpenStream(std::ofstream&s, const std::string& fname)
{
    int         err=0;
    struct stat dirstat;
    std::string newname;
    //
    //  First check for an existing file and create a backup.
    err=stat(fname.c_str(), &dirstat);
    if (err == 0) {
        newname=fname+".bak";
        err=stat(newname.c_str(), &dirstat);
        if (err == 0) {
            remove(newname.c_str());
        }
        rename(fname.c_str(), newname.c_str());
    }
    s.open(fname);
}

void CPackageBase::DumpMakefileHeader(std::ofstream &s, const std::string& fname)
{
    s << "################################################################################################################\n";
    s << "##\n";
    s << "##  Makefile for "<< fname << "\n";
    s << "##\n";
    s << "##  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n";
    s << "##\n";
    s << "################################################################################################################\n";
}

void CPackageBase::DumpMakefileSource(std::ofstream &s, const std::list<std::string> &modules)
{
    std::list<std::string>::const_iterator i;

    s << "SRC+=";

    for (i=modules.begin(); i!=modules.end(); ++i) {
        s << "\\\n    " << *i << ".cpp";
    }
    s << "\n\n";

}

void CPackageBase::Add(std::shared_ptr<MElement> aElement) {
    MElement::Add(aElement);
    if (aElement->IsClassBased()) {
        Classes.emplace_back(aElement);
    }
    else if (aElement->IsPackageBased()) {
        Packages.emplace_back(aElement);
    }
    else if (aElement->type == eElementType::Dependency) {
        Dependency.emplace_back(aElement);
    }
    else if (aElement->type == eElementType::Object) {
        Objects.emplace_back(aElement);
    }
}

void CPackageBase::DumpMakefileObjects(std::ofstream &s, const std::list<std::string> &modules)
{
    std::list<std::string>::const_iterator i;

    s << "OBJ=$(SRC:.cpp=.o)\n\n";
}

void CPackageBase::PrepareBase(const std::map<std::string, std::string>& tags) {
    std::map<std::string, std::string>::const_iterator i;
    //
    //  Only if no other default value has been set.
    if (Directory.empty()) {
        Directory="./";
    }
    for (i=tags.begin(); i!= tags.end(); ++i) {
        std::string tagname = helper::tolower(i->first);

        if ((tagname == "directory") && (!i->second.empty())) {
            Directory=i->second;
            if (!Directory.empty()) {
                //
                //  Check for relative pathes.
                std::string prefix = Directory.substr(0, 2);

                if ((prefix != "./") && (prefix != "..") && (prefix[0] != '/')) {
                    Directory.insert(0, "./");
                }
            } else {
                //
                //  We found the tag but no content.
                //  Fill it up with the name of the package
                Directory = "./"+name;
            }
            //
            //  The directory always has a content. So we do not check again.
            if (*Directory.rbegin() != '/') {
                Directory.push_back('/');
            }
        } else if ((tagname == "outputname") && (!i->second.empty())) {
            OutputName=i->second;
        } else if ((tagname == "extrainclude") && (!i->second.empty())) {
            ExtraInclude = i->second;
        }
        else if ((tagname == "outputpath") && (!i->second.empty())) {
            OutputPath = i->second;
        } else if ((tagname == "namespace") && (!i->second.empty())) {
            mNameSpace = i->second;
        }
        else if ((tagname == "namespace") && (!i->second.empty())) {
            mNameSpace = i->second;
        }
    }
}

void CPackageBase::DumpBase(std::shared_ptr<CModel> model) {
    int         err=0;
    struct stat dirstat;
    std::string path;
    //
    //  cast the incoming pointer.
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);

    //
    //  First check for directory and create it if needed.
    if (cmodel->pathstack.empty()) {
        path = Directory;
    } else {
        path = cmodel->pathstack.back()+"/"+Directory;
    }
    err = stat(path.c_str(), &dirstat);
    if ((err == -1) && (errno==ENOENT)) {
        helper::mkdir(path.c_str(), 0777);
    } else if ((err == 0) && (!(S_IFDIR & dirstat.st_mode))) {
        std::cerr << "Not a directory " << path << "\n";
        return;
    }
    //
    //  At this point we have a directory to create the generation
    //  results into.
    cmodel->pathstack.push_back(path);
}

std::list<tConnector<MElement, MElement>> CPackageBase::GetLibraryDependency() {
    std::list<tConnector<MElement, MElement>> result;
    std::list<tConnector<MElement, MElement>> more;

    for (auto & di : Supplier) {
        auto db = std::dynamic_pointer_cast<CPackageBase>(di.getElement());

        if (db != nullptr) {
            more = db->GetLibraryDependency() ;

            result.push_back(di);
            if (!more.empty()) {
                result.insert(result.end(), more.begin(), more.end());
            }
        }
    }
    return (result);
}


std::list<std::shared_ptr<MClass>> CPackageBase::GetAllContent(std::list<eElementType> types) {
    std::list<std::shared_ptr<MClass>> result;
    std::list<eElementType>::iterator  eti;

    for (auto & ci : Classes) {
        for (eti = types.begin(); eti != types.end(); ++eti) {
            if (ci->type == *eti) {
                result.push_back(std::dynamic_pointer_cast<MClass>(*ci));
                break;
            }
        }
    }
    for (auto & pi : Packages) {
        if (pi->type == eElementType::Package) {
            //
            //  Add the content of simple packages
            std::list<std::shared_ptr<MClass>> subpackage = std::dynamic_pointer_cast<CPackageBase>(*pi)->GetAllContent(types);

            result.insert(result.end(), subpackage.begin(), subpackage.end());
        } else if (pi->type == eElementType::ModulePackage) {
            //
            //  Module packages combine the content in a single module.
            //  So only a single dummy class will be returned that will contain all content in it.
        }
    }
    return (result);
}

bool CPackageBase::HasCode() {
    bool retval = false;

    for (auto & ci : Classes) {
        if (std::dynamic_pointer_cast<CClassBase>(*ci)->HasSrc()) {
            retval = true;
            break;
        }
    }
    if (!retval) {
        for (auto & pi : Packages) {
            if (pi->type == eElementType::Package) {
                retval = std::dynamic_pointer_cast<CPackageBase>(*pi)->HasCode();
            }
        }
    }
    return retval;
}

std::string CPackageBase::GetNameSpace() const {
    std::string retval;

    if ((parent) && (parent->IsPackageBased())) {
        retval = std::dynamic_pointer_cast<CPackageBase>(*parent)->GetNameSpace();
    }
    if (!mNameSpace.empty()) {
        if (!retval.empty()) {
            retval += NameSpaceDelimiter;
        }
        retval += mNameSpace.getString();
    }
    return retval;
}


std::string CPackageBase::GetExtraHeader() {
    std::string retval;

    if (ExtraInclude.empty()) {
        if (parent) {
            if (parent->IsClassBased()) {
                retval = std::dynamic_pointer_cast<CClassBase>(*parent)->GetExtraHeader();
            } else {
                if (parent->IsPackageBased()) {
                    retval = std::dynamic_pointer_cast<CPackageBase>(*parent)->GetExtraHeader();
                }
            }
        }
    } else {
        retval = ExtraInclude;
    }

    return retval;
}

bool CPackageBase::IsExternPackage() {
    bool retval = false;

    if ((!HasStereotype("Extern")) && (type != eElementType::ExternPackage)) {
        if (parent != nullptr) {
            if (parent->IsClassBased()) {
                retval = std::dynamic_pointer_cast<CClassBase>(*parent)->IsExternClass();
            } else {
                if (parent->IsPackageBased()) {
                    retval = std::dynamic_pointer_cast<CPackageBase>(*parent)->IsExternPackage();
                }
            }
        }
    } else {
        retval = true;
    }

    return retval;
}

std::vector<std::string> CPackageBase::GetNameSpaceList() {
    std::vector<std::string> retval;
    //
    //  Walkup the parent-tree.
    if (parent && (parent->IsPackageBased())) {
        retval = std::dynamic_pointer_cast<CPackageBase>(*parent)->GetNameSpaceList();
    }
    //
    //  We have a namespace to append.
    if (!mNameSpace.empty()) {
        //
        //  Split the own namespace to have correct diffs.
        std::string ns = mNameSpace.getString();
        size_t pos = ns.find("::");

        while ((pos != std::string::npos) || (!ns.empty())) {
            std::string haihappen = ns.substr(0, pos);

            if (!haihappen.empty()) {
                retval.push_back(haihappen);
            }
            if (pos == std::string::npos) {
                break;
            }
            //
            //  This could only be reached if we found a ::
            ns.erase(0, pos+2);
            pos = ns.find("::");
        }
    }

    return retval;
}

std::list<std::string> CPackageBase::GetPathList() {
    std::list<std::string> retval;
    //
    //  Walkup the parent-tree.
    if (parent && (parent->IsPackageBased())) {
        retval = std::dynamic_pointer_cast<CPackageBase>(*parent)->GetPathList();
    }
    //
    //  We have a namespace to append.
    if (!Directory.empty()) {
        retval.push_back(Directory+'/');
    }
    return retval;
}

std::string CPackageBase::GetPathToPackage(std::shared_ptr<MElement> e) {
    std::string retval = "./";

    std::list<std::string> minepath = GetPathList();
    if (e->IsPackageBased()) {
        auto pb = std::dynamic_pointer_cast<CPackageBase>(e);

        std::list<std::string> otherpath = pb->GetPathList();

        while ((!minepath.empty()) && (!otherpath.empty())) {
            if (*minepath.begin() == *otherpath.begin()) {
                minepath.erase(minepath.begin());
                otherpath.erase(otherpath.begin());
            } else {
                break;
            }
        }

        //
        // uppath
        for (auto m : minepath) {
            retval = retval + "../";
        }
        //
        // downpath
        for (auto o : otherpath) {
            retval = retval + o;
        }
    }
    CPath cleaner = retval;

    return (cleaner);
}

std::ofstream &CPackageBase::getExportStream() {
    //
    // We limit the export to the first library package that we can find upwards.
    if (type == eElementType::LibraryPackage) {
        return mExportFile;
    } else {
        if (parent && (parent->IsPackageBased())) {
            return std::dynamic_pointer_cast<CPackageBase>(*parent)->getExportStream();
        }
    }
    return mExportFile;
}

SubsystemFormat CPackageBase::getSubsystemFormat() {
    //
    // We limit the export to the first library package that we can find upwards.
    if ((type == eElementType::LibraryPackage) && (mCreateSubsystem)) {
        return mSubsystemFormat;
    } else {
        if (parent && (parent->IsPackageBased())) {
            return std::dynamic_pointer_cast<CPackageBase>(*parent)->getSubsystemFormat();
        }
    }
    return SubsystemFormat::None;
}

void CPackageBase::DumpEA(std::shared_ptr<CModel> aModel, std::ostream& aExport) {

    aExport << "<packagedElement xmi:type=\"uml:Package\" xmi:id=\"" << id << "\" name=\"" << name << "\">" << std::endl;

    for ( auto o : owned) {
        if (o->IsClassBased()) {
            std::dynamic_pointer_cast<CClassBase>(*o)->DumpEA(aModel, aExport);
        } else {
            if (o->IsPackageBased()) {
                std::dynamic_pointer_cast<CPackageBase>(*o)->DumpEA(aModel, aExport);
            }
        }
    }

    aExport << "</packagedElement>" << std::endl;
}

void CPackageBase::DumpEAExtension(std::shared_ptr<CModel> aModel, std::ostream &aExport) {

}

