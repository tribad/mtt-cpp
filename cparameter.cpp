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
#include "melement.h"
#include "mparameter.h"
#include "cparameter.h"

#include "mclass.h"

std::string CParameter::FQN() const {
    return name;
}

void CParameter::Prepare(void) {
    for (auto & t : tags) {
        if (t.first=="Qualifier") {
            mQualifier=t.second;
            auto position = mQualifier.find_first_of(':');
            if (position != std::string::npos) {
                mQualifierName = mQualifier.substr(0, position);
                mQualifierType = mQualifier.substr(position+1);
            } else {
                mQualifierName = mQualifier;
            }
        } else {
        }
    }


    if (!Classifier) {
        auto ci = MClass::mByFQN.find(ClassifierName);
        if (ci != MClass::mByFQN.end()) {
            Classifier = ci->second->id;
        }
    }
}

void CParameter::Dump(std::shared_ptr<MModel> model) {

}
