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

#ifndef XMI2CODE_NAMESPACE_H
#define XMI2CODE_NAMESPACE_H

#include <vector>
#include <string>

class NameSpace {
public:
    NameSpace() = default;
    NameSpace(const std::vector<std::string> & aNameList);
    NameSpace(const std::string& aNameSpace);
    std::string diff(const std::string& aFqn)  const;
    const std::vector<std::string>& get() const {return mNameSpace;}
    const std::string& getString() const {return mNameSpaceString;}
    void add(const std::string& aNameSpace) {mNameSpace.push_back(aNameSpace);fillstring();}
    void prepend(const NameSpace& other) {mNameSpace.insert(mNameSpace.begin(), other.get().begin(), other.get().end());fillstring();}
    bool empty() const {return mNameSpace.empty();}
    void clear() {mNameSpace.clear();}
    size_t size() const {return mNameSpace.size();}
    NameSpace combine(const NameSpace& other);
    operator std::string() { return mNameSpaceString;}
    const std::string& operator[](size_t index);
private:
    void fillstring();
    bool isSpecial(char c);
private:
    static std::string       mEmpty;
    std::vector<std::string> mNameSpace;
    std::string              mNameSpaceString;
};


#endif //XMI2CODE_NAMESPACE_H
