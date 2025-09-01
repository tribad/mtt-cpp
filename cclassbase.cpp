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
#include <string.h>
#ifdef __linux__
#include <unistd.h>
#endif

#include <helper.h>

#include "systemtime.h"
#include "main.h"

#include "melement.h"
#include "mparameter.h"
#include "cparameter.h"

#include "cclassbase.h"
#include "csimobjectv2.h"
#include "csimenumeration.h"
#include "ccclass.h"
#include "cclass.h"
#include "ccxxclass.h"
#include "cenumeration.h"
#include "cstruct.h"
#include "csimstruct.h"
#include "cmessageclass.h"
#include "csignalclass.h"
#include "cunion.h"
#include "cqtclass.h"
#include "cphpclass.h"
#include "cdatatype.h"
#include "cexternclass.h"
#include "cjsclass.h"
#include "chtmlpageclass.h"
#include "cwxformsclass.h"
#include "cprimitivetype.h"
#include "ccollaboration.h"
#include "cattribute.h"
#include "cassociationend.h"
#include "cgeneralization.h"
#include "cmodel.h"
#include "cpackagebase.h"
#include "cdependency.h"
#include "cmoduleclass.h"
#include "cinterface.h"

#include "msimmessage.h"
#include "csimmessage.h"

#include "msimsignal.h"
#include "csimsignal.h"

#include "mjsonmessage.h"
#include "cjsonmessage.h"

#include "path.h"

extern long simversion;
extern bool doDump;
extern bool dumpHelp;



const std::string cModulTag     = "$modul$";
const std::string cExtensionTag = "$extension$";
const std::string cAuthorTag    = "$author$";
const std::string cYearTag      = "$year$";
const std::string cMailTag      = "$email$";

std::shared_ptr<MClass> CClassBase::construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent)
{

    std::shared_ptr<MClass>     newclass;
    std::string stype;

    if (aStereotype) {
        stype = aStereotype->name;
        newclass = CClassBase::construct(aId, stype, aParent);
        newclass->stereotypes.insert(std::pair<std::string, std::shared_ptr<MStereotype>>(helper::tolower(stype), aStereotype));
    } else {
        newclass = CClassBase::construct(aId, stype, aParent);
    }
    return (newclass);
}

std::shared_ptr<MClass> CClassBase::construct(const std::string&aId, const std::string& aClassType,  std::shared_ptr<MElement> aParent) {
    MClass*     newclass;
    std::string classtype = helper::tolower(aClassType);

    if ((classtype == "simobject") || (classtype == "modelitem")) {
        newclass = new CSimObjectV2(aId, aParent);
    } else if ((classtype == "simenumerator") || (classtype == "modelenum")) {
        newclass = new CSimEnumeration(aId, aParent);
    } else if (classtype == "cxx") {
        newclass = new CCxxClass(aId, aParent);
    } else if (classtype == "c") {
        newclass = new CCClass(aId, aParent);
    } else if (classtype == "enumeration") {
        newclass = new CEnumeration(aId, aParent);
    } else if (classtype == "struct") {
        newclass = new CStruct(aId, aParent);
    } else if (classtype == "simstruct") {
        newclass = new CSimStruct(aId, aParent);
    } else if (classtype == "simmessage") {
        newclass = new CMessageClass(aId, aParent);
    } else if ((classtype == "simsignal") || (classtype == "json") || (classtype == "tlv") || (classtype == "protobuf") || (classtype == "signal") )  {
        newclass = new CSignalClass(aId, aParent);
    } else if (classtype == "union") {
        newclass = new CUnion(aId, aParent);
    } else if (classtype == "qt") {
        newclass = new CQtClass(aId, aParent);
    } else if (classtype == "extern") {
        newclass = new CExternClass(aId, aParent);
    } else if (classtype == "jscript") {
        newclass = new CJSClass(aId, aParent);
    } else if (classtype == "primitivetype") {
        newclass = new CPrimitiveType(aId, aParent);
    } else if (classtype == "collaboration") {
        newclass = new CCollaboration(aId, aParent);
    } else if (classtype == "php") {
        newclass = new CPHPClass(aId, aParent);
    } else if (classtype == "htmlpage") {
        newclass = new CHtmlPageClass(aId, aParent);
    } else if (classtype == "wxform") {
        newclass = new CWxFormsClass(aId, aParent);
    } else if (classtype == "dataType") {
        newclass = new CDataType(aId, aParent);
    } else if (classtype == "interface") {
        newclass = new CInterface(aId, aParent);
    } else if (classtype == "moduleclass") {
        newclass = new CModuleClass(aId, aParent);
    } else {
        newclass = new CClass(aId, aParent);
    }
    return (newclass->sharedthis<MClass>());
}


void CClassBase::PrepareBase() {
    if (!PrepDone) {
        //
        //  Set it at once here so that we cannot recurse into it from some dependencies
        //  and the like.
        PrepDone = true;

        std::shared_ptr<CPackageBase> pp = GetPackage();

        if (pp) {
            mNameSpace = pp->GetNameSpaceList();

            std::string fqn = mNameSpace.getString();

            if (!fqn.empty()) {
                fqn += "::" + name;

                auto exists = mByFQN.find(fqn);

                if (exists == mByFQN.end()) {
                    mByFQN.insert(std::pair<std::string, std::shared_ptr<CClassBase>>(fqn, sharedthis<CClassBase>()));
                } else {
                    //std::cerr << "Class " << fqn << " already exists.\n";
                }
            }
        }

        if ( HasTaggedValue("ExtraInclude")) {
            extraheader = GetTaggedValue("ExtraInclude");
        } else {
        }
        if ( HasTaggedValue("SystemHeader")) {
            mSystemHeader = GetTaggedValue("SystemHeader");
        } else {
        }
        if (HasTaggedValue("InModelHeader")) {
            mHasInModelHeader = true;
            mInModelHeader    = GetTaggedValue("InModelHeader");
        }
        if (HasTaggedValue("Export")) {
            mExport = true;
        }
        if (HasTaggedValue("TreatParameterAsReference")) {
            std::string references = GetTaggedValue("TreatParameterAsReference");
            //
            //  This is a list of formal parameters by name, somehow delimited.
            int state = 0;
            std::string pname;

            for (auto & c : references) {
                switch (state) {
                case 0:
                    if (!isblank(c) && (c != ',')) {
                        pname.push_back(c);
                        state = 1;
                    } else {
                    }
                    break;
                case 1:
                    if (isblank(c) || (c == ',')) {
                        int position = -1;

                        for (auto & cp : mClassParameter) {
                            if (cp.second->name == pname) {
                                position = cp.first;
                                break;
                            }
                        }
                        if (position != -1) {
                            mTreatAsReference.emplace_back(position,pname);
                        } else {
                            std::cerr << "Class Parameter with name : '" << pname << "' is not defined in the class '" << name << "'" << std::endl;
                        }
                        pname.clear();
                        state = 0;
                    } else {
                        pname.push_back(c);
                    }
                    break;
                default:
                    break;
                }
            }
            if (!pname.empty()) {
                int position = -1;

                for (auto & cp : mClassParameter) {
                    if (cp.second->name == pname) {
                        position = cp.first;
                        break;
                    }
                }
                if (position != -1) {
                    mTreatAsReference.emplace_back(position,pname);
                } else {
                    std::cerr << "Class Parameter with name :" << pname << " is not defined in the class '" << name << "'" << std::endl;
                }
                pname.clear();
            }
        }
        std::map<std::string, std::string>::const_iterator i;
        for (i=tags.begin(); i!= tags.end(); ++i) {
            SetFromTags(i->first, i->second);
        }
        //
        //  prep the generalizations.
        for (auto & gi : Generalization) {
            auto g = std::dynamic_pointer_cast<CGeneralization>(*gi);

            g->Prepare();
            if (g->base && g->derived) {

                auto base = std::dynamic_pointer_cast<CClassBase>(*g->base);
                auto derived = std::dynamic_pointer_cast<CClassBase>(*g->derived);


                derived->Base.emplace_back(base, g);
                base->Derived.emplace_back(derived, g);
            }
        }
        //
        //  We may store the dependencies on model-level.
        std::vector<std::shared_ptr<MOperation>> io = GetImportOperation();

        for (auto & operation : io) {
            operation->Prepare();
        }
    }
}
//
//  This method first creates a backup file before opening the stream
void CClassBase::OpenStream(std::ofstream& s, const std::string& fname)
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

