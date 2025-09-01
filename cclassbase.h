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
#ifndef CCLASSBASE_H
#define CCLASSBASE_H

#include <string>
#include <map>
#include <list>
#include <set>
#include <fstream>
#include <memory>

#include "HeaderList.h"

#include "mclass.h"

#include "cgeneralization.h"

#include "mmodel.h"
#include "cmodel.h"

#include "namespace.h"

#include "typenode.h"

class CModel;
class MElement;
class CAttribute;
class CAssociationEnd;
class CPackageBase;

class CClassBase : public MClass
{
public:
    CClassBase() = default;
    CClassBase(std::string aId, std::shared_ptr<MElement> e) : MClass(aId, e) {}
    static std::shared_ptr<MClass> construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype , std::shared_ptr<MElement> aParent);
    static std::shared_ptr<MClass> construct(const std::string&aId, const std::string& aPackageType, std::shared_ptr<MElement>aParent);
    virtual ~CClassBase() = default;

    virtual void SetFromTags(const std::string& name, const std::string&value) = 0;
    virtual std::string getHeaderFile();
    virtual std::string getFileName();

    std::shared_ptr<CClassBase> shared_this() {return std::dynamic_pointer_cast<CClassBase>(MClass::Instances[id]);}

    std::string GetExtraHeader(void) ;
    void OpenStream(std::ofstream& s, const std::string& fname);
    void CloseStreams(void);
    void DumpFileHeader(std::ofstream& s, const std::string& fname, const std::string& aExt, const std::string& name = "Hans-Juergen Lange", const std::string& email = "hjl@simulated-universe.de");
    void DumpGuardHead(std::ofstream& s, const std::string& name, std::string aNameSpace = "");
    void DumpGuardTail(std::ofstream& s, const std::string& name, std::string aNameSpace = "");
    void DumpFunctionHeader(std::ofstream& s, const std::string& name, std::string author, const std::string& creationdate);

    void DumpExtraIncludes(std::ostream& src, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>>& aDoneList);
    void DumpExtraIncludes(const std::string& aHeaders, std::ostream& src, std::set<std::string>& aDoneIncludes, bool aCheckModel = true, const std::string& aPath = "");

    void DumpInModelIncludes(std::ostream& src, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>>& aDoneList);
    void DumpInModelIncludes(const std::string& aHeaders, std::ostream& src, std::set<std::string>& aDoneIncludes, bool aCheckModel = true, const std::string& aPath = "");

    void DumpHeaderIncludes(std::ostream& src, std::shared_ptr<CClassBase> aBase, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>>& aDoneList);
    void DumpNeededIncludes(std::ostream& src, std::shared_ptr<CClassBase> aBase, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>>& aDoneList);
    void DumpOptionalIncludes(std::ostream& src, std::shared_ptr<CClassBase> aBase, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>>& aDoneList);
    bool InModelHeader(const std::string& aHeader);
    void DumpMessageIncludes(std::ostream& src, std::set<std::string>& aDoneList, std::set<std::string> &aIncludesDone);
    void DumpSystemHeader(std::ostream& hdr);

    void PrepareBase();
    void DumpBase(std::shared_ptr<CModel> model, const std::string& name);
    const std::list<std::shared_ptr<MElement>>& GetNeededHeader(void) {return (neededmodelheader.mHeaderList);}
    void IndentIn() {indentation+=4;indent.clear();indent.assign(indentation, ' ');}
    void IndentOut() {indentation-=4;indent.clear();if (indentation > 0) indent.assign(indentation, ' ');}
    void DumpPublicMacros(std::ofstream& hdr);
    void DumpPrivateMacros(std::ofstream& src);

    std::vector<std::shared_ptr<MAttribute> > GetImportAttributes();
    std::vector<std::shared_ptr<MAttribute>> GetDerivedAttributes() ;
    std::vector<std::shared_ptr<MAssociationEnd>> GetImportAssociationEnds();
    std::vector<std::shared_ptr<MAssociationEnd>> GetDerivedAssociationEnds() ;


    std::vector <std::shared_ptr<MMessage>> GetImportIncoming();
    std::vector <std::shared_ptr<MMessage>> GetImportOutgoing();

    std::vector <std::shared_ptr<MOperation>> GetImportOperation();
    std::vector <std::shared_ptr<MElement>>   GetImportSupplier();

    std::list<std::shared_ptr<MAttribute>> GetAttributes();
    std::list<std::shared_ptr<MAssociationEnd>> GetAssociationEnds();

    std::list<std::shared_ptr<CClassBase>> getEnclosedClasses();
    std::shared_ptr<CClassBase> getContainerClass() {
        std::shared_ptr<CClassBase>  retval = shared_this();

        while (retval->parent->IsClassBased()) {
            retval = std::dynamic_pointer_cast<CClassBase>(*retval->parent);
        }
        return retval;
    }

    void CollectAttributes();

    std::string MkType(std::shared_ptr<CAttribute> attr);
    std::string MkType(std::shared_ptr<CAssociationEnd> attr);
    bool IsVariantType(const std::string &aClassifierName);
    bool HasSrc() {return (!has_src.empty());}
    bool HasHdr() {return (!has_hdr.empty());}

    bool IsExternClass();
    bool IsExternInModel();
    //
    // Template class specifics.
    bool isTemplateClass() {return !mClassParameter.empty();}
    std::string GetPathTo(std::shared_ptr<MElement> e);
    std::shared_ptr<CPackageBase> GetPackage();
    std::shared_ptr<CPackageBase> GetPathEffectivePackage();
    std::string GetNameSpace();
    //
    //  Get the types from the mTypeTree.
    std::list<std::shared_ptr<CClassBase>> getTypes();
    std::list<std::shared_ptr<CClassBase>> getRefTypes();
    //
    //  Qt specific methods.
    bool IsQtDerived();
    //
    //
    //static void prepTypeTree(TypeNode& aNode);
    static std::list<std::shared_ptr<CClassBase>> getTypes(const TypeNode& aNode);
    static std::list<std::shared_ptr<CClassBase>> getRefTypes(const TypeNode& aNode);

    void DumpEA(std::shared_ptr<CModel> aModel, std::ostream& aExport);
    void DumpEAExtension(std::shared_ptr<CModel> aModel, std::ostream& aExport);

    //
    //  Namespaces
    inline void DumpNameSpaceIntro(std::ostream& file);
    inline void DumpNameSpaceClosing(std::ostream& file);
    //
    //  Template parameter specific
    bool treatAsReference(int aPos);
    void DumpTemplatePrefix(std::ostream & file);
    void DumpTemplateParameter(std::ostream & file);
