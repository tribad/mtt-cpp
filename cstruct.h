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
#ifndef CSTRUCT_H
#define CSTRUCT_H


//class CStruct : public CClassBase
class CStruct : public CCxxClass
{
public:
    CStruct() = default;
    CStruct(const std::string& aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {has_hdr = ".h";type = eElementType::Struct; mClassifierType = "struct";}
    ~CStruct() override = default;
#if 0
    void SetFromTags(const std::string& name, const std::string&value) override;
    //
    //  Virtuals from MElement
    std::string FQN(void) const override;
    void Prepare(void) override;
    void Dump(std::shared_ptr<MModel> aModel) override;
private:
    std::string lower_name;
    std::string upper_name;
#endif
};

#endif // CSTRUCT_H
