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
#include "helper.h"
#include "mattribute.h"
#include "cattribute.h"
#include "mmessage.h"
#include "msimmessage.h"
#include "mclass.h"
#include "csimmessage.h"
#include "mlifeline.h"
#include "clifeline.h"
#include "cmessageclass.h"

std::string CSimMessage::FQN() const {
    return name;
}

void CSimMessage::Prepare(void) {
    if (source) {
        std::dynamic_pointer_cast<CLifeLine>(*source)->SetOutgoing(sharedthis<MElement>());
    }
    if (target) {
       std::dynamic_pointer_cast<CLifeLine>(*target)->SetIncoming(sharedthis<MElement>());
    }
}

std::shared_ptr<MClass> CSimMessage::GetClass() {

    if (!Class) {
        auto ci = MClass::mByFQN.find(name);

        if (ci != MClass::mByFQN.end()) {
            Class = ci->second;
        }
    }
    return Class;
}

void CSimMessage::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}
