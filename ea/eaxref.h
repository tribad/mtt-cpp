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

#ifndef EAXREF_H
#define EAXREF_H

#include <string>

#include "variant.h"
#include "melement.h"

class xrefValue {
public:
    std::string          mName;
    std::string          mValue;
    std::list<xrefValue> mStruct;
};

class eaXref
{
public:
    eaXref();
    virtual ~eaXref();
    virtual void setupDescription(const std::string& aDescription);
    static eaXref* construct(const std::string& aType);

    virtual tVariant get(const std::string& name);
private:
    virtual std::list<xrefValue> setup(std::string& aDescription);
public:
    std::string mXrefId;
    std::string mName;
    std::string mType;
    eVisibility mVisibility;
    std::string mNamespace;
    std::string mRequirement;
    std::string mConstraint;
    std::string mBehavior;
    std::string mPartition;
    xrefValue   mDescription;
    std::string mClient;
    std::string mSupplier;
    std::string mLink;
};

class elementXref : public eaXref {
public:
    ~elementXref() override;
    std::list<xrefValue> setup(std::string& aDescription) override;
    tVariant get(const std::string& name) override;
};

class packageXref : public eaXref {
public:
    ~packageXref() override;
    std::list<xrefValue> setup(std::string &aDescription) override;
    tVariant get(const std::string& name) override;
};

class parameterXref : public eaXref {
public:
    ~parameterXref() override;
    std::list<xrefValue> setup(std::string& aDescription) override;
    tVariant get(const std::string& aName) override;
};


class ownedXref : public eaXref {
public:
    ~ownedXref() override;
    void setupDescription(const std::string& aDescription) override;
    tVariant get(const std::string& name) override;
};


#endif // EAXREF_H
