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

#include "massociationend.h"
#include "cassociationend.h"

CAssociationEnd::CAssociationEnd()
{
    isMultiple = false;
}

std::string CAssociationEnd::FQN() const {
    if (Classifier) {
        return (Classifier->FQN());
    }
    return "<undefined>";
}

void CAssociationEnd::Prepare(void) {
    size_t  position;
    //
    // check the tagged value only if the qualifier in the assoc end is empty.
    if (mQualifier.empty()) {
        for (auto const & t : tags) {
            if (t.first == "Qualifier") {
                mQualifier = t.second;
            }
        }
    }
    //
    // Now process the mQualifier if set.
    if (!mQualifier.empty()) {
        position = mQualifier.find_first_of(':');
        if (position != std::string::npos) {
            QualifierName = helper::trim(mQualifier.substr(0, position));
            QualifierType = helper::trim(mQualifier.substr(position+1));
        } else {
            QualifierName = mQualifier;
        }
    }
    if ((Multiplicity == "0..*") || (Multiplicity == "1..*") ||
         (Multiplicity == "*")) {
        isMultiple   = true;
        isCollection = true;
    } else if ((Multiplicity.empty()) || (Multiplicity == "1") || (Multiplicity == "0..1")) {
        isMultiple   = false;
        isCollection = false;
    } else if ((!Multiplicity.empty())) {
        isMultiple   = true;
        isCollection = false;
    }
}

void CAssociationEnd::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}
