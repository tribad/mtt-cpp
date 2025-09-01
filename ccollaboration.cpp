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
#include "mclass.h"
#include "cclassbase.h"
#include "cclass.h"
#include "ccollaboration.h"

#include "minteraction.h"
#include "cinteraction.h"

CCollaboration::CCollaboration()
{

}

void CCollaboration::SetFromTags(const std::string& name, const std::string&value)
{
}


std::string CCollaboration::FQN() const {
    return name;
}

void CCollaboration::Prepare(void) {
    for (auto & in : mInteraction) {
        if (in != nullptr) {
            for (auto & msg : std::dynamic_pointer_cast<CInteraction>(*in)->messages) {
                msg->Prepare();
            }
            for (auto & ll : std::dynamic_pointer_cast<CInteraction>(*in)->lifelines) {
                ll->Prepare();
            }
        }
    }
}

void CCollaboration::Dump(std::shared_ptr<MModel> model) {
    (void) model;
}
