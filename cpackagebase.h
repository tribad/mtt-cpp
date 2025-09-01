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
#ifndef CPACKAGEBASE_H
#define CPACKAGEBASE_H

#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <map>

#include "melement.h"
#include "mdependency.h"
#include "mpackage.h"
#include "subsystemformat.h"

class CModel;

enum class ERepositoryType {
    svn,
    git,
    none
};

class CPackageBase : public MPackage
{
public:
    CPackageBase& operator=(CPackageBase&&) = default;
    CPackageBase(const std::string& aId, std::shared_ptr<MElement> e) : MPackage(aId, e) {RepoType = ERepositoryType::none;}
    virtual ~CPackageBase() = default;
    static std::shared_ptr<MPackage> construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype = nullptr, std::shared_ptr<MElement> aParent = nullptr);
    static std::shared_ptr<MPackage> construct(const std::string&aId, const std::string& aPackageType, std::shared_ptr<MElement> aParent = nullptr);

    void OpenStream(std::ofstream&s, const std::string& fname);
    void DumpMakefileHeader(std::ofstream&, const std::string& fname);
    void DumpMakefileSource(std::ofstream& s, const std::list<std::string>& modules);
    void DumpMakefileObjects(std::ofstream& s, const std::list<std::string>& modules);
    void PrepareBase(const std::map<std::string, std::string>& tags);
    void DumpBase(std::shared_ptr<CModel> model);
    virtual std::list<tConnector<MElement, MElement>> GetLibraryDependency();
    std::list<std::string> GetPathList();
    std::string GetPathToPackage(std::shared_ptr<MElement> e);
    std::list<std::shared_ptr<MClass>> GetAllContent(std::list<eElementType> types);
    bool HasCode();
    bool hasNameSpace() const {return !mNameSpace.empty();}
    std::string GetNameSpace() const;
    std::vector<std::string> GetNameSpaceList();
    std::string GetExtraHeader();

    bool IsExternPackage();
    void Add(std::shared_ptr<MElement> e) override;
    //
    //  Subsystem handling.
    SubsystemFormat getSubsystemFormat();
    std::ofstream& getExportStream();
    void DumpEA(std::shared_ptr<CModel> aModel, std::ostream& aExport);
    void DumpEAExtension(std::shared_ptr<CModel> aModel, std::ostream& aExport);
public:
    ERepositoryType RepoType;
    std::string     Directory;                 //  Directory where generated content is put. For extern packages its the include path.
    std::string     NameSpaceDelimiter = "::"; //  This is the delimiter used for namespaces.
    std::string     OutputName;                //  Name of the build result from the 'Directory' content
    std::string     OutputPath;                //  Directory where the build result can be found.
    std::string     ExtraInclude;              //  Special include files that must be included in every module
    std::ofstream   makefile;
    bool            mCreateSubsystem = false;
    SubsystemFormat mSubsystemFormat = SubsystemFormat::EAXMI;
    std::ofstream   mExportFile;
    bool            m_init_done = false;       //  Prevent double initialization.
    std::string     m_cxxstandard = "c++17";
};

#endif // CPACKAGEBASE_H