void CClassBase::CloseStreams(void)
{
    if (src.is_open()) {
        src.close();
    }
    if (hdr.is_open()) {
        hdr.close();
    }
    for (auto & morestreams : more)
    {
        if (morestreams.is_open()) {
            morestreams.close();
        }
    }
}

void CClassBase::DumpFileHeader(std::ofstream &s, const std::string& fname, const std::string& aExt, const std::string& aName, const std::string& aEmail)
{
    std::string output = gCxxFileHeader;
    size_t      position;

    if (aExt == ".h") {
        output = gCxxHFileHeader;
    }
    if (aExt == ".hpp") {
        output = gCxxHppFileHeader;
    }
    if (aExt == ".cpp") {
        output = gCxxCppFileHeader;
    }

    position = output.find(cModulTag);
    if (position != std::string::npos) {
        output.replace(position, cModulTag.size(), fname);
    }
    position = output.find(cExtensionTag);
    if (position != std::string::npos) {
        output.replace(position, cExtensionTag.size(), aExt);
    }
    position = output.find(cAuthorTag);
    if (position != std::string::npos) {
        output.replace(position, cAuthorTag.size(), aName);
    }
    position = output.find(cYearTag);
    if (position != std::string::npos) {
        SystemTime now;
        output.replace(position, cYearTag.size(), now.Format("%Y"));
    }

    position = output.find(cMailTag);
    if (position != std::string::npos) {
        output.replace(position, cMailTag.size(), aEmail);
    }
    s << output;
}

void CClassBase::DumpGuardHead(std::ofstream &s, const std::string& name, std::string aNameSpace)
{
    std::string guard;

    if (gUseNameSpaceInGuards) {
        size_t position;
        std::string ns = GetNameSpace();

        while ((position = ns.find("::")) != std::string::npos) {
            ns.replace(position, 2, "_");
        }
        guard = helper::toupper(ns);
        if (!ns.empty()) {
            guard += "_";
        }
    }

    guard += helper::toupper(mTypeTree.mName);

    guard += gHeaderGuardExtension;


    if (gUsePragmaOnce) {
        s << "#pragma once\n";
    }
    s << "#ifndef " << guard << "\n";
    s << "#define " << guard << "\n";
}

void CClassBase::DumpGuardTail(std::ofstream &s, const std::string& name, std::string aNameSpace)
{
    std::string guard;

    if (gUseNameSpaceInGuards) {
        size_t position;
        std::string ns = GetNameSpace();

        while ((position = ns.find("::")) != std::string::npos) {
            ns.replace(position, 2, "_");
        }
        guard = helper::toupper(ns);
        if (!ns.empty()) {
            guard += "_";
        }
    }

    guard += helper::toupper(mTypeTree.mName);

    guard += gHeaderGuardExtension;

    s << "\n#endif  // " << guard << "\n";
}

void CClassBase::DumpFunctionHeader(std::ofstream &s, const std::string& name, std::string author, const std::string& creationdate)
{
    s << "// FH ************************************************************************\n";
    s << "//\n";
    s << "//  Method-Name   : "<< name << "()\n";
    if (!author.empty()) {
        s << "//  Author        : "<< author << "\n";
    }
    if (!creationdate.empty()) {
        s << "//  Creation-Date : " << creationdate << "\n";
    }
    s << "//\n";
    s << "// **************************************************************************\n";

}


void CClassBase::DumpBase(std::shared_ptr<CModel> model, const std::string& name) {
    std::string path;
    //
    //  cast the incoming pointer.
    std::shared_ptr<CModel> cmodel = model;
    //
    //  open the source output stream if needed.
    if (!has_src.empty())
    {
        path=cmodel->pathstack.back()+"/."+getFileName()+has_src;
        cmodel->generatedfiles.push_back( tGenFile {path, id, "//", "src"});
        OpenStream(src, path);
    }
    //
    //  open the header output stream if needed.
    if (!has_hdr.empty())
    {
        path=cmodel->pathstack.back()+"/."+getFileName()+has_hdr;
        cmodel->generatedfiles.push_back(  tGenFile { path, id, "//", "hdr"} );
        OpenStream(hdr, path);
    }
    //
    //  At this point we have a directory to create the generation
    //  results into.
}

void CClassBase::DumpPublicMacros(std::ofstream &hdr) {
    size_t                                       msize = 0;
    std::map<std::string, std::string>           macrolist;

    for (auto & attr : Attribute) {
        std::shared_ptr<CAttribute> a = std::dynamic_pointer_cast<CAttribute>(*attr);

        if (!a->Classifier && a->ClassifierName.empty() && (a->visibility==vPublic)) {
            macrolist.insert(std::pair<std::string, std::string>(a->name, a->defaultValue ));
        }
    }
    //
    //  Search the max size of the macro name;
    for (auto & mi : macrolist) {
        if (mi.first.size() > msize) {
            msize = mi.first.size();
        }
    }
    if (!macrolist.empty()) {
        hdr << "//\n//  This is a list of public macros\n";
    }
    for (auto & mi : macrolist) {
        std::string filler;

        filler.assign(msize - mi.first.size()+1, ' ');
        hdr << "#define " << mi.first << filler << mi.second << "\n";
    }
}

