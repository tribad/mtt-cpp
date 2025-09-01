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
#include "mlifeline.h"
#include "clifeline.h"
#include "mattribute.h"
#include "cattribute.h"
#include "mclass.h"

std::string CLifeLine::FQN() const {
    return name;
}

void CLifeLine::Prepare(void) {
}

void CLifeLine::Dump(std::shared_ptr<MModel> model) {
    (void) model;
}

void CLifeLine::SetIncoming(std::shared_ptr<MElement> e) {
    if (role) {
        std::shared_ptr<CAttribute> a=std::dynamic_pointer_cast<CAttribute>(*role);
        if (a->Classifier) {
            std::shared_ptr<MClass> c = std::dynamic_pointer_cast<MClass>(*a->Classifier);
            //
            //  Check if we already have the message as incoming in the vector.
            bool gotit = false;
            for (auto & i : c->Incoming) {
                if (i == e) {
                    gotit = true;
                    break;
                }
            }
            //
            //  Only add it if dont got it before.
            if (!gotit) {
                c->Incoming.emplace_back(e);
            }
        }
    }
}

void CLifeLine::SetOutgoing(std::shared_ptr<MElement> e) {
    if (role) {
        std::shared_ptr<CAttribute> a=std::dynamic_pointer_cast<CAttribute>(*role);
        if (a->Classifier) {
            std::shared_ptr<MClass> c = std::dynamic_pointer_cast<MClass>(*a->Classifier);
            //
            //  Check if we already have the message as incoming in the vector.
            bool gotit = false;
            for (auto & i : c->Outgoing) {
                if (i == e) {
                    gotit = true;
                    break;
                }
            }
            //
            //  Only add it if dont got it before.
            if (!gotit) {
                c->Outgoing.emplace_back(e);
            }
        }
    }
}
