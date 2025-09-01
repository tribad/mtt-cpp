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
#include "massociation.h"
#include "cassociation.h"

std::map<std::string, std::shared_ptr<MAssociation>> MAssociation::Instances;

MAssociation::MAssociation()
{

}

MAssociation::MAssociation(const std::string&aId, std::shared_ptr<MElement> aParent) : MElement(aId, aParent) {
    type=eElementType::Association;
    MAssociation::Instances.insert(std::pair<std::string, std::shared_ptr<MAssociation>>(id, sharedthis<MAssociation>()));
}


std::shared_ptr<MAssociation> MAssociation::construct(const std::string&aId, std::shared_ptr<MElement> aParent)
{
    auto retval = new CAssociation(aId, aParent);

    return retval->sharedthis<MAssociation>();
}

std::shared_ptr<MAssociationEnd> MAssociation::ThisEnd(const std::string &ref) {
    if (!ends.empty() && (ends[0]) && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[0])->Classifier == ref)) {
        return (std::dynamic_pointer_cast<MAssociationEnd>(*ends[0]));
    } else if ((ends.size() > 1) && (ends[1]) && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[1])->Classifier == ref)) {
        return (std::dynamic_pointer_cast<MAssociationEnd>(*ends[1]));
    }
    return {};
}

std::shared_ptr<MAssociationEnd> MAssociation::OtherEnd(const std::string &ref) {
    if ((ends.size() > 1) && (ends[0]) && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[0])->Classifier == ref)) {
        return (std::dynamic_pointer_cast<MAssociationEnd>(*ends[1]));
    } else if ((ends.size() > 1) && (ends[1]) && (std::dynamic_pointer_cast<MAssociationEnd>(*ends[1])->Classifier == ref)) {
        return (std::dynamic_pointer_cast<MAssociationEnd>(*ends[0]));
    }
    return {};
}

std::shared_ptr<MAssociationEnd> MAssociation::OtherEnd(std::shared_ptr<MAssociationEnd> thisend) {
    if (ends[0] == thisend) {
        return std::dynamic_pointer_cast<MAssociationEnd>(*ends[1]);
    }
    return std::dynamic_pointer_cast<MAssociationEnd>(*ends[0]);
}

