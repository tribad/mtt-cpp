//
// Copyright 2020 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#include "mconnector.h"
#include "cconnector.h"

std::shared_ptr<MConnector> MConnector::construct(const std::string&aId, std::shared_ptr<MElement> aParent)
{
    auto retval = new CConnector(aId, aParent);

    return retval->sharedthis<MConnector>();
}

std::shared_ptr<MAssociationEnd> MConnector::ThisEnd(const std::string &ref) {
    std::shared_ptr<MAssociationEnd> retval;

    if (ends[0] && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[0])->Classifier == ref)) {
        retval = std::dynamic_pointer_cast<MAssociationEnd>(*ends[0]);
    } else if (ends[1] && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[1])->Classifier == ref)) {
        retval = std::dynamic_pointer_cast<MAssociationEnd>(*ends[1]);
    }
    return (retval);
}

std::shared_ptr<MAssociationEnd> MConnector::OtherEnd(const std::string &ref) {
    std::shared_ptr<MAssociationEnd> retval;

    if (ends[0] && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[0])->Classifier == ref)) {
        retval = std::dynamic_pointer_cast<MAssociationEnd>(*ends[1]);
    } else if (ends[1] && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[1])->Classifier == ref)) {
        retval = std::dynamic_pointer_cast<MAssociationEnd>(*ends[0]);
    }
    return (retval);
}

std::shared_ptr<MAssociationEnd> MConnector::OtherEnd(const std::shared_ptr<MAssociationEnd> thisend) {
    std::shared_ptr<MAssociationEnd> retval;

    if (ends[0] == thisend) {
        retval = std::dynamic_pointer_cast<MAssociationEnd>(*ends[1]);
    } else {
        retval = std::dynamic_pointer_cast<MAssociationEnd>(*ends[0]);
    }
    return retval;
}

