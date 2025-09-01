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

#include "namespace.h"

std::string NameSpace::mEmpty;

NameSpace::NameSpace(const std::vector<std::string> &aNameList) : mNameSpace (aNameList) {
    fillstring();
}

NameSpace::NameSpace(const std::string &aNameSpace) {
    int           state     = 0;
    std::string   ident;
    const char*   ci = aNameSpace.c_str();
    char          c;

    while (*ci != '\0') {
        c = *ci;
        switch (state) {
            case 0:     // start of some item must start with a alpha.
                if (!isSpecial(c)) {
                    state = 1;
                    ident.push_back(c);
                }
                break;
            case 1:     //  now wait until end. can be any alnum
                if ((!isSpecial(c)) || (c == '_')) {
                    ident.push_back(c);
                } else if (c == ':') {
                    if (!ident.empty()) {
                        mNameSpace.push_back(ident);
                    }
                    state           = 2;      //  Wait for other colon
                    ident.clear();
                } else {
                    state = -1;
                }
                break;
            case 2:     //  Wait for second colon.
                if (c == ':') {
                    state = 0;
                } else {
                    state = -1;   //  Abort because of syntax error.
                }
                break;
            default:
                break;
        }
        if (state == -1) {
            ident.clear();
            break;
        } else {
            ci++;
        }
    }
    if (!ident.empty()) {
        mNameSpace.push_back(ident);
    }
    std::string delimiter;

    for (auto & ns : mNameSpace) {
        mNameSpaceString += delimiter + ns;
        delimiter = "::";
    }
}

std::string NameSpace::diff(const std::string &aFqn) const {
    std::string retval = aFqn;

    for (auto ns : mNameSpace) {
        size_t position = retval.find("::");

        if (position != std::string::npos) {
            std::string part = retval.substr(0, position);

            if (part == ns) {
                retval.erase(0, position+2);
            }
        }
    }
    return retval;
}

void NameSpace::fillstring() {
    bool first = true;

    for (auto ns : mNameSpace) {
        if (!first) {
            mNameSpaceString += "::";
            mNameSpaceString += ns;
        } else {
            mNameSpaceString = ns;
            first = false;
        }
    }
}

bool NameSpace::isSpecial(char c) {
    bool result = false;

    if ((c == '*') || (c == '&') ||
        (c == '<') || (c== '>')  ||
        (c == ',') || (c == ':')) {
        result = true;
    }
    return result;
}

NameSpace NameSpace::combine(const NameSpace &other) {
    std::vector<std::string> retval;

    if (!mNameSpace.empty() && !other.get().empty()) {
        size_t overlap_size = 0U;
        auto mine_last = mNameSpace.back();
        //
        //  initialize the retval from the two namespaces.
        retval.insert(retval.begin(), mNameSpace.begin(), mNameSpace.begin() + mNameSpace.size());
        retval.insert(retval.end(), other.get().begin(), other.get().end());

        do {
            overlap_size++;
            //
            //  Setup o_tmp
            std::vector<std::string> o_tmp(mNameSpace.size() - overlap_size, "");
            o_tmp.insert(o_tmp.end(), other.get().begin(), other.get().end());

            size_t pos;
            for (pos = 0; pos < overlap_size; ++pos) {
                //
                //  Check that posistion is not outside the other namespace size.
                if ((pos < other.size()) &&(mNameSpace[mNameSpace.size() - overlap_size + pos] != other.get()[pos])) {
                    break;
                }
            }
            //
            //  Check if all matched.
            if (pos == overlap_size) {
                retval.clear();
                //
                //  create the retval from the two namespaces.
                retval.insert(retval.begin(), mNameSpace.begin(), mNameSpace.begin() + mNameSpace.size() - overlap_size);
                retval.insert(retval.end(), other.get().begin(), other.get().end());
            }
        } while(overlap_size < mNameSpace.size());
    } else {
        //
        // The result contains the sum of the namespaces.
        retval = mNameSpace;
        retval.insert(retval.end(), other.get().begin(), other.get().end());
    }
    return NameSpace(retval);
}

const std::string &NameSpace::operator[](size_t index) {
    if (index < mNameSpace.size()) {
        return mNameSpace[index];
    }
    return mEmpty;
}
