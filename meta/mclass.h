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
#ifndef MCLASS_H
#define MCLASS_H

#include <vector>

#include "melement.h"
#include "mmessage.h"
#include "mattribute.h"
#include "typenode.h"

class MAssociation;
class MAssociationEnd;
class MAttribute;
class MStatemachine;
class MParameter;
class MOperation;
class MDependency;
class MGeneralization;
class MActivity;
class MInteraction;

class MClass : public MElement
{
public:
    MClass() = default;
    MClass(const std::string&aId, std::shared_ptr<MElement> aParent);
    static std::shared_ptr<MClass> construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent = nullptr);
    static std::shared_ptr<MClass> construct(const std::string&aId, std::string aPackageType, std::shared_ptr<MElement> aParent = nullptr);
    void Add(std::shared_ptr<MAssociation> a);
    inline void Add(std::shared_ptr<MAttribute> a);
    inline std::string getTypeName();
    eElementType getPackageType();
    std::string GetDefaultStereotype();
    std::string getEnclosingScope();
    void add();
    static std::list<std::shared_ptr<MClass>> findBestMatch(const std::string& aName);
    static std::shared_ptr<MClass> findBestMatch(const std::string& aName, const std::string& aClassNameSpace);
public:
    static std::map<std::string, std::shared_ptr<MClass>> Instances;
    static std::map<std::string, std::shared_ptr<MClass>> mByFQN;
    static std::map<std::string, std::shared_ptr<MClass>> mByReverseFQN;
    static std::map<std::string, std::shared_ptr<MClass>> mByModelPath;
public:
    std::vector<MElementRef> mSubClass;
    std::vector<MElementRef> mSuperClass;
    bool                     mIsInterface = false;
    bool                     mIsFinal     = false;
    TypeNode                 mTypeTree;
    NameSpace                mNameSpace;

    std::map<int, std::shared_ptr<MParameter>> mClassParameter;
    std::vector<MElementRef> Attribute;
    std::vector<MElementRef> Operation;
    std::vector<MElementRef> Association;
    std::vector<MElementRef> OwnEnd;
    std::vector<MElementRef> OtherEnd;
    std::vector<MElementRef> Incoming;
    std::vector<MElementRef> Outgoing;
    std::vector<MElementRef> Dependency;
    std::vector<MElementRef> Generalization;
    std::vector<MElementRef> Activity;
    std::vector<MElementRef> mInteraction;
    MElementRef              statemachine;
};


inline void MClass::Add(std::shared_ptr<MAttribute> a) {
    Attribute.emplace_back(a);
}

inline std::string MClass::getTypeName() {
    std::string retval = mTypeTree.getFQN();

    return retval;
}


#endif // MCLASS_H
