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
#include "mpackage.h"

#include "cpackagebase.h"

std::map<std::string, std::shared_ptr<MPackage>> MPackage::Instances;

std::shared_ptr<MPackage> MPackage::construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent)
{
    return CPackageBase::construct(aId, aStereotype, aParent);
}

std::shared_ptr<MPackage> MPackage::construct(const std::string&aId, std::string aPackageType,  std::shared_ptr<MElement> aParent)
{
    return CPackageBase::construct (aId, aPackageType, aParent);
}

std::string MPackage::GetDefaultStereotype() {
    std::string result;

    result = GetTaggedValue("DefaultStereotype");
    if ((parent) && (result.empty())) {
        switch (parent->type) {
        case eElementType::Package:
        case eElementType::SimulationPackage:
        case eElementType::LibraryPackage:
        case eElementType::ExecPackage:
            result= std::dynamic_pointer_cast<MPackage>(*parent)->GetDefaultStereotype();
            break;
        default:
            break;
        }
    }
    return (result);
}

void MPackage::Add(std::shared_ptr<MElement> aElement) {
    if (aElement->IsClassBased()) {
        Add(std::dynamic_pointer_cast<MClass>(aElement));
    } else if (aElement->IsPackageBased()) {
        Add(std::dynamic_pointer_cast<MPackage>(aElement));
    } else if (aElement->type == eElementType::Object) {
        Add(std::dynamic_pointer_cast<MObject>(aElement));
    } else {
        owned.emplace_back(aElement);
    }
}


void MPackage::Delete(std::shared_ptr<MElement> aElement) {
    if (aElement->IsClassBased()) {
        for (auto i = Classes.begin(); i != Classes.end();++i) {
            if (*i == aElement) {
                Classes.erase(i);
                break;
            }
        }
    
    } else if (aElement->IsPackageBased()) {
        for (auto i = Packages.begin(); i != Packages.end();++i) {
            if (*i == aElement) {
                Packages.erase(i);
                break;
            }
        }
    } else if (aElement->type == eElementType::Object) {
        for (auto i = Objects.begin(); i != Objects.end();++i) {
            if (*i == aElement) {
                Objects.erase(i);
                break;
            }
        }
    }
    MElement::Delete(aElement);
}
