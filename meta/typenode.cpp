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
#include "main.h"
#include "typenode.h"
#include "cclassbase.h"

constexpr size_t constspacesize = 6;

void TypeNode::clear() {
    mNameSpace.clear();
    mName.clear();
    mExtension = TypeExtension::None;
    mParameter.clear();
}

bool TypeNode::setupComplete() {
    bool complete = false;
    if (mClassifier) {
        complete = true;
        for (auto & p : mParameter) {
            complete = p.setupComplete();
            if (!complete) {
                break;
            }
        }
    }
    return complete;
}

std::string TypeNode::getFQN(bool aConstLeft, bool aReadOnly) const {
    std::string retval;
    std::string ns = getFullScope();

    if (aConstLeft && (mConst || aReadOnly)) {
        retval = "const " + ns + mName;
    } else {
        retval += ns + mName;
    }
    if (mTemplateType) {
        std::string delimiter = "<";
        for (auto &p: mParameter) {
            retval += delimiter + p.getFQN(aConstLeft);
            if (p.mFunction == TypeFunction::ReturnType) {
                delimiter = "(";
            } else {
                delimiter = ", ";
            }
            if (p.mFunction == TypeFunction::LastParameterType) {
                retval+=')';
            }
        }
        if (!mParameter.empty()) {
            retval += '>';
        }
        if (!mMember.empty()) {
            retval += "::" + mMember;
        }
    }
    if (!aConstLeft && (mConst || aReadOnly)) {
        retval+=" const";
    }
    retval.append(mPointerDereference, '*');
    if (mExtension == TypeExtension::Reference) {
        retval += '&';
    } else if (mExtension == TypeExtension::Moveable) {
        retval += "&&";
    }
    if (mHasPack) {
        retval += "...";
    }
    return retval;
}

void TypeNode::fill(const std::string& aClassNameSpace) {
    std::string storedName;

    if (mNameSpace.getString().empty()) {
        storedName = mName;
    } else {
        storedName = mNameSpace.getString() + "::" + mName;
    }

    if (!mClassifier) {
        auto matchedClass = MClass::findBestMatch(storedName, aClassNameSpace);
        mClassifier = matchedClass;
    }

    if (mClassifier) {
        mScope = std::dynamic_pointer_cast<MClass>(*mClassifier)->getEnclosingScope();
    }

    for (auto &p: mParameter) {
        p.fill(aClassNameSpace);
    }
}


TypeNode TypeNode::parse(const char *&last) {
    parseState state = parseState::start;
    std::string ident;
    bool        fncnotation = false;
    TypeNode    result;

    while (*last != '\0') {
        switch (state) {
            case parseState::start:
                if (!isDelimiter(*last)) {
                    ident.push_back(*last);
                    state = parseState::intype;
                }
                if (*last != '\0') {
                    last++;
                } else {
                    break;
                }
                break;
            case parseState::intype:
                if (isStartOfParameterList(*last)) {
                    state = parseState::inparameterlist;
                    result.mTemplateType = true;
                } else if (isEndOfParameter(*last)) {
                    //
                    // We are done with the type here, anyways this is the end of the parameter list
                    // or any parameter is following.
                    state = parseState::done;
                    if (*last == '>') {
                        //
                        //  If this is the end of the parameter list we do not skip the character.
                        //  We need the character wo be seen in the parameter list state
                        break;
                    } else if ((*last == '(') || (*last == ')')) {
                        //
                        // If this is a function delimiter do not skip it. so that it can be analysed within the parameter list.
                        break;
                    }
                } else {
                    ident.push_back(*last);
                }
                if (*last != '\0') {
                    last++;
                } else {
                    break;
                }
                break;
            case parseState::inparameterlist:
                //
                //  Check if this is the end of the parameter list or some more parameters are following.
                if (isEndOfParameterList(*last)) {
                    //
                    //  As last points to the character that leads to leaving the parameter parser we need to move it on.
                    //  If we at the end of the parameter list we make the member-check on the next character.
                    state = parseState::membercheck;
                } else {
                    //
                    //  Recursive call to parse the parameter.
                    result.mParameter.push_back(parse(last));
                    if (fncnotation) {
                        result.mParameter.rbegin()->mFunction = TypeFunction::ParameterType;
                    }
                    //
                    //  Check if this is the end of the parameter list or some more parameters are following.
                    if (isEndOfParameterList(*last)) {
                        //
                        //  As last points to the character that leads to leaving the parameter parser we need to move it on.
                        //  If we at the end of the parameter list we make the member-check on the next character.
                        state = parseState::membercheck;
                    } else {
                        //
                        //  Check for function notation.
                        if (*last == '(') {
                            fncnotation = true;
                            result.mParameter.rbegin()->mFunction = TypeFunction::ReturnType;
                        } else if (*last == ')') {
                            fncnotation = false;
                            result.mParameter.rbegin()->mFunction = TypeFunction::LastParameterType;
                        }
                    }
                }
                if (*last != '\0') {
                    last++;
                } else {
                    break;
                }
                break;
            case parseState::membercheck:
                //
                //  check if this is the member start.
                //  if it is not we are in type and do not move the pointer on to the next character.
                if (*last == ':') {  //  Member start.
                    state = parseState::memberSecondColon;
                    result.setName(ident);
                    ident.clear();
                } else {
                    state = parseState::intype;
                    break;                          //  Skip the movement of the character.
                }
                if (*last != '\0') {
                    last++;
                } else {
                    break;
                }
                break;
            case parseState::memberSecondColon:
                if (*last == ':') {  //  Member start.
                    state = parseState::inMember;
                } else {
                    std::cerr << "Error in member separation\n";
                    state = parseState::done;
                }
                if (*last != '\0') {
                    last++;
                } else {
                    break;
                }
                break;
            case parseState::inMember:
                if (isEndOfParameter(*last)) {
                    if (!ident.empty()) {
                        result.mMember = ident;
                        ident.clear();
                    }
                    state = parseState::done;

                    if (*last == '>') {
                       break;
                    } else {
                    }
                } else {
                    ident.push_back(*last);
                }
                if (*last != '\0') {
                    last++;
                } else {
                    break;
                }
                break;
            case parseState::done:
                if (!ident.empty()) {
                    result.setName(ident);
                    ident.clear();
                }
                return result;
            default:
                break;
        }
    }
    if (state == parseState::inMember) {
        if (!ident.empty()) {
            result.mMember = ident;
        }
    } else {
        if (!ident.empty()) {
            result.setName(ident);
        }
    }
    return result;
}


