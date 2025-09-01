//
// Copyright 2023 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#pragma once

#ifndef TYPENODE_H
#define TYPENODE_H

#include <vector>
#include <string>
#include "namespace.h"
#include "melement.h"
#include "namespace.h"

class MClass;

enum class parseState {
    start,             // wait for first alpha.
    intype,            // wait for end of type.
    inparameterlist,   // in list
    membercheck,       // check if a member follows
    memberSecondColon, // wait for the second colon before the member.
    inMember,          // now we are in the member name.
    done               // done parsing the type
};

enum class TypeExtension {
    None,
    Reference,
    Moveable
};

enum class TypeFunction {
    None,
    ReturnType,
    ParameterType,
    LastParameterType
};

class TypeNode {
public:
    TypeNode()  = default;
    ~TypeNode() = default;
    //
    //  This is the typical start for creating a typetree. Expecting only
    //  a single type before running down into the parameters.
    //TypeNode(const std::string& aTypeText);
    //
    //  Create a typenode from the classbased
    //TypeNode(std::shared_ptr<MClass> aClassifier);
    //
    // Assignment operator.
    TypeNode & operator=(const TypeNode&) = default;
    //
    //  Check if a forward is needed.
    bool NeedsForward() const {return (mExtension != TypeExtension::None);}
    //
    //  Check if this is a composite type.
    bool isCompositeType() const { return (mTemplateType || mConst || (mExtension != TypeExtension::None) || mPointerDereference);}
    //
    //  Check if all Classifiers are set.
    bool setupComplete();
    //
    //  create the FQN and return it.
    std::string getFQN(bool aConstLeft = true, bool aReadOnly = false) const;
    //
    //  Get the namespace
    std::string getNameSpace() const ;
    //
    //  Get the full scope. So for encapsulated classes.
    std::string getFullScope() const ;
    //
    //  fill the type tree from the existing types.
    //  This maybe done multiple times before the type tree is completed.
    //  It depends on the order of the creation of the dependenies.
    //  This order cannot be guaranteed while reading the model from the model-storage.
    void fill(const std::string& aClassNameSpace);
    //
    //  This is the recursive parser used within creating the type tree.
    static TypeNode parse(const char* &last);
    //
    //  parsing from a name
    static inline TypeNode parse(const std::string& aName);
    //
    //  setName does not only set the name but splits up the namespace and any other special information
    //  like pointer/reference/move info.
    void setName(const std::string& aName);
private:
    void clear();
    static inline  bool isStartOfParameterList(char c);
    static inline bool isEndOfParameter(char c);
    static inline bool isEndOfParameterList(char c);
    static inline bool isDelimiter(char c);
public:
    std::string           mName;
    MElementRef           mClassifier;
    NameSpace             mNameSpace;                                 // namespace from the name
    std::string           mScope;                                     // For inner classes the scope they are in.
    TypeExtension         mExtension          = TypeExtension::None;  // * & &&
    int                   mPointerDereference = 0;                    // Number of asterisks.
    TypeFunction          mFunction           = TypeFunction::None;
    bool                  mConst              = false;
    bool                  mTemplateType       = false;
    bool                  mTypeNameKeyword    = false;
    bool                  mHasPack            = false;                // This is the ... on a template parameter.
    std::string           mMember;                                    // This is the member that may be defined behind the template parameter list.
    std::vector<TypeNode> mParameter;
};

inline bool TypeNode::isStartOfParameterList(char c) {
    return (c == '<') ;
}

inline bool TypeNode::isEndOfParameter(char c) {
    return ((c == '>') || (c == '(') || (c == ')') || (c == ',') || (c == '\0'));
}

inline bool TypeNode::isEndOfParameterList(char c) {
    return (c == '>');
}

inline bool TypeNode::isDelimiter(char c) {
    return (isblank(c) || (c == ',') || c == '(');
}

inline TypeNode TypeNode::parse(const std::string &aName) {
    const char *s = aName.c_str();
    TypeNode  result = parse(s);

    //std::cerr << aName << "  ||  " << result.getFQN(false) << std::endl;

    return result;
}

#endif 

