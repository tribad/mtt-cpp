//
// Copyright 2019 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include "token.h"
#include "melement.h"
#include "mnode.h"

std::shared_ptr<MNode> tToken::validate() {
    std::shared_ptr<MNode> retval;
    //
    //  All pathes must end on the same node.
    //  For input-pins we do check the Node the pin is mounted on.
    for (auto pl : path) {
        if (!pl.empty()) {
            if (pl.back()->IsNodeBased()) {
                if (retval == nullptr) {
                    retval = std::dynamic_pointer_cast<MNode>(*pl.back());
                } else {
                    if (retval != *pl.back()) {
                        //
                        //  Error we break the loop
                        std::cerr << "Path validation on node failed\n";
                        return nullptr;
                    }
                }
            } else {
                if (pl.back()->type == eElementType::Pin) {
                    if (pl.back()->parent->IsNodeBased()) {
                        if (retval == nullptr) {
                            retval = std::dynamic_pointer_cast<MNode>(*pl.back()->parent);
                        } else {
                            if (pl.back()->parent != retval) {
                                //
                                //  Error we break the loop
                                std::cerr << "Path validation on pin failed\n";
                                return nullptr;
                            }
                        }
                    }
                }
            }
        }
    }
    return retval;
}
