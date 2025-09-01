//
// Copyright 2017 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#ifndef SIMOBJECTV2_H
#define SIMOBJECTV2_H

#include <cstdint>

#ifndef __UINT64_MAX__
#define __UINT64_MAX__ UINT64_MAX
#endif

#include "ccxxclass.h"

class MElement;
class CAssociationEnd;
class CAttribute;
class MState;
class MTransition;

class CSimObjectV2 : public CCxxClass
{
public:
    CSimObjectV2() = default;
    CSimObjectV2(const std::string& aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {has_src = ".cpp"; has_hdr = ".h";type = eElementType::SimObject;}
    virtual void SetFromTags(const std::string& name, const std::string&value);

    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);
private:
    //void CollectNeededModelHeader(std::shared_ptr<MElement>);
    //void CollectOptionalModelHeader(std::shared_ptr<MElement> e) ;
    void DumpEventTransition(std::ostream &src, const std::string& evname);
    void DumpEventTransition(std::shared_ptr<MState> fromstate, std::ostream &src, const std::string& evname) ;
    void DumpSetValueSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& donelist);
    void DumpGetValueSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& donelist);
    void DumpSetValueDBSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& donelist);
    void DumpSetStateSwitch(std::ostream& output);
    void DumpGetStateSwitch(std::ostream& output);
    void DumpGetReferenceSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& donelist);
    void DumpSetReferenceSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& donelist);
    void DumpRemoveReferenceSwitch(std::shared_ptr<CSimObjectV2> where, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& donelist);
    void DumpInitEmptyObject(std::ostream& output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::set<std::string>& aDoneList);
    void DumpInitDBObject(std::ostream &output, std::shared_ptr<MElement> e, std::string localobject,
                                        std::string prefix, std::set<std::string>& aDoneList)    ;
    void DumpInitAttributesInDB(std::ostream& output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, int filler, std::set<std::string>& aDoneList);
    void DumpCopyTemplate(std::ostream& output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::string found, std::set<std::string>& aDoneList);
    void DumpCopyFromTemplate(std::ostream& output, std::shared_ptr<MElement> e, std::string localobject, std::string prefix, std::string found, std::set<std::string>& donelist);
    void DumpStateHistory(std::ostream& output);
    void DumpStateFunctionPrototypes(std::ostream& output, std::shared_ptr<MState> st, std::string prefix);
    void DumpStateFunctions(std::ostream& output, std::shared_ptr<MState> st);
    void DumpStateEntry(std::ostream& output, std::shared_ptr<MState> st, int spacer);
    void DumpStateExit(std::ostream& output, std::shared_ptr<MState> st, int spacer);
    void ChangeState(std::ostream& output, std::shared_ptr<MState> to, int spacer, std::string statevar);
    void DumpTransition(std::ostream& output, std::shared_ptr<MTransition> m, int spacer, std::string statevar);
    void DumpTransitionAction(std::ostream& output, std::shared_ptr<MTransition> m, int spacer);
    void DumpHistoryFromTransition(std::ostream& output, std::shared_ptr<MState> from, std::shared_ptr<MState> to, int spacer) ;
    void DumpCompositeStateEnum(std::ostream& output, std::string prefix, std::shared_ptr<MState> st);
    void DumpInitNew(std::ostream&src);
    void DumpProcessSigSwitch(std::ostream& src, std::set<std::string>& aDoneList) ;
    void DumpProcessMsgSwitch(std::ostream& src, std::set<std::string>& aDoneList) ;
    void DumpNewProcessMsg(std::ostream& src);
    void DumpCreateObject(std::ostream& src) ;
    void DumpCopyFromTemplate(std::ostream& src) ;
    void DumpCreateNewObject(std::ostream& src) ;
    void DumpCreateNewObjectFromTemplate(std::ostream& src) ;
    void DumpSaveFunction(std::ostream&);
    void DumpDefaultSigHandler(std::ostream&);
    void DumpDefaultMsgHandler(std::ostream&);
    void DumpMessageProcessingProto(std::ostream& src, std::set<std::shared_ptr<MElement>> aDoneList);
    void DumpMessageProcessingFunctions(std::shared_ptr<CSimObjectV2> where, std::set<std::shared_ptr<MElement>> aDoneList);

    void DumpStatemachine(std::ostream& src, std::string statevar);
    void DumpCompositeStatemachine(std::ostream& src, std::shared_ptr<MState> aState);
    void DumpStateInit(std::ostream& output, std::shared_ptr<MState> aState, int spacer);

    void DumpMessageForwards(std::ostream& src);
    void DumpReturnTypeForwards(std::ostream& src);

    std::string MkType(std::shared_ptr<CAttribute> attr);
    std::string MkType(std::shared_ptr<CAssociationEnd> attr);
    std::string MkSimAttr(std::shared_ptr<CAttribute> attr);
    std::string MkSimAttr(std::shared_ptr<CAssociationEnd> attr);
    std::string MkVariant(std::shared_ptr<CAttribute> attr);
    std::string MkVariant(std::shared_ptr<CAssociationEnd> attr);
    std::string MapType(std::shared_ptr<CAttribute> attr);
    std::string MapType(std::shared_ptr<CAssociationEnd> attr);
    std::string MapVariant(const std::string& classifier);
    std::string MapSimAttr(const std::string& classifier);
    std::string MapVariantType(const std::string& classifier);
    std::string MapDBAttrType(const std::string& classifier);
    int GetStateIndex(std::shared_ptr<MState> st);
    bool HaveOperation(const char* opname);
    bool HaveProcessOperation(std::string opname, std::string msg);
    std::shared_ptr<MState> GetInitialTargetState(std::shared_ptr<MState> st);
    std::string GetBaseClasses(void);
    bool HaveSimObjectBase(void);
    std::string GetStateClass(void);
    std::list<std::string> GetStateVars(std::shared_ptr<MElement> check, std::string prefix);

    std::string GetInitOperation();
private:
    //void CollectNeededRecursive(std::shared_ptr<MElement> e);
    //void CollectNeededFromTextTyped(std::string aTextType);
    void CollectNeededFromMessages(std::shared_ptr<MElement> e, HeaderList & a_headerlist);
public:
    std::map<std::string, uint64_t>    id_map;             //  This map is used to be exported to the ids.
    std::map<std::string, uint64_t>    id_name_map;        //  This map is used to create the name mapping for the code and the DB.
    std::map<std::string, std::string> macrolist;          //  List of public macros.
    std::set<uint64_t>                 iddb;               //  This set of ids is used to detect already processed attributes.
    std::list<std::string>             statelist;          //  Mapping between state function names and index into function array.
    std::list<std::string>             smhistorylist;      //  List of state history attribute names
    std::list<std::shared_ptr<MElement>> refs;               //  List of references to initialize
    std::string                        basename;
    std::string                        lower_basename;
    std::string                        upper_basename;
    bool                               MainViewPort = false;
    uint64_t                           ReleaseTimeout = __UINT64_MAX__;
};

#endif // CSimObjectV2_H
