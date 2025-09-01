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
#ifndef CMODEL_H
#define CMODEL_H

#include <iostream>
#include "mmodel.h"

typedef struct tagGenFile {
    std::string ofile;
    std::string id;
    std::string comment;
    std::string filetype;
} tGenFile;

class CModel :  public std::enable_shared_from_this<MModel>, public MModel
{
public:
    CModel();
    virtual ~CModel() = default;
    //
    //
    virtual void Prepare(void);
    virtual void Dump(void);
    //
    void Merge(void);
    void Merge(const std::string& gfile, const std::string& ofile, const std::string& lname, const std::string& comment);
    void MergeSysHeader(const std::string& gfile, const std::string& ofile, const std::string& comment, const std::string& a_id);
    void LoadLastGeneratedFiles(void);
    void DumpGeneratedFiles(void);
public:
    std::list<std::string>            pathstack;
    std::list< tGenFile >             generatedfiles;
    std::map<std::string, tGenFile >  lastgeneratedfiles;
public:
    std::string                       Directory;
    std::string                       License;
    std::string                       LicenseFile;
    std::string                       Copyright;
    std::string                       AppCoreVersion;
    std::string                       SQLDirectory;
    std::string                       TestEnvironment;
};

#endif // CMODEL_H
