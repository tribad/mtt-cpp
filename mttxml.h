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

#pragma once
#ifndef MTTXML
#define MTTXML

#include <string>
#include <memory>
#include <mutex>

#ifdef _MSC_VER
#include <MsXml6.h>
#include <atlcomcli.h>
#endif

#ifdef __linux__
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif


class MttXmlNode {
public:
#ifdef __linux__
#if (LIBXML_VERSION >=20907)
    MttXmlNode(xmlNode* aNode) : node(aNode) {};
#else
    MttXmlNode(xmlNodePtr aNode) : node(aNode) {};
#endif
#else
MttXmlNode(CComPtr<IXMLDOMNode> aNode) : mNode(aNode) {}
#endif
    std::shared_ptr<MttXmlNode> findChild(std::string const& aName);

    bool  boolProperty(std::string const& aName, bool aDefaultValue = false);
    long  longProperty(std::string const& aName, long aDefaultValue = 0L);
    std::string stringProperty(std::string const& aName, std::string const& aDefaultValue = "");

    std::string content();
private:
#ifdef __linux__
#if (LIBXML_VERSION >=20907)
     xmlNode *node;
#else
     xmlNodePtr node;
#endif
#else
    CComPtr<IXMLDOMNode>  mNode;
#endif
};


class MttXmlDoc {
public:
    MttXmlDoc();
    ~MttXmlDoc();
    bool readFile(std::string const& aFileName, std::string const& aSchemeName = "");
    std::shared_ptr<MttXmlNode> getRoot();
private:
    std::mutex mInstanceLock;
    static int               mInstanceCounter;
#ifdef __linux__
    xmlDoc* mDocument = nullptr;
#else
    HRESULT                  mHandle;
    CComPtr<IXMLDOMDocument> mDoc;
#endif
};

#endif
