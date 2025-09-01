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
#ifndef MELEMENT_H
#define MELEMENT_H

#include <vector>
#include <map>
#include <list>
#include <string>
#include <memory>
#include <iostream>
#include "helper.h"

class MModel;
class MElement;
class MStereotype;

enum class eElementType {
    Element,
    Stereotype,
    Class,
    PrimitiveType,
    DataType,
    Attribute,
    Parameter,
    Operation,
    Association,
    AssociationEnd,
    Connector,
    Dependency,
    Generalization,
    LifeLine,
    WebPageLifeLine,
    JSLifeLine,
    Interaction,
    UseCase,
    CxxClass,
    ModuleClass,
    Collaboration,
    Struct,
    Union,
    Enumeration,
    CClass,
    ExternClass,
    JSClass,
    Statemachine,
    State,
    PseudoState,
    FinalState,
    StateTransition,
    Action,
    Event,
    QtClass,
    SimMessageClass,
    SimSignalClass,
    PHPClass,
    SimObject,
    SimEnumeration,
    SimStruct,
    SimStatemachine,
    Document,
    Signal,
    SimSignal,
    Message,
    SimMessage,
    JSONMessage,
    Package,
    ModelPackage,
    LibraryPackage,
    ModulePackage,
    JSPackage,
    ExecPackage,
    ExternPackage,
    SimulationPackage,
    ProfilePackage,
    PHPPackage,
    Object,
    Link,
    Activity,
    Node,
    ActionNode,
    InitialNode,
    FinalNode,
    DecisionNode,
    MergeNode,
    ForkNode,
    JoinNode,
    Edge,
    Pin,
    Token,
    InterfaceClass,
    //
    //  Html Content Stereotypes
    HttpIfcLibrary,
    HtmlPageClass,
    //
    //
    WxFormsClass,
    //
    Requirement,
    Artifact,
    Note,
    UMLDiagram,
    AsciiDocPackage,
    RaASPackage,
    NoteLink,
    SubsystemPackage
};

typedef enum enumVisibility {
    vNone,
    vPackage,
    vPublic,
    vProtected,
    vPrivate
} eVisibility;

typedef enum enumAggregation {
    aNone,
    aShared,
    aComposition,
    aAggregation
} eAggregation;

enum class eNavigable {
    unspecified,
    yes,
    no
};

class MElement;

class MElementRef {
public:
    MElementRef() = default;
    inline MElementRef(const std::string& aId);
    inline MElementRef(std::shared_ptr<MElement> aPtr);
    inline MElementRef & operator=(const std::string& aId);
    inline MElementRef & operator=(std::shared_ptr<MElement> aPtr);
    ~MElementRef() = default;
    const std::string& Id() {return (mId);}

    inline std::shared_ptr<MElement> operator->() const ;

    std::shared_ptr<MElement> operator*() const {return operator->();}
    operator std::shared_ptr<MElement>() const { return operator->();}

    inline operator bool() const ;
    inline bool operator==(std::shared_ptr<MElement> aOther) const;
    bool operator!=(std::shared_ptr<MElement> aOther) const {return !(*this == aOther);}
    inline bool operator== (const std::string& aRef) const;
public:
    std::string                       mId;
    mutable std::shared_ptr<MElement> mElement;
};


template <typename E, typename C>
class tConnector : protected std::pair<std::shared_ptr<E>, std::shared_ptr<C>> {
public:
    tConnector(std::shared_ptr<E> aElement, std::shared_ptr<C> aConnector) {
        this->first  = aElement;
        this->second = aConnector;
    }
    std::shared_ptr<E> getElement() { return this->first; }
    std::shared_ptr<C> getConnector() { return this->second; }
};

class MElement
{
public:
    MElement();
    MElement(const std::string &aId, std::shared_ptr<MElement> aParent);
    virtual ~MElement() = default;
    void SetId(const std::string &aId) {id=aId;}
    void SetParent(MElementRef aParent) { parent = aParent;}
    bool AddTag(const std::string& name, const std::string&value) {return (tags.insert(std::pair<std::string, std::string>(name, value))).second;}
    virtual std::string FQN(void) const;
    virtual void Prepare(void) ;
    virtual void Dump(std::shared_ptr<MModel> aModel) ;
    virtual void Add(std::shared_ptr<MElement> aElement);

    virtual void Delete(std::shared_ptr<MElement> aElement);
    std::list<std::string> GetComment(size_t length = 90);
    void DumpComment(std::ostream& output, int spacer=0, size_t length=90, std::string commentintro="/*", std::string commentcontinue=" *", std::string commentend=" */");
    bool HasStereotype(const std::string& aName) const {return (stereotypes.find(helper::tolower(aName))!=stereotypes.end());}
    bool HasTaggedValue(const std::string& aName) const;
    std::string GetTaggedValue(const std::string& aName) const;
    inline bool IsClassBased() const;
    inline bool IsPackageBased() const;
    inline bool IsNodeBased() const;
    std::string getModelPath() ;
    std::list<std::shared_ptr<MElement>> getOwned(eElementType aType);

