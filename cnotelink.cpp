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
#include "cnotelink.h"

std::string CNoteLink::FQN() const {
    return name;
}

void CNoteLink::Prepare(void) {
   //
   //  We do not expect that source and target are switch while
   //  drawing the other way round. But this may change.
   //
   //  We may not got the target
   if (target) {
       if (target->type == eElementType::Note) {
           target->Client.emplace_back(src, sharedthis<MElement>());
       } else {
           target->Supplier.emplace_back(src, sharedthis<MElement>());
       }
   }
   //
   //  We may not got the source
   if (src) {
        if (src->type == eElementType::Note) {
            src->Client.emplace_back(target, sharedthis<MElement>());
        }
        else {
            src->Supplier.emplace_back(target, sharedthis<MElement>());
        }
   }
}

void CNoteLink::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}
