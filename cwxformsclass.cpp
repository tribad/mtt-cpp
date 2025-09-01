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

#include "helper.h"
#include "cgeneralization.h"
#include "cwxformsclass.h"

CWxFormsClass::CWxFormsClass(const std::string&aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {
    type = eElementType::WxFormsClass;
}


void CWxFormsClass::SetFromTags(const std::string& name, const std::string&value)
{
    std::string tname = helper::tolower(name);

    if (tname == "filename") {
        mWxFormsFile = value;
    }
}

void CWxFormsClass::Prepare() {
    PrepareBase();
    if (mWxFormsFile.empty()) {
        mWxFormsFile = name;
    }
}

void CWxFormsClass::Dump(std::shared_ptr<MModel> aModel) {
    //CCxxClass::Dump(aModel);
}

std::string CWxFormsClass::getHeaderFile() {
    return mWxFormsFile;
}

std::string CWxFormsClass::getFileName() {
    return mWxFormsFile;
}