void TypeNode::setName(const std::string& aName) {
    std::string thename = aName;
    std::string tname;  //  The typename without any namespace.
    size_t nsstart = 0;
    //
    //  check for const keyword.
    size_t constpos = thename.find("const ");
    //
    //  Check if const is at the beginning. No const& possible
    if ((constpos != std::string::npos) && (constpos == 0)) {
        nsstart = constspacesize;
        auto s = thename.begin() + constspacesize;
        //
        // Skip any blanks.
        while ((s != thename.end()) && (*s == ' ')) {
            nsstart++;
            s++;
        }
        mConst = true;
    }
    //
    //  Check for pack ...
    size_t packpos = thename.find("...");
    if (packpos != std::string::npos) {
        //
        // Set the flag and remove it from the name
        mHasPack = true;
        thename.erase(packpos, 3);
    }
    //
    //  Split the namespace from the typename.
    //  We search the last occuring :: expecting right the typename and left of it the namespace.
    size_t nspos = thename.find_last_of("::");
    //
    if (nspos != std::string::npos) {
        //
        //  If we find a :: we setup the name and ns from it.
        mNameSpace = thename.substr(nsstart,nspos+1);
        tname      = thename.substr(nspos+1);
    } else {
        //
        //  No namespace in the name.
        tname = thename.substr(nsstart);
    }
    //
    //  check if behind the class name a const can befound.
    constpos = tname.find(" const");
    //
    //  check if const is any part of the name.
    if (constpos != std::string::npos) {
        //
        // check if const is part of the name or namespace or if it realy ment to be the compiler keyword.
        mConst = true;
        //
        //  remove the const from the end.
        tname  = tname.erase(constpos, constspacesize);
    }
    //
    //  Now check for reference extensions
    //  Starting with simple pointer references
    for (auto c = tname.begin(); c != tname.end();) {
        if (*c == '*') {
            mPointerDereference++;
            c = tname.erase(c);
        } else {
            ++c;
        }
    }
    //
    //  Check for move
    size_t movepos = tname.find("&&");
    if (movepos != std::string::npos) {
        mExtension = TypeExtension::Moveable;
        tname.erase(movepos, 2);
    } else {
        //
        //  Check for reference
        size_t refpos = tname.find('&');
        if (refpos != std::string::npos) {
            mExtension = TypeExtension::Reference;
            tname.erase(refpos, 1);
        }
    }
    mName = tname;
}

std::string TypeNode::getNameSpace() const {
    auto c = std::dynamic_pointer_cast<MClass>(*mClassifier);
    std::string ns;

    if (c) {
        ns = c->mNameSpace.getString();
    } else {
        ns = mNameSpace.getString();
    }
    return ns;
}

std::string TypeNode::getFullScope() const {
    auto c = std::dynamic_pointer_cast<MClass>(*mClassifier);
    std::string ns;

    if (c) {
        ns = c->mNameSpace.getString();
    } else {
        ns = mNameSpace.getString();
    }
    if (!ns.empty()) {
        ns += "::";
    }
    if (!mScope.empty()) {
        ns += mScope + "::";
    }

    return ns;
}