void CClassBase::DumpPrivateMacros(std::ofstream &src) {
    size_t                                       msize = 0;
    std::map<std::string, std::string>           macrolist;

    for (auto & attr : Attribute) {
        std::shared_ptr<CAttribute> a = std::dynamic_pointer_cast<CAttribute>(*attr);

        if (!a->Classifier && a->ClassifierName.empty() && (a->visibility == vPrivate)) {
            macrolist.insert(std::pair<std::string, std::string>(a->name, a->defaultValue ));
        }
    }
    //
    //  Search the max size of the macro name;
    for (auto & mi : macrolist) {
        if (mi.first.size()> msize) {
            msize = mi.first.size();
        }
    }
    if (!macrolist.empty()) {
        hdr << "//\n//  This is a list of private macros\n";
    }

    for (auto & mi : macrolist) {
        std::string filler;

        filler.assign(msize - mi.first.size()+1, ' ');
        src << "#define " << mi.first << filler << mi.second << "\n";
    }
}

std::list<std::shared_ptr<MAttribute>> CClassBase::GetAttributes() {
    std::list<std::shared_ptr<MAttribute>> retval;

    for (auto & attr : Attribute) {
        retval.push_back(std::dynamic_pointer_cast<MAttribute>(*attr));
    }
    /*
     * Process the import dependencies.
     */
    for (auto& se : Supplier) {
        if (se.getConnector()->HasStereotype("import")) {
            /*
            * Because we already that one end of the dependency is us.
            * We only check if any end of the dependency is a supplier.
            */
            std::list<std::shared_ptr<MAttribute>> moreattributes = (std::dynamic_pointer_cast<CClassBase>(se.getElement()))->GetAttributes();
            retval.insert(retval.end(), moreattributes.begin(), moreattributes.end());
        }
    }
    return retval;
}

std::vector<std::shared_ptr<MAttribute>> CClassBase::GetImportAttributes() {
    std::vector<std::shared_ptr<MAttribute>>  alist;
    //
    //  Copy the shared pointers from the attributes references.
    for (auto & attr : Attribute) {
        alist.emplace_back(std::dynamic_pointer_cast<MAttribute>(*attr));
    }
    //
    //  Now we go along the dependencies that have an import stereotype.
    for (auto & deplist : Dependency) {

        if (deplist->HasStereotype("Import")) {
            auto target = std::dynamic_pointer_cast<MDependency>(*deplist)->target;

            if (target->IsClassBased()) {
                std::vector<std::shared_ptr<MAttribute>> morealist = std::dynamic_pointer_cast<CClassBase>(*target)->GetImportAttributes();

                alist.insert(alist.end(), morealist.begin(), morealist.end());
            }
        }
    }
    return (alist);
}

std::vector<std::shared_ptr<MElement>> CClassBase::GetImportSupplier() {
    std::vector<std::shared_ptr<MElement>>           alist;

    for (auto & dependency : Dependency) {
        std::shared_ptr<CDependency> d = std::dynamic_pointer_cast<CDependency>(*dependency);

        d->Prepare();
        if ((*d->src).get() == this) {
            alist.push_back(*d->target);
        }
    }


    for (auto & deplist : Dependency) {
        if (deplist->HasStereotype("Import")) {
            auto target = std::dynamic_pointer_cast<MDependency>(*deplist)->target;

            if (target->IsClassBased()) {
                std::vector<std::shared_ptr<MElement>> morealist = std::dynamic_pointer_cast<CClassBase>(*target)->GetImportSupplier();

                alist.insert(alist.end(), morealist.begin(), morealist.end());
            }
        }
    }
    return (alist);
}

std::vector<std::shared_ptr<MAssociationEnd>> CClassBase::GetImportAssociationEnds() {
    std::vector<std::shared_ptr<MAssociationEnd>>           alist;
    //
    //  Copy the shared pointers from references.
    for (auto & other : OtherEnd) {
        alist.push_back(std::dynamic_pointer_cast<MAssociationEnd>(*other));
    }

    for (auto & deplist : Dependency) {
        if (deplist->HasStereotype("Import")) {
            auto target = std::dynamic_pointer_cast<MDependency>(*deplist)->target;

            if ((target->IsClassBased()) && (target->type != eElementType::Class)) {
                std::vector<std::shared_ptr<MAssociationEnd>> morealist = std::dynamic_pointer_cast<CClassBase>(*target)->GetImportAssociationEnds();

                alist.insert(alist.end(), morealist.begin(), morealist.end());
            }
        }
    }
    return (alist);
}

std::vector<std::shared_ptr<MAttribute>> CClassBase::GetDerivedAttributes() {
    std::vector<std::shared_ptr<MAttribute>>           alist;

    for (auto & attr : Attribute) {
        alist.push_back(std::dynamic_pointer_cast<MAttribute>(*attr));
    }

    for (auto baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            auto morealist = baselist.getElement()->GetDerivedAttributes();

            alist.insert(alist.end(), morealist.begin(), morealist.end());
        }
    }
    return (alist);
}


std::vector<std::shared_ptr<MAssociationEnd>> CClassBase::GetDerivedAssociationEnds() {
    std::vector<std::shared_ptr<MAssociationEnd>>           alist;
    //
    //  Copy the shared pointers from references.
    for (auto & other : OtherEnd) {
        alist.push_back(std::dynamic_pointer_cast<MAssociationEnd>(*other));
    }

    for (auto baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            auto morealist = baselist.getElement()->GetDerivedAssociationEnds();

            alist.insert(alist.end(), morealist.begin(), morealist.end());
        }
    }
    return (alist);
}

