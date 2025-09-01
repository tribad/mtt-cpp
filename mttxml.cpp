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
#include <cstring>
#include <string>
#include "mttxml.h"
#include "helper.h"

int MttXmlDoc::mInstanceCounter = 0;

std::shared_ptr<MttXmlNode> MttXmlNode::findChild(std::string const& aName) {
    std::shared_ptr<MttXmlNode> retval;
#ifdef __linux__
    xmlNode* children = node->children;

    while (children != nullptr) {
        if (children->type == XML_ELEMENT_NODE) {
            if (helper::tolower(std::string((const char*)(children->name))) == aName) {
                retval = std::make_shared<MttXmlNode>(children);
                break;
            }
        }
        children = children->next;
    }
#else
    CComPtr<IXMLDOMNodeList> pNode;
    CComBSTR                 nodeName;

    mNode->get_childNodes(&pNode);
    long nLen;
    pNode->get_length(&nLen);    // Child node list
    for (long index = 0; index != nLen; ++index) // Traverse
    {
        CComPtr<IXMLDOMNode> pCurNode;
        pNode->get_item(index, &pCurNode);

        pCurNode->get_nodeName(&nodeName);
        if (nodeName == aName.c_str()) {
            retval = std::make_shared<MttXmlNode>(pCurNode);
            break;
        }
     }
#endif
    return retval;
}

bool  MttXmlNode::boolProperty(std::string const& aName, bool aDefaultValue) {
    bool retval = aDefaultValue;
#ifdef __linux__
    if (node != nullptr) {
#if (LIBXML_VERSION >=20907)
        xmlChar* prop = xmlGetProp(node, (const xmlChar*)aName.c_str());
#else
        xmlChar* prop = xmlGetProp((xmlNodePtr)node, (const xmlChar*)name.c_str());
#endif
        if (prop != nullptr) {
            if (strcmp((const char*)prop, "true")==0) {
                retval = true;
            }
            xmlFree(prop);
        }
    }
#else
    CComPtr<IXMLDOMNamedNodeMap> pNode;
    CComBSTR                     nodeName;
    CComVariant                  nodeValue;

    mNode->get_attributes(&pNode);
    long nLen;
    pNode->get_length(&nLen);    // Child node list
    for (long index = 0; index != nLen; ++index) // Traverse
    {
        CComPtr<IXMLDOMNode> pCurNode;
        pNode->get_item(index, &pCurNode);

        pCurNode->get_nodeName(&nodeName);
        if (nodeName == aName.c_str()) {
            pCurNode->get_nodeTypedValue(&nodeValue);

            if (nodeValue.vt == VT_BSTR) {
                if (CComBSTR(nodeValue.bstrVal) == "true") {
                    retval = true;
                }
            }
            break;
        }
    }
#endif
    return retval;
}

long  MttXmlNode::longProperty(std::string const& aName, long aDefaultValue) {
    long retval = aDefaultValue;
#ifdef __linux__
    if (node != nullptr) {
#if (LIBXML_VERSION >=20907)
        xmlChar* prop = xmlGetProp(node, (const xmlChar*)aName.c_str());
#else
        xmlChar* prop = xmlGetProp((xmlNodePtr)node, (const xmlChar*)name.c_str());
#endif
        if (prop != nullptr) {
            retval = strtol((const char*)prop, 0, 0);

            xmlFree(prop);
        }
    }
#else
    CComPtr<IXMLDOMNamedNodeMap> pNode;
    CComBSTR                     nodeName;
    CComVariant                  nodeValue;

    mNode->get_attributes(&pNode);
    long nLen;
    pNode->get_length(&nLen);    // Child node list
    for (long index = 0; index != nLen; ++index) // Traverse
    {
        CComPtr<IXMLDOMNode> pCurNode;
        pNode->get_item(index, &pCurNode);

        pCurNode->get_nodeName(&nodeName);
        if (nodeName == aName.c_str()) {
            pCurNode->get_nodeTypedValue(&nodeValue);

            if (nodeValue.vt == VT_BSTR) {
                retval = strtol((const char*)nodeValue.bstrVal, 0, 0);
            }
            break;
        }
    }
#endif
    return retval;
}

