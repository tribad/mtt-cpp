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

#include <iostream>
#include "helper.h"
#include "eaxref.h"

eaXref::eaXref()
{

}

eaXref::~eaXref()
{

}

std::list<xrefValue> eaXref::setup(std::string &aDescription) {
    std::list<xrefValue>   retval;
    xrefValue           entry;

    entry.mName  = "noname";
    entry.mValue = aDescription;

    retval.push_back(entry);
    return retval;
}

void eaXref::setupDescription(const std::string &aDescription) {
    std::string desc = aDescription;

    mDescription.mName   = ".";
    mDescription.mStruct = setup(desc);
}


eaXref* eaXref::construct(const std::string &aType) {
    eaXref*     retval = nullptr;
    std::string      t = helper::tolower(aType);

    if (t == "element property") {
        retval = new elementXref;
    } else if (t == "package property") {
        retval = new packageXref;
    } else if (t == "parameter property") {
        retval = new parameterXref;
    } else if (t == "ownedelement property") {
        retval = new ownedXref;
    } else if (t == "connector property") {
        retval = new elementXref;
    } else {
        retval = new eaXref;
    }
    if (retval != nullptr) {
        retval->mType = aType;
    }
    return retval;
}

tVariant eaXref::get(const std::string &name) {
    tVariant retval;

    (void)name;
    return retval;
}

elementXref::~elementXref()
{

}


std::list<xrefValue> elementXref::setup(std::string &aDescription) {
    std::list<xrefValue>   retval;
    xrefValue           entry;

    //
    //  The element properties are split along the ;
    size_t startpos = 0;
    size_t endpos;

    while ((endpos = aDescription.find_first_of(';', startpos)) != std::string::npos) {
        std::string parameter = aDescription.substr(startpos, endpos-startpos);
        //
        // check if it is a name=value pair.
        size_t equalposition = parameter.find_first_of('=');
        if (equalposition == std::string::npos) {
            //
            // No equal sign. So storing it as name only.
            entry.mName = parameter;
            entry.mValue.clear();
            retval.push_back(entry);
        } else {
            entry.mName = parameter.substr(0, equalposition);
            equalposition++;
            if (equalposition < parameter.size()) {
                entry.mValue = parameter.substr(equalposition);
            } else {
                entry.mValue.clear();
            }
            retval.push_back(entry);
        }

        startpos = endpos+1;
        if (startpos >= aDescription.size()) {
            break;
        }
    }

    return retval;
}


tVariant elementXref::get(const std::string &name) {
    tVariant retval;

    (void)name;
    return retval;
}


packageXref::~packageXref()
{

}

std::list<xrefValue> packageXref::setup(std::string& aDescription) {
    std::list<xrefValue>   retval;
    xrefValue           entry;

    entry.mName  = "noname";
    entry.mValue = aDescription;

    retval.push_back(entry);
    return retval;
}

tVariant packageXref::get(const std::string &name) {
    tVariant retval;

    (void)name;
    return retval;
}


ownedXref::~ownedXref()
{

}

void ownedXref::setupDescription(const std::string &aDescription) {
    std::string desc = aDescription;

    mDescription.mName   = "ClassifierRef";
    mDescription.mValue = aDescription;
}


tVariant ownedXref::get(const std::string &name) {
    tVariant retval;

    (void)name;
    return mDescription.mValue;
}


parameterXref::~parameterXref()
{

}

#if 0
@PROP=
        @NAME=lower
        @ENDNAME;
        @TYPE=Integer
        @ENDTYPE;
        @VALU=1
        @ENDVALU;
        @PRMT=
        @ENDPRMT;
@ENDPROP;
@PROP=
        @NAME=upper
        @ENDNAME;
        @TYPE=UnlimitedNatural
        @ENDTYPE;
        @VALU=15
        @ENDVALU;
        @PRMT=
        @ENDPRMT;
@ENDPROP;
#endif

std::list<xrefValue> parameterXref::setup(std::string &aDescription) {
    std::list<xrefValue>   retval;
    xrefValue              entry;

    int                    state = 0;
    std::string            name;
    std::string            endname;
    std::string            value;

    while (aDescription.begin() != aDescription.end()) {
        switch (state) {
        case 0:  //  wait on start of name
            if (*aDescription.begin() == '@') {  //  start of name detected.
                state = 1;
                value.clear();
                name.clear();
                endname.clear();
                entry.mName.clear();
                entry.mValue.clear();
            } else {
                //
                //  Ignore character.
            }
            break;
        case 1:  // search end of name
            if (*aDescription.begin() == '=') { //  end of name
                state = 2;   //  value follows.
                entry.mName = name;
            } else if(*aDescription.begin() == ';') {   // end of substruct.
                return retval;                          //  quick exit.
            } else {
                name.push_back(*aDescription.begin());
            }
            break;
        case 2: //  now value or substructure follows or end of
            if (*aDescription.begin() == '@') {  //  substructure follows.
                std::string str;
                std::string cmp;

                str = "@END" + name + ';';
                cmp = aDescription.substr(0, str.size());

                if (str != cmp) {    // check if value
                    entry.mStruct = setup(aDescription);
                    state = 0;
                    retval.push_back(entry);
                } else {             // is end of value.
                    state = 0;
                    retval.push_back(entry);
                    aDescription.erase(0, str.size()-1);
                }
            } else {          //  not @ so it is a value without means.
                state = 3;    //  read value
                value.push_back(*aDescription.begin());
            }
            break;
        case 3:
            if (*aDescription.begin() == '@') {  //  this should be the start of the endname
                state = 4;    //  process endname
                entry.mValue = value;
            } else {
                value.push_back(*aDescription.begin());
            }
            break;
        case 4:   // process endname
            if (*aDescription.begin() == ';') {   //  end of endname
                state = 0;
                retval.push_back(entry);
            } else {
                endname.push_back(*aDescription.begin());
            }
            break;
        default:
            break;
        }

        aDescription.erase(aDescription.begin());
    }

    return retval;
}


tVariant parameterXref::get(const std::string &aName) {
    tVariant retval;
    std::string value;
    std::string name;
    //
    //  Loop along the props.
    for (auto & p : mDescription.mStruct) {
        //
        //  Loop along the values.
        for (auto & v : p.mStruct) {
            if (helper::tolower(v.mName) == "valu") {
                value = v.mValue;
            } else if (helper::tolower(v.mName) == "name") {
                name = v.mValue;
            }
        }
        if (name == aName) {
            retval = value;
            break;
        }
    }
    return retval;
}
