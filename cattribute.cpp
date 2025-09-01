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

#include "mconnector.h"
#include "cconnector.h"

#include "cclassbase.h"
#include "mattribute.h"
#include "cattribute.h"
#include "mclass.h"

std::string CAttribute::FQN(void) const {
    std::string fqn;

    if (!Classifier) {
        fqn = ClassifierName;
    } else {
        fqn = Classifier->FQN();
    }
    return fqn;
}

void CAttribute::Prepare(void) {
    size_t                                             position;
    std::map<std::string, std::string>::const_iterator i;

    for ( auto &  i : tags) {
        if (i.first == "Qualifier") {
            Qualifier = i.second;
            position = Qualifier.find_first_of(':');
            if (position != std::string::npos) {
                QualifierName = Qualifier.substr(0, position);
                QualifierType = Qualifier.substr(position+1);
            } else {
                QualifierName = Qualifier;
            }
        } else {
        }
    }
    if ((Multiplicity == "0..*") || (Multiplicity == "1..*") ||
            (Multiplicity == "*")) {
        isMultiple = true;
        isCollection = true;
    } else if ((Multiplicity.empty()) || (Multiplicity=="1") || (Multiplicity == "0..1")) {
        isMultiple = false;
        isCollection = false;
    } else if ((!Multiplicity.empty())) {
        isMultiple = true;
        isCollection = false;
    }

    if ((!Classifier) && (!ClassifierName.empty())) {
        auto ci = MClass::mByFQN.find(ClassifierName);
        //
        //  Found a classifier by name
        if (ci != MClass::mByFQN.end()) {
            Classifier = ci->second->id;
        }
    } else if (Classifier && ClassifierName.empty()) {
        ClassifierName = Classifier->name;
    }
    for (auto & cc : connector) {
        //
        //  As prepare is virtual we should not need any cast operation here.
        cc->Prepare();
    }
}

void CAttribute::Dump(std::shared_ptr<MModel> model) {

}