std::string MttXmlNode::stringProperty(std::string const& aName, std::string const& aDefaultValue) {
    std::string retval = aDefaultValue;

#ifdef __linux__
    if (node != nullptr) {
#if (LIBXML_VERSION >=20907)
        xmlChar* prop = xmlGetProp(node, (const xmlChar*)aName.c_str());
#else
        xmlChar* prop = xmlGetProp((xmlNodePtr)node, (const xmlChar*)name.c_str());
#endif
        if (prop != nullptr) {
            retval = (const char*)prop;

            xmlFree(prop);
        }
    }
#else
    CComPtr<IXMLDOMNamedNodeMap> pNode;
    CComBSTR                     nodeName;
    CComVariant                  nodeValue;

    mNode->get_attributes(&pNode);
    long nLen;
    pNode->get_length(&nLen);    // Child node list
    for (long index = 0; index != nLen; ++index) // Traverse
    {
        CComPtr<IXMLDOMNode> pCurNode;
        pNode->get_item(index, &pCurNode);

        pCurNode->get_nodeName(&nodeName);
        if (nodeName == aName.c_str()) {
            pCurNode->get_nodeTypedValue(&nodeValue);
            CComBSTR value(nodeValue.bstrVal);

            if (nodeValue.vt == VT_BSTR) {
                int n = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
                if (n <= 0) {
                    break;
                }
                char* buffer = new char[n];
                WideCharToMultiByte(CP_UTF8, 0, value, -1, buffer, n, nullptr, nullptr);
                retval = buffer;
                delete [] buffer;
            }
            break;
        }
    }
#endif
    return retval;
}


std::string MttXmlNode::content() {
    std::string retval;
#if __linux__
    if (node != nullptr) {
        xmlNode *children = node->children;

        while (children != nullptr) {
            if ((children->type == XML_TEXT_NODE) || (children->type == XML_CDATA_SECTION_NODE)) {
                retval += std::string((const char *) (children->content));
            }
            children = children->next;
        }
    }
#else
    CComPtr<IXMLDOMNodeList> pNode;
    CComBSTR                 nodeName;
    CComVariant              nodeValue;

    mNode->get_childNodes(&pNode);
    long nLen;
    pNode->get_length(&nLen);    // Child node list
    for (long index = 0; index != nLen; ++index) // Traverse
    {
        CComPtr<IXMLDOMNode> pCurNode;
        pNode->get_item(index, &pCurNode);
        DOMNodeType nt;
        pCurNode->get_nodeType(&nt);

        if ((nt == NODE_TEXT) || (nt == NODE_CDATA_SECTION)) {
            pCurNode->get_nodeTypedValue(&nodeValue);
            CComBSTR value(nodeValue.bstrVal);

            if (nodeValue.vt == VT_BSTR) {
                int n = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
                if (n <= 0) {
                    break;
                }
                char* buffer = new char[n];
                WideCharToMultiByte(CP_UTF8, 0, value, -1, buffer, n, nullptr, nullptr);
                retval = buffer;
                delete [] buffer;
            }
        }
     }

#endif
    return retval;
}

MttXmlDoc::MttXmlDoc() {
#ifdef __linux__
#else
    std::lock_guard<std::mutex> lck(mInstanceLock);
    if (mInstanceCounter == 0) {
        mInstanceCounter++;
        CoInitialize(NULL);
    }
    mHandle = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mDoc));
    if (SUCCEEDED(mHandle))
    {
        // these methods should not fail so don't inspect result
        mDoc->put_async(VARIANT_FALSE);
        mDoc->put_validateOnParse(VARIANT_FALSE);
        mDoc->put_resolveExternals(VARIANT_FALSE);
    }
#endif
}

MttXmlDoc::~MttXmlDoc() {
    std::lock_guard<std::mutex> lck(mInstanceLock);
    mInstanceCounter--;
#ifdef _MSC_VER
    mDoc.Release();
    if (mInstanceCounter == 0) {
        CoUninitialize();
    }
#endif
}

bool MttXmlDoc::readFile(std::string const& aFileName, std::string const& aSchemeName ) {
    bool retval = false;
#ifdef __linux__
    const char* scheme = nullptr;

    if (!aSchemeName.empty()) {
        scheme = aSchemeName.c_str();
    }
    mDocument = xmlReadFile(aFileName.c_str(), scheme, 0);
    if (mDocument != nullptr) {
        retval = true;
    }
#else
    HRESULT result = S_OK;
    VARIANT_BOOL bSuccess = false;

    // Load it from a url/filename...
    result = mDoc->load(CComVariant(aFileName.c_str()), &bSuccess);
    // filePath = "./test.xml";
    // hr = iXMLDoc->load(CComVariant(filePath.c_str()), &bSuccess);
    if (SUCCEEDED(result)) {
        retval = true;
    }
#endif

    return retval;
}

std::shared_ptr<MttXmlNode> MttXmlDoc::getRoot() {
    std::shared_ptr<MttXmlNode> retval;

#ifdef __linux__
    if (mDocument != nullptr) {
        retval = std::make_shared<MttXmlNode>(xmlDocGetRootElement(mDocument));
    }
#else
    CComPtr<IXMLDOMNodeList> pNode;
    CComBSTR                 nodeName;

    mDoc->get_childNodes(&pNode);
    long nLen;
    pNode->get_length(&nLen);    // Child node list
    for (long index = 0; index != nLen; ++index) // Traverse
    {
        CComPtr<IXMLDOMNode> pCurNode;
        pNode->get_item(index, &pCurNode);

        pCurNode->get_nodeName(&nodeName);
        if (nodeName != "xml") {
            retval = std::make_shared<MttXmlNode>(pCurNode);
            break;
        }
    }
#endif
    return retval;
}