public:
    bool                                               PrepDone = false;
    std::list<tConnector<CClassBase, CGeneralization>> Base;
    std::list<tConnector<CClassBase, CGeneralization>> Derived;
    std::vector<std::shared_ptr<CAttribute>>           allAttr;
    std::vector<std::shared_ptr<CAssociationEnd>>      allEnds;
    std::string                                        mInModelHeader;
    bool                                               mHasInModelHeader = false;
    bool                                               mExport = false;
public:
    std::set<std::string>                              donelist;
    HeaderList                             neededmodelheader;
    std::set<std::string>                  optionaldonelist;
    HeaderList                             optionalmodelheader;
    std::list<std::shared_ptr<CClassBase>> extramodelheader;
    std::set<std::shared_ptr<CClassBase>>  forwards;
    std::string                            extraheader;
    std::string                            mSystemHeader;
    std::string                            has_src;
    std::string                            has_hdr;
    std::ofstream                          src;
    std::ofstream                          hdr;
    std::list<std::ofstream>               more;
    std::list<std::pair<int, std::string>> mTreatAsReference;
protected:
    int                                    indentation = 0;
    std::string                            indent;
};


inline void CClassBase::DumpNameSpaceIntro(std::ostream &file) {
    for (const auto & ni : mNameSpace.get()) {

        file << "namespace " << ni << std::endl << "{" << std::endl;
    }
}

inline void CClassBase::DumpNameSpaceClosing(std::ostream &file) {
    if (!mNameSpace.get().empty()) {
        file << std::endl;
        for (auto ni = mNameSpace.get().rbegin(); ni != mNameSpace.get().rend(); ni++) {
            file << "} // namespace - " << *ni << std::endl;
        }
    }
}


#endif // CCLASSBASE_H