std::string CClassBase::MkType(std::shared_ptr<CAttribute> a) {
    std::string retval;
    std::string fqn=a->FQN();

    if (!(a->ClassifierName.empty())) {
        if (fqn == "__simobject__") {
            fqn = (a->Aggregation == aShared)?"tSimObj*":"tSimObjRef";
        } else {
        }
        if (a->Multiplicity == "1") {
            retval=fqn;
        } else if (a->Multiplicity == "0..1") {
            retval=fqn+"*";
        } else if (a->Multiplicity == "0..*") {
            if (a->QualifierType.empty()) {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        } else if (a->Multiplicity == "1..*") {
            if (a->QualifierType.empty()) {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        } else if (a->Multiplicity == "*") {
            if (a->QualifierType.empty()) {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        } else if (a->Multiplicity.empty()) {
            retval=fqn;
        } else {
            retval=std::string("std::map< uint64_t, ")+fqn+" >";
        }
    }
    return retval;
}

std::string CClassBase::MkType(std::shared_ptr<CAssociationEnd> a) {
    std::string retval;
    std::string fqn=a->FQN();
    eAggregation ta;  //  Aggregation at this end.
    std::shared_ptr<MAssociation> ma = std::dynamic_pointer_cast<MAssociation>(*a->parent);

    if  (ma)  {
        ta = ma->OtherEnd(a)->Aggregation;
    }

    if (fqn == "__simobject__") {
        fqn = "tSimObjRef";
    } else {
    }
    if (a->Multiplicity == "1") {
        retval=fqn;
    } else if (a->Multiplicity == "0..1") {
        retval=fqn+"*";
    } else if ((a->Multiplicity == "0..*") || (a->Multiplicity == "1..*") || (a->Multiplicity == "*")) {
        if (a->QualifierType.empty()) {
            if (ta == aShared) {
                retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
            } else {
                retval=std::string("std::map< uint64_t, ")+fqn+" >";
            }
        } else {
            if (ta == aShared) {
                retval=std::string("LockedMap< ") + a->QualifierType + ", "+fqn+" >";
            } else {
                retval=std::string("std::map< ") + a->QualifierType + ", "+fqn+" >";
            }
        }
    } else if (a->Multiplicity.empty()) {
        retval=fqn;
    } else {
        if (ta == aShared) {
            retval=std::string("LockedMap< uint64_t, ")+fqn+" >";
        } else {
            retval=std::string("std::map< uint64_t, ")+fqn+" >";
        }
    }

    return retval;
}

std::shared_ptr<CPackageBase> CClassBase::GetPackage() {
    std::shared_ptr<CPackageBase> retval;
    MElementRef     p = parent;

    //
    //  Skipping non package parents the way up in the hierarchy.
    while ((p) && (!p->IsPackageBased())) {
        p=p->parent;
    }
    retval = std::dynamic_pointer_cast<CPackageBase>(*p);

    return retval;
}

std::shared_ptr<CPackageBase> CClassBase::GetPathEffectivePackage() {
    std::shared_ptr<CPackageBase> retval;
    MElementRef                   p      = parent;
    std::shared_ptr<CPackageBase> pbase  = std::dynamic_pointer_cast<CPackageBase>(*p);

    while (((p) && (!p->IsPackageBased())) ||
           ((p) && (p->IsPackageBased()) && ((pbase->Directory.empty()) || (pbase->Directory == ".") || (pbase->Directory == "./"))  )
           ) {
        p = p->parent;
        pbase  = std::dynamic_pointer_cast<CPackageBase>(*p);
    }
    retval = pbase;
    return retval;
}
//
//  Find the path to an element based on the packages.
std::string CClassBase::GetPathTo(std::shared_ptr<MElement> e) {
    std::string   retval;
    std::shared_ptr<CPackageBase> mine;
    std::shared_ptr<CPackageBase> other;
    //
    //  Not searching the path to ourselves
    if (e.get() != this) {
        //
        //  Only done for class based elements
        if (e->IsClassBased()) {
            auto cb = std::dynamic_pointer_cast<CClassBase>(e);
            //
            //  Get the packages that "this" and "e" are part of.
            mine  = GetPathEffectivePackage();
            other = cb->GetPathEffectivePackage();
            //
            //  check that both are valid.
            if (other && mine) {
                //
                //  Create the lists of path elements.
                std::list<std::string> minepath  = mine->GetPathList();
                std::list<std::string> otherpath = other->GetPathList();
                //
                //  Remove all path elements that are equal. So we do not run
                //  up the whole path to the root before descending.
                while ((!minepath.empty()) && (!otherpath.empty())) {
                    if (*minepath.begin() == *otherpath.begin()) {
                        minepath.erase(minepath.begin());
                        otherpath.erase(otherpath.begin());
                    } else {
                        break;
                    }
                }
                //
                // uppath.
                //  For each entry left over in minepath we introduce the uppath "../"
                for (auto & m: minepath) {
                    //
                    //  Ignore "." and similar.
                    if ((m != ".") &&
                            (m != "./") &&
                            (m != ".//")) {
                        retval +=  "../";
                    }
                }
                //
                // downpath
                for (auto o: otherpath) {
                    retval += o;
                }
            } else {
//                std::cerr << "lost other package\n";
            }
        }
        //
        //  Cleanup the pathes. The CPath class will create the smallest possible path.
        CPath cleaner = retval;

        retval = cleaner;
    } else {
//        std::cerr << "same package\n";
    }
    return retval;
}

void CClassBase::DumpExtraIncludes(const std::string& aHeaders, std::ostream &src, std::set<std::string>& aDoneIncludes, bool aCheckModel, const std::string& aPath) {
    std::string extra;
    size_t      start       = 0;
    size_t      end         = 0;
    //
    //  The aPath argument is used to detect an InModel header that does not gets modeled.
    //  This is needed to maintain the path and change <> to ""
    do {
        end = aHeaders.find_first_of(' ', start);
        if (end != std::string::npos) {
            extra = aHeaders.substr(start, end-start);
        } else {
            extra = aHeaders.substr(start);
        }
        /*
         * trim front
         */
        while (extra[0]==' ') extra.erase(0, 1);
        /*
         * if extraheader has a size after trimming we can add it to the
         * list of extra headers.
         */
        if (!extra.empty()) {
            /*
             * First check if we have it already.
             */
            if (aDoneIncludes.find(extra) == aDoneIncludes.end()) {
                /*
                 * Check if the Header is part of the needed header list.
                 * Headers from the model have precedence over the extra-headers.
                 */
                if (!aCheckModel || !InModelHeader(extra)) {
                    if (aPath.empty()) {
                        src << "#include <" << extra << ">\n";
                    } else {
                        src << "#include \"" << aPath << extra << "\"\n";
                    }
                }
                /*
                 * Anyway the header is part of the model or a real extra
                 * we put into the donelist to prevent processing it again.
                 */
                aDoneIncludes.insert(extra);
            }
        }
        /*
         * if we are not at end of search we skip a character.
         * At end we do nothing and let the loop condition stop
         * the loop.
         */
        if (end == std::string::npos) {
            start = end;
        } else {
            start = end + 1;
        }
    } while (end!=std::string::npos);
}

void CClassBase::DumpExtraIncludes(std::ostream &src, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>> &aDoneList) {
   for (auto i : extramodelheader) {
        if (aDoneList.find(i) == aDoneList.end()) {
            std::string header = i->GetExtraHeader();

            if (!header.empty()) {
                bool externSet = i->IsExternClass();
                std::string inmodelpath;

                if (i->mHasInModelHeader) {
                    if (i->IsClassBased()) {
                        inmodelpath = GetPathTo(i);
                    }
                }
                DumpExtraIncludes(header, src, aDoneIncludes, !externSet, inmodelpath);
            } else {
                if ((i->type == eElementType::ExternClass) || (i->type == eElementType::QtClass)) {
                    if (aDoneIncludes.find(i->name) == aDoneIncludes.end()) {
                        src << "#include <" << i->name << ">\n";
                        aDoneIncludes.insert(i->name);
                    }
                }
            }
            aDoneList.insert(i);
            i->DumpExtraIncludes(src, aDoneIncludes, aDoneList);
        }
    }
}

void CClassBase::DumpInModelIncludes(const std::string& aHeaders, std::ostream &src, std::set<std::string>& aDoneIncludes, bool aCheckModel, const std::string& aPath) {
    std::string extra;
    size_t      start       = 0;
    size_t      end         = 0;
    //
    //  The aPath argument is used to detect an InModel header that does not gets modeled.
    //  This is needed to maintain the path and change <> to ""
    do {
        end = aHeaders.find_first_of(' ', start);
        if (end != std::string::npos) {
            extra = aHeaders.substr(start, end-start);
        } else {
            extra = aHeaders.substr(start);
        }
        /*
         * trim front
         */
        while (extra[0]==' ') extra.erase(0, 1);
        /*
         * if extraheader has a size after trimming we can add it to the
         * list of extra headers.
         */
        if (!extra.empty()) {
            /*
             * First check if we have it already.
             */
            if (aDoneIncludes.find(extra) == aDoneIncludes.end()) {
                /*
                 * Check if the Header is part of the needed header list.
                 * Headers from the model have precedence over the extra-headers.
                 */
                if (!aCheckModel || !InModelHeader(extra)) {
                    if (aPath.empty()) {
                        src << "#include <" << extra << ">\n";
                    } else {
                        src << "#include \"" << aPath << extra << "\"\n";
                    }
                }
                /*
                 * Anyway the header is part of the model or a real extra
                 * we put into the donelist to prevent processing it again.
                 */
                aDoneIncludes.insert(extra);
            }
        }
        /*
         * if we are not at end of search we skip a character.
         * At end we do nothing and let the loop condition stop
         * the loop.
         */
        if (end == std::string::npos) {
            start = end;
        } else {
            start = end + 1;
        }
    } while (end!=std::string::npos);
}

void CClassBase::DumpInModelIncludes(std::ostream &src, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>> &aDoneList) {
    for (auto i : extramodelheader) {
        if (aDoneList.find(i) == aDoneList.end()) {
            std::string header = i->GetExtraHeader();

            if (!header.empty()) {
                bool externSet = i->IsExternClass();
                std::string inmodelpath;

                if (i->mHasInModelHeader) {
                    if (i->IsClassBased()) {
                        inmodelpath = GetPathTo(i);
                    }
                }
                DumpInModelIncludes(header, src, aDoneIncludes, !externSet, inmodelpath);
            } else {
                if ((i->type == eElementType::ExternClass) || (i->type == eElementType::QtClass)) {
                    if (aDoneIncludes.find(i->name) == aDoneIncludes.end()) {
                        src << "#include <" << i->name << ">\n";
                        aDoneIncludes.insert(i->name);
                    }
                }
            }
            aDoneList.insert(i);
            i->DumpInModelIncludes(src, aDoneIncludes, aDoneList);
        }
    }
}


void CClassBase::DumpHeaderIncludes(std::ostream &src, std::shared_ptr<CClassBase> aBase, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>> &aDoneList) {
    //src << "// Need-In : " << name << "\n";
    for (auto i : neededmodelheader) {
        std::string n = i->name;
        if (i->IsClassBased()) {
            n = std::dynamic_pointer_cast<CClassBase>(i)->getHeaderFile();
        }
        //
        //  Remove &
        size_t pos = 0;

        while ((pos = n.find_first_of('&')) != std::string::npos) {
            n.erase(pos, 1);
        }

        if (dumpHelp) {
            std::cerr << i->name << std::endl;
        }
        if (aDoneIncludes.find(n) == aDoneIncludes.end()) {
            switch (i->type) {
            case eElementType::QtClass:
                src << "#include <" << n << ">\n";
                break;
            default:
                break;
            }
            aDoneIncludes.insert(n);
        }
    }
    //src << "// Need-Out: " << name << "\n";
}


void CClassBase::DumpNeededIncludes(std::ostream &src, std::shared_ptr<CClassBase> aBase, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>> &aDoneList) {
    //src << "// Need-In : " << name << "\n";
    for (auto & i : neededmodelheader) {
        std::string n = i->name;
        //
        //  Prepare for switching over to a method.
        if (i->IsClassBased()) {
            n = std::dynamic_pointer_cast<CClassBase>(i)->getHeaderFile();
        }
        //
        //  Remove &
        size_t pos = 0;

        while ((pos = n.find_first_of('&')) != std::string::npos) {
            n.erase(pos, 1);
        }

        switch (i->type) {
        case eElementType::QtClass:
            if (aDoneIncludes.find(n) == aDoneIncludes.end()) {
                src << "#include <" << n << ">\n";
                aDoneIncludes.insert(n);
            }
            break;
        case eElementType::PrimitiveType:
            if ((i->Supplier.empty()) && (i->IsClassBased())) {
                std::string e = std::dynamic_pointer_cast<CClassBase>(i)->GetExtraHeader();

                if (!e.empty()) {
                    DumpExtraIncludes(e, src, aDoneIncludes);
                } else {
                    if ((i->type == eElementType::ExternClass) || (i->type == eElementType::QtClass)) {
                        if (aDoneIncludes.find(i->name) == aDoneIncludes.end()) {
                            src << "#include <" << i->name << ">\n";
                            aDoneIncludes.insert(i->name);
                        }
                    }
                }
            } else {
                for (auto s : i->Supplier) {
                    if (s.getElement()) {
                        std::string ns = s.getElement()->name;

                        if (s.getElement()->IsClassBased()) {
                            //
                            //  Remove &
                            size_t spos = 0;

                            while ((spos = ns.find_first_of('&')) != std::string::npos) {
                                ns.erase(spos, 1);
                            }
                            std::string e = std::dynamic_pointer_cast<CClassBase>(s.getElement())->GetExtraHeader();

                            if (!e.empty()) {
                                DumpExtraIncludes(e, src, aDoneIncludes);
                            } else {
                                std::string p = aBase->GetPathTo(s.getElement());
                                std::string pp;

                                if (p.empty()) {
                                    pp = ns + ".h";

                                    if (aDoneIncludes.find(pp) == aDoneIncludes.end()) {
                                        src << "#include \"" << pp <<"\"  // Needed primitive without path\n";
                                        aDoneIncludes.insert(pp);
                                    }
                                } else {
                                    pp = p +"/" + ns +".h";
                                    CPath tmp = pp;
                                    std::string tmps = tmp;
                                    if (aDoneIncludes.find(tmps) == aDoneIncludes.end()) {
                                        src << "#include \"" << tmps << "\" //  Needed primitive with path\n";
                                        aDoneIncludes.insert(tmps);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:

                if ((i->IsClassBased() && std::dynamic_pointer_cast<CClassBase>(i)->IsExternClass()) || (i->type == eElementType::QtClass)) {
                    if (i->HasTaggedValue("ExtraInclude")) {
                        DumpExtraIncludes(i->GetTaggedValue("ExtraInclude"), src, aDoneIncludes, false);
                    } else {
                        if (aDoneIncludes.find(i->name) == aDoneIncludes.end()) {
                            src << "#include <" << i->name << ">\n";
                            aDoneIncludes.insert(i->name);
                        }
                    }
                } else {
                    std::string p = aBase->GetPathTo(i);
                    std::string pp;

                    if (p.empty()) {
                        pp = n + ".h";

                        if (aDoneIncludes.find(pp) == aDoneIncludes.end()) {
                            src << "#include \"" << pp <<"\" // Needed default without path\n";
                            aDoneIncludes.insert(pp);
                        }
                    } else {
                        CPath tmp = p + n + ".h";
                        std::string tmps = tmp;

                        if (aDoneIncludes.find(tmps) == aDoneIncludes.end()) {
                            src << "#include \"" << tmps << "\" // Needed default \n";
                            aDoneIncludes.insert(tmps);
                        }
                    }
                }
            break;
        }
    }
    //src << "// Need-Out: " << name << "\n";
}

void CClassBase::DumpOptionalIncludes(std::ostream &src, std::shared_ptr<CClassBase> aBase, std::set<std::string>& aDoneIncludes, std::set<std::shared_ptr<MElement>> &aDoneList) {
    //src << "// Opt-In   : " << name << "\n";
    for (auto & i : optionalmodelheader) {
        std::string n = i->name;
        //
        //  Prepare for switching over to a method.
        if (i->IsClassBased()) {
            n = std::dynamic_pointer_cast<CClassBase>(i)->getHeaderFile();
        }
        //
        //  Remove &
        size_t pos = 0;

        while ((pos = n.find_first_of("&*")) != std::string::npos) {
            n.erase(pos, 1);
        }
        if (dumpHelp) {
            std::cerr << i->name << std::endl;
        }
        switch (i->type) {
        case eElementType::QtClass:
            if (aDoneIncludes.find(n) == aDoneIncludes.end()) {
                src << "#include <" << n << ">\n";
                aDoneIncludes.insert(n);
            }
            break;
        case eElementType::ExternClass:
            break;
        case eElementType::PrimitiveType:
            //std::cerr << "Doing optional prim " << i->name << std::endl;
            if ((!i->Supplier.empty()) && (!(std::dynamic_pointer_cast<CClassBase>(i)->IsExternClass()))) {
                //
                //  The needed model includes are to be dumped as optionals here.
                //((CClassBase*)(*i))->DumpNeededIncludes(src, aBase, aDoneIncludes, aDoneList);
                if (aDoneList.find(i) == aDoneList.end()) {
                    aDoneList.insert(i);
                    if (i->IsClassBased()) {
                        std::dynamic_pointer_cast<CClassBase>(i)->DumpOptionalIncludes(src, aBase, aDoneIncludes, aDoneList);
                    }
                }
                if (i->IsClassBased()) {
                    std::string p=aBase->GetPathTo(i);
                    if (p.empty()) {
                        std::string pp = n + ".h";

                        if (aDoneIncludes.find(pp) == aDoneIncludes.end()) {
                            src << "#include \"" << pp << "\"\n";
                            aDoneIncludes.insert(pp);
                        }
                    } else {
                        std::string pp = p+ "/" + n + ".h";
                        CPath tmp = pp;
                        std::string tmps = tmp;

                        if (aDoneIncludes.find(tmps) == aDoneIncludes.end()) {
                            src << "#include \"" << tmps << "\"\n";
                            aDoneIncludes.insert(tmps);
                        }
                    }
                } else {
                    std::string pp = n + ".h";

                    if (aDoneIncludes.find(pp) == aDoneIncludes.end()) {
                        src << "#include \"" << pp << "\"\n";
                        aDoneIncludes.insert(pp);
                    }
                }
            }
            break;
        default:
            //
            //  Exclude simple classes from optionals.
                if (i->type != eElementType::Class) {
                    //
                    //  The needed model includes are to be dumped as optionals here.
                    //((CClassBase*)i)->DumpNeededIncludes(src, aBase, aDoneIncludes, aDoneList);
                    if (aDoneList.find(i) == aDoneList.end()) {
                        aDoneList.insert(i);
                        if (i->IsClassBased()) {
                            //((CClassBase*)i)->DumpOptionalIncludes(src, aBase, aDoneIncludes, aDoneList);
                        }
                    }

                    if (i->IsClassBased()) {
                        if (i->HasStereotype("QtDesigner")) {
                            n = "ui_" + n;
                        }
                        std::string p = aBase->GetPathTo(i);
                        if (p.empty()) {
                            std::string pp = n + ".h";

                            if (aDoneIncludes.find(pp) == aDoneIncludes.end()) {
                                src << "#include \"" << pp << "\"\n";
                                aDoneIncludes.insert(pp);
                            }
                        } else {
                            std::string pp = p+ "/" + n + ".h";
                            CPath tmp = pp;
                            std::string tmps = tmp;

                            if (aDoneIncludes.find(tmps) == aDoneIncludes.end()) {
                                src << "#include \"" << tmps << "\"\n";
                                aDoneIncludes.insert(tmps);
                            }
                        }
                    } else {
                        std::string pp = n + ".h";

                        if (aDoneIncludes.find(pp) == aDoneIncludes.end()) {
                            src << "#include \"" << pp << "\"\n";
                            aDoneIncludes.insert(pp);
                        }
                    }
                }
            break;
        }
    }
    //src << "// Opt-Out  : " << name << "\n";
}


std::vector <std::shared_ptr<MMessage>> CClassBase::GetImportIncoming() {
    std::vector<std::shared_ptr<MMessage>> mlist;

    for (auto & in : Incoming) {
        mlist.push_back(std::dynamic_pointer_cast<MMessage>(*in));
    }

    for (auto & deplist : Dependency) {
        if (deplist->HasStereotype("Import")) {
            auto target = std::dynamic_pointer_cast<MDependency>(*deplist)->target;

            if (target->IsClassBased()) {
                std::vector<std::shared_ptr<MMessage>> morelist = std::dynamic_pointer_cast<CClassBase>(*target)->GetImportIncoming();

                mlist.insert(mlist.end(), morelist.begin(), morelist.end());
            }
        }
    }

    return (mlist);

}

std::vector <std::shared_ptr<MMessage>> CClassBase::GetImportOutgoing() {
    std::vector<std::shared_ptr<MMessage>> mlist;

    for (auto & in : Outgoing) {
        mlist.push_back(std::dynamic_pointer_cast<MMessage>(*in));
    }

    for (auto & deplist : Dependency) {
        if (deplist->HasStereotype("Import")) {
            auto target = std::dynamic_pointer_cast<MDependency>(*deplist)->target;

            if (target->IsClassBased()) {
                std::vector<std::shared_ptr<MMessage>> morelist = std::dynamic_pointer_cast<CClassBase>(*target)->GetImportOutgoing();

                mlist.insert(mlist.end(), morelist.begin(), morelist.end());
            }
        }
    }
    return (mlist);
}

std::vector <std::shared_ptr<MOperation>> CClassBase::GetImportOperation() {
    std::vector<std::shared_ptr<MOperation>> olist;

    for (auto & op : Operation) {
        olist.push_back(std::dynamic_pointer_cast<MOperation>(*op));
    }
    for (auto & deplist : Dependency) {
        if (deplist->HasStereotype("Import")) {
            auto target = std::dynamic_pointer_cast<MDependency>(*deplist)->target;

            if (target->IsClassBased()) {
                std::vector<std::shared_ptr<MOperation>> morelist = std::dynamic_pointer_cast<CClassBase>(*target)->GetImportOperation();

                olist.insert(olist.end(), morelist.begin(), morelist.end());
            }
        }
    }
    return (olist);
}

void CClassBase::CollectAttributes() {
    //
    //  Check if there are already attributes in the allAttr array.
    if (allAttr.empty()) {
        //
        //  Collect all attributes including from imports.
        std::vector<std::shared_ptr<MAttribute>> alist = GetImportAttributes();

        for (auto &a: alist) {
            if ((a->Classifier) || (!(a->ClassifierName.empty()))) {
                allAttr.push_back(std::dynamic_pointer_cast<CAttribute>(a));
            }
        }
    }
    //
    //  Check if there are already association ends in the allEnds array.
    if (allEnds.empty()) {
        //
        //  Now fetch all association ends on other side including
        //  from imports.
        std::vector<std::shared_ptr<MAssociationEnd>> elist = GetImportAssociationEnds();

        for (auto &e: elist) {
            allEnds.push_back(std::dynamic_pointer_cast<CAssociationEnd>(e));
        }
    }
}

std::string CClassBase::GetNameSpace() {

    std::string retval;

    retval = mNameSpace.getString();
    return retval;
}
//
//  Check if a specific header shall be dumped as in Model Header
bool CClassBase::InModelHeader(const std::string &aHeader) {
    bool retval = false;

    for (auto & n : neededmodelheader) {
        if ((aHeader == n->name + ".h")) {
            retval = true;
            break;
        }
    }
    if (!retval) {
        for (auto & o : optionalmodelheader) {
            if (aHeader == o->name + ".h") {
                retval = true;
                break;
            }
        }
    }
    return retval;
}

bool CClassBase::IsQtDerived() {
    bool retval = false;

    for (auto & b : Base) {
        if (b.getElement()->HasStereotype("Qt")) {
            retval = true;
        } else {
            retval = b.getElement()->IsQtDerived();
        }
        if (retval) {
            break;
        }
    }
    return retval;
}

bool CClassBase::IsVariantType(const std::string &aClassifierName) {
    bool retval = false;

    if ((aClassifierName == "double") || (aClassifierName == "float")) {
        retval = true;
    } else if ((aClassifierName == "int64_t") || (aClassifierName == "int32_t") || (aClassifierName == "int16_t") ||
               (aClassifierName == "int8_t")  || (aClassifierName == "char") || (aClassifierName == "short") || (aClassifierName == "int") ||
               (aClassifierName == "long")) {
        retval = true;
    } else if (aClassifierName == "bool") {
        retval = true;
    } else if ((aClassifierName == "uint64_t") || (aClassifierName == "uint32_t") || (aClassifierName == "uint16_t") ||
               (aClassifierName == "uint8_t") || (aClassifierName == "objectid_t") || (aClassifierName == "valueid_t") ||
               (aClassifierName == "unsigned") || (aClassifierName == "unsigned char") || (aClassifierName == "unsigned int") ||
               (aClassifierName == "unsigned long") || (aClassifierName == "unsigned short")) {
        retval = true;
    } else if ((aClassifierName == "string") || (aClassifierName == "std::string")) {
        retval = true;
    }

    return retval;
}


std::string CClassBase::GetExtraHeader() {
    std::string retval;

    if (extraheader.empty()) {
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
        retval = extraheader;
    }

    return retval;
}

bool CClassBase::IsExternClass() {
    bool retval = false;

    if ((!HasStereotype("Extern")) && (type != eElementType::ExternClass)) {
        if (parent) {
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

bool CClassBase::IsExternInModel() {
    bool retval = false;

    if ((IsExternClass()) && (mHasInModelHeader)) {
        retval = true;
    }
    return retval;
}

std::string CClassBase::getHeaderFile() {
    return mTypeTree.mName;
}

std::string CClassBase::getFileName() {
    return mTypeTree.mName;
}


void CClassBase::DumpMessageIncludes(std::ostream &src, std::set<std::string>& aDoneList, std::set<std::string>& aIncludesDone) {
#if 0
    std::set<std::shared_ptr<MElement>>              ldone;  // local done list.

    for (auto const & mi : GetImportIncoming()) {
        if (mi->type == eElementType::SimMessage) {
            auto const s  = std::dynamic_pointer_cast<CSimMessage>(mi);
            auto const sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

            if (sc != nullptr) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    for (auto const & inc : sc->GetNeededHeader()) {
                        std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(src, std::dynamic_pointer_cast<CClassBase>(MElement::Instances[id]), aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming Simulation-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if (mi->type == eElementType::JSONMessage) {
            auto const s  = std::dynamic_pointer_cast<CJSONMessage>(mi);
            auto const sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

            if (sc) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    src << "#include \"" << sc->name << ".h\"\n";
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming JSON-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if (mi->type == eElementType::SimSignal) {
            auto const s  = std::dynamic_pointer_cast<CSimSignal>(mi);
            auto const sc = std::dynamic_pointer_cast<CSignalClass>(s->GetClass());

            if (sc) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    for (auto const & inc : sc->GetNeededHeader()) {
                        std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(src, std::dynamic_pointer_cast<CClassBase>(MElement::Instances[id]), aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming Simulation-Signal:" << s->name << ": in SimObject:" << name << ":\n";
            }
        }
    }


    for (auto const & mo : GetImportOutgoing()) {
        if (mo->type == eElementType::SimMessage) {
            auto const  s  = std::dynamic_pointer_cast<CSimMessage>(mo);
            auto const  sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

            if (sc) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    for (auto const & inc :  sc->GetNeededHeader()) {
                        std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(src, std::dynamic_pointer_cast<CClassBase>(MElement::Instances[id]), aIncludesDone, ldone);
                        if ((aIncludesDone.find(inc->name) == aIncludesDone.end())   && (inc->type != eElementType::ExternClass)) {
//                            src << "#include \"" << (*inc)->name << ".h\"\n";
                            aIncludesDone.insert(inc->name);
                        }
                    }
                    aDoneList.insert(sc->name);
                }
            } else {
                std::cerr << "No Class definition found for outgoing Simulation-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if (mo->type == eElementType::JSONMessage) {
            auto const s  = std::dynamic_pointer_cast<CJSONMessage>(mo);
            auto const sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

            if (sc) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    for (auto const & inc : sc->GetNeededHeader()) {
                        std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(src, std::dynamic_pointer_cast<CClassBase>(MElement::Instances[id]), aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                }
            } else {
                std::cerr << "No Class definition found for outgoing JSON-Message:" << s->name << ": in SimObject:" << name << ":\n";
            }
        } else if (mo->type == eElementType::SimSignal) {
            auto const s  = std::dynamic_pointer_cast<CSimSignal>(mo);
            auto const sc = std::dynamic_pointer_cast<CSignalClass>(s->GetClass());

            if (sc) {
                if (aDoneList.find(sc->name) == aDoneList.end()) {
                    for (auto const & inc : sc->GetNeededHeader()) {
                        std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(src, std::dynamic_pointer_cast<CClassBase>(MElement::Instances[id]), aIncludesDone, ldone);
                    }
                    aDoneList.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for outgoing Simulation-Signal:" << s->name << ": in SimObject:" << name << ":\n";
            }
        }
    }

    //
    //
    for (auto & baselist : Base) {
        if (baselist.getElement()->type == eElementType::SimObject) {
            std::dynamic_pointer_cast<CSimObjectV2>(baselist.getElement())->DumpMessageIncludes(src, aDoneList, aIncludesDone);
        }
    }
#endif
}

void CClassBase::DumpSystemHeader(std::ostream &hdr) {
    std::string extra;
    size_t      start       = 0;
    size_t      end         = 0;
    bool        dumpComment = true;

    if (!mSystemHeader.empty()) {
        do {
            end = mSystemHeader.find_first_of(' ', start);
            if (end != std::string::npos) {
                extra = mSystemHeader.substr(start, end-start);
            } else {
                extra = mSystemHeader.substr(start);
            }
            /*
             * trim front
             */
            while (extra[0]==' ') extra.erase(0, 1);
            /*
             * if extraheader has a size after trimming we can add it to the
             * list of extra headers.
             */
            if (!extra.empty()) {
                if (dumpComment) {
                    hdr << "//\n//  Add self contained system header here.\n";
                    dumpComment = false;
                }
                hdr << "#include <" << extra << ">\n";
            }
            /*
             * if we are not at end of search we skip a character.
             * At end we do nothing and let the loop condition stop
             * the loop.
             */
            if (end == std::string::npos) {
                start = end;
            } else {
                start = end + 1;
            }
        } while (end!=std::string::npos);
    }
}

std::list<std::shared_ptr<CClassBase>> CClassBase::getTypes() {
    return getTypes(mTypeTree);
}

std::list<std::shared_ptr<CClassBase>> CClassBase::getRefTypes() {
    return getRefTypes(mTypeTree);
}

#if 0
void CClassBase::prepTypeTree(TypeNode &aNode) {
    if (!aNode.mClassifier) {
        auto found = MClass::mByFQN.find(aNode.mName);

        if ((found != MClass::mByFQN.end()) && (found->second->IsClassBased())) {
            //
            //  Get the Name space from the package if no namespace set in the class.
            aNode.mClassifier = MElementRef(found->second);
        }
    }
    for (auto& p : aNode.mParameter) {
        prepTypeTree(p);
    }
}
#endif

std::list<std::shared_ptr<CClassBase>> CClassBase::getTypes(const TypeNode& aNode) {
    std::list<std::shared_ptr<CClassBase>> retval;

    if (aNode.mClassifier) {
        retval.push_back(std::dynamic_pointer_cast<CClassBase>(*aNode.mClassifier));

        for (auto & p : aNode.mParameter) {
            auto pc = getTypes(p);

            retval.insert(retval.end(), pc.begin(), pc.end());
        }
    }

    return retval;
}

std::list<std::shared_ptr<CClassBase>> CClassBase::getRefTypes(const TypeNode& aNode) {
    std::list<std::shared_ptr<CClassBase>> retval;

    if (aNode.mClassifier != nullptr) {
        if (aNode.NeedsForward()) {
            retval.push_back(std::dynamic_pointer_cast<CClassBase>(*aNode.mClassifier));
        }

        for (auto & p : aNode.mParameter) {
            auto pc = getRefTypes(p);

            retval.insert(retval.end(), pc.begin(), pc.end());
        }
    }

    return retval;
}


void CClassBase::DumpEA(std::shared_ptr<CModel> aModel, std::ostream& aExport) {
    if (mExport) {
        aExport << "<packagedElement xmi:type=\"uml:Class\" xmi:id=\"" << id << "\" name=\"" << name << "\">"
                << std::endl;
    }

    for (auto & o : owned) {
        if (o->IsClassBased()) {
            std::dynamic_pointer_cast<CClassBase>(*o)->DumpEA(aModel, aExport);
        } else {
            if (o->IsPackageBased()) {
                std::dynamic_pointer_cast<CPackageBase>(*o)->DumpEA(aModel, aExport);
            }
        }
    }
    if (mExport) {
        aExport << "</packagedElement>" << std::endl;
    }
}

void CClassBase::DumpEAExtension(std::shared_ptr<CModel> aModel, std::ostream &aExport) {
    (void) aModel;
    (void) aExport;
}

std::list<std::shared_ptr<CClassBase>> CClassBase::getEnclosedClasses() {
    std::list<std::shared_ptr<CClassBase>> retval;

    for (auto& o : owned) {
        if (o->IsClassBased()) {
            retval.emplace_back(std::dynamic_pointer_cast<CClassBase>(*o));
        }
    }
    return retval;
}

void CClassBase::DumpTemplatePrefix(std::ostream & file) {
    file << "template <";
    for (auto & p : mClassParameter) {
        if (p.first > 0) {
            file << ", ";
        }
        file << p.second->ClassifierName << " " << p.second->name;
    }
    file << ">\n";
}

void CClassBase::DumpTemplateParameter(std::ostream& file) {
    file << "<";
    for (auto & p : mClassParameter) {
        if (p.first > 0) {
            file << ", ";
        }
        file << p.second->name;
    }
    file << ">";
}