    template <class T>
            std::shared_ptr<T> sharedthis() {
                auto ptr = MElement::Instances[id];

                if (!ptr) {
                    std::cerr << "Cannot find pointer for " << id << std::endl;
                }
                return std::dynamic_pointer_cast<T>(ptr);
            }

public:
    static std::map<std::string, std::shared_ptr<MElement>> Instances;
    eElementType                                            type;
    MElementRef                                             parent;
    std::string                                             id;
    std::string                                             name;
    std::string                                             mModelPath;
    std::string                                             comment;
    eVisibility                                             visibility;
    std::vector<tConnector<MElement, MElement>>             Supplier;
    std::vector<tConnector<MElement, MElement>>             Client;
    std::map<std::string, std::string>                      tags;
    std::list<MElementRef>                                  owned;
    std::map<std::string, std::shared_ptr<MStereotype>>     stereotypes;
    std::vector<MElementRef>                                mCollaboration;
    //
    //  Some specials for modellers that do not sort their result on reading.
    int64_t                                                 mPosition = -1;
    //
    //  Creation and modification dates are interesting as well.
    std::string                                             mCreationDate;
    std::string                                             mModificationDate;
    std::string                                             mAlias;
};



inline MElementRef::MElementRef(const std::string & aId) {
    mId = aId;

    auto element = MElement::Instances.find(aId);

    if (element != MElement::Instances.end()) {
        mElement = element->second;
    }
}

inline std::shared_ptr<MElement> MElementRef::operator->() const {
    if (mElement == nullptr) {
        auto element = MElement::Instances.find(mId);

        if (element != MElement::Instances.end()) {
            mElement = element->second;
        }
    }
    return mElement;
}

inline MElementRef::operator bool() const {
    return operator->().get() != nullptr;
}

inline bool MElementRef::operator==(std::shared_ptr<MElement> aOther) const {
    if (mElement.get() == aOther.get()) {
        return true;
    }
    return false;
}

inline bool MElementRef::operator==(const std::string &aRef) const {
    if (mId == aRef) {
        return true;
    }
    return false;
}

inline MElementRef::MElementRef(std::shared_ptr<MElement> aPtr) : MElementRef(aPtr->id) {

}

inline MElementRef &MElementRef::operator=(const std::string &aId) {
    mId = aId;

    auto element = MElement::Instances.find(aId);

    if (element != MElement::Instances.end()) {
        mElement = element->second;
    }
    return *this;
}

inline MElementRef &MElementRef::operator=(std::shared_ptr<MElement> aPtr) {
    if (aPtr) {
        mId = aPtr->id;
    }
    mElement = aPtr;
    return *this;
}


inline bool MElement::IsClassBased() const {
    switch (type) {
    case eElementType::SimObject:
    case eElementType::SimEnumeration:
    case eElementType::ModuleClass:
    case eElementType::CxxClass:
    case eElementType::CClass:
    case eElementType::Class:
    case eElementType::Enumeration:
    case eElementType::Struct:
    case eElementType::SimMessageClass:
    case eElementType::SimSignalClass:
    case eElementType::Union:
    case eElementType::QtClass:
    case eElementType::ExternClass:
    case eElementType::JSClass:
    case eElementType::PrimitiveType:
    case eElementType::Collaboration:
    case eElementType::PHPClass:
    case eElementType::DataType:
    case eElementType::HtmlPageClass:
    case eElementType::InterfaceClass:
    case eElementType::WxFormsClass:
        return (true);
    default:
        return (false);
    }
}

inline bool MElement::IsPackageBased() const {
    switch(type) {
    case eElementType::LibraryPackage:
    case eElementType::ModulePackage:
    case eElementType::ExecPackage:
    case eElementType::SimulationPackage:
    case eElementType::JSPackage:
    case eElementType::ExternPackage:
    case eElementType::PHPPackage:
    case eElementType::Package:
    case eElementType::ModelPackage:
    case eElementType::AsciiDocPackage:
    case eElementType::HttpIfcLibrary:
        return true;
    default:
        return false;
    }
}

inline bool MElement::IsNodeBased() const {
    switch(type) {
    case eElementType::Node:
    case eElementType::ActionNode:
    case eElementType::InitialNode:
    case eElementType::FinalNode:
    case eElementType::DecisionNode:
    case eElementType::MergeNode:
    case eElementType::ForkNode:
    case eElementType::JoinNode:
    case eElementType::Activity:
        return true;
    default:
        break;
    }
    return false;
}


#endif // MELEMENT_H
