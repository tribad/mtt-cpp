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
#include "mobject.h"
#include "cobject.h"

std::map<std::string, std::shared_ptr<MObject>> MObject::Instances;

MObject::MObject()
{
    type = eElementType::Object;
    MObject::Instances.insert(std::pair<std::string, std::shared_ptr<MObject>>(id, sharedthis<MObject>()));
}

MObject::MObject(const std::string&aId, std::shared_ptr<MElement> aParent) : MElement(aId, aParent) {
    type = eElementType::Object;
    MObject::Instances.insert(std::pair<std::string, std::shared_ptr<MObject>>(aId, sharedthis<MObject>()));
}

std::shared_ptr<MObject> MObject::construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent)
{
    auto retval = new CObject(aId, aParent);

    return retval->sharedthis<MObject>();
}
