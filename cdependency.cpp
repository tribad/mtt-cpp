//
// Copyright 2016 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include "cdependency.h"

std::string CDependency::FQN() const {
    return name;
}

void CDependency::Prepare(void) {
   //
   //  We may not got the target
   if (target) {
       bool already = false;
       for (auto & c : target->Client) {
           if (c.getElement() == *src) {
               already = true;
               break;
           }
       }
       if (!already) {
            target->Client.emplace_back(*src, MElement::Instances[id]);
       }
   }
   //
   //  We may not got the source
   if (src != nullptr) {
       bool already = false;
       for (auto & s : src->Supplier) {
           if (s.getElement() == *target) {
               already = true;
               break;
           }
       }
       if (!already) {
           src->Supplier.emplace_back(*target, MElement::Instances[id]);
       }
   }
}

void CDependency::Dump(std::shared_ptr<MModel> model) {

}
