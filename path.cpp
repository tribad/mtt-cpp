//
// Copyright 2011 Hans-Juergen Lange <hjl@simulated-universe.de>
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

// **************************************************************************
//                   E x t r a   I n c l u d e   L i s t
// **************************************************************************
#include <sys/stat.h>
#ifdef __linux__
#include <unistd.h>
#include <pwd.h>
#endif // __linux__
#include <sys/types.h>

#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "helper.h"
// **************************************************************************
//                   F i r s t   I n c l u d e   L i s t
// **************************************************************************
#include "path.h"
// **************************************************************************
//                   L a s t   I n c l u d e   L i s t
// **************************************************************************
// **************************************************************************
//                   N a m e s p a c e   L i s t
// **************************************************************************
// **************************************************************************
//
//  Method-Name       : CFileName()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CPath::CPath() {
    
}
// **************************************************************************
//
//  Method-Name       : CFileName()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CPath::CPath(const std::string aPath) {
    Setup(aPath);
}
// **************************************************************************
//
//  Method-Name       : ~CFileName()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CPath::~CPath() {
    
}
// **************************************************************************
//
//  Method-Name       : Setup()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CPath::Setup(const std::string aPath) {
    size_t      startpos;
    size_t      endpos;
    std::string subber;
    std::string path=aPath;
    //
    BaseName.clear();
    Ext.clear();
    Dirs.clear();
    //
    //  replace '/' with '\'
    //
    while (path.find('\\')!=std::string::npos) {
        path.replace(path.find('\\'), 1, 1,'/');
    }
    //  Re-interpret the leading ~
    if (aPath[0]=='~') {
        path = helper::homedir();
    } else {
    }
    //
    //  Remove double / except at the front.
    //
    //  We always search the complete string.
    //  But starting with position 1 we will always skip a double delimiter in front.
    while ((startpos = path.find(PATH_DELIMITER_DOUBLE, 1)) != std::string::npos) {
        path.replace(startpos, 2, "/");
    }
    //
    //  Create a list of dirs.
    startpos = 0;
    while ((endpos=path.find_first_of(PATH_DELIMITER, startpos)) != std::string::npos) {
        subber=path.substr(startpos, (endpos-startpos)+1);
        //
        //  Skip all "./" entries.
        if (subber != PATH_THIS_DIR) {
            if ((subber == PATH_UPPER_DIR) && (Dirs.size()>1)) {
                if (Dirs.back() != PATH_UPPER_DIR) {
                    Dirs.pop_back();
                } else {
                    Dirs.push_back(subber);
                }
            } else if ((subber == PATH_DELIMITER_STRING) && (Dirs.size() > 0)) {
            } else {
                Dirs.push_back(subber);
            }
        } //  Do nothing if we have a this dir.
        startpos=endpos+1;
    }
    //
    //  Check if the path only consists of directories.
    if (!path.empty()) {
        //
        // Now we do not need to check if this is a valid expression.
        //  Check the last character
        if (*path.rbegin() != PATH_DELIMITER) {
            //
            //  Have a filename. Create the base and the extension.
            subber=path.substr(startpos);
            endpos=subber.find_last_of('.');
            if (endpos != std::string::npos) {
                BaseName=subber.substr(0, (endpos));
                Ext=subber.substr(endpos+1);
            } else {
                BaseName=path.substr(startpos);
            }
        }
    }
}
// **************************************************************************
//
//  Method-Name       : SetBase()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CPath::SetBase(const std::string aBase) {
    BaseName=aBase;
}
// **************************************************************************
//
//  Method-Name       : SetExtension()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CPath::SetExtension(const std::string aExt) {
    Ext=aExt;
}
// **************************************************************************
//
//  Method-Name       : DirDepth()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
size_t CPath::DirDepth() {
    return (Dirs.size());
}
// **************************************************************************
//
//  Method-Name       : ReduceDepth()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CPath::ReduceDepth() {
    if (Dirs.size()>1) {
        Dirs.pop_back();
    } else {
    }
    
}
// **************************************************************************
//
//  Method-Name       : Directory()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
std::string CPath::Directory() {
    std::vector<std::string>::iterator i;
    std::string result;
    
    i=Dirs.begin();
    while (i!=Dirs.end()) {
        result=result+(*i);
        i++;
    }
    return (result);
    
}
// **************************************************************************
//
//  Method-Name       : Base()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
std::string CPath::Base() {
    return (BaseName);
}
// **************************************************************************
//
//  Method-Name       : Extension()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
std::string CPath::Extension() {
    return (Ext);
}
// **************************************************************************
//
//  Method-Name       : operator std::string()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CPath::operator std::string() {
    std::string re;
    std::vector<std::string>::iterator i;
    
    i=Dirs.begin();
    while (i != Dirs.end()) {
        re=re+(*i);
        i++;
    }
    if (BaseName.size()>0) {
        re=re+BaseName;
    }
    if (Ext.size()>0) {
        re=re+".";
        re=re+Ext;
    }
    return (re);
    
}
// **************************************************************************
//
//  Method-Name       : operator=()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
std::string CPath::operator=(const std::string& aPath) {
    Setup(aPath);
    return ((std::string)(*this));
}
// **************************************************************************
//
//  Method-Name       : operator=()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
std::string CPath::operator=(const char* aPath) {
    Setup(aPath);
    return ((std::string)(*this));
}
// **************************************************************************
//
//  Method-Name       : operator+()
//  Author            : 
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CPath CPath::operator+(const std::string aMore) {
    std::string newpath=(std::string)(*this)+aMore;
    return (CPath(newpath));
}
// **************************************************************************
//
//  Method-Name       : operator+()
//  Author            :
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CPath CPath::operator+(const char aMore) {
    std::string newpath=(std::string)(*this)+aMore;
    return (CPath(newpath));
}
// **************************************************************************
//
//  Method-Name       : operator==()
//  Author            : 
//  Creation-Date     : 20.02.2011
//  Modification-Date : 03.04.2021
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
bool CPath::operator==(const char* aB) {
    std::string s=(std::string)(*this);
    
    return (s==aB);
}

void CPath::Create() {
    //
    //  We go through the path starting with the first element in the dirs list.
    //  and check if the directory exists.
    //  if not we create the directory and
    //  Than we extend the path with the next directory and start all over again.
    int err;
    struct stat dirstat;
    std::string crpath;

    for (auto & p : Dirs) {
        crpath += p;

        err = stat(crpath.c_str(), &dirstat);
        if ((err == -1) && (errno==ENOENT)) {
            helper::mkdir(crpath.c_str(), 0777);
        } else {
        }
    }
}