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
#ifndef __EXTENSIONLIBS_FILESYSTEM_LINUXFILE_CPATH_INC
#define __EXTENSIONLIBS_FILESYSTEM_LINUXFILE_CPATH_INC
// **************************************************************************
#define PATH_DELIMITER '/'

#define PATH_DELIMITER_STRING "/"
#define PATH_DELIMITER_DOUBLE "//"
#define PATH_THIS_DIR         "./"
#define PATH_UPPER_DIR        "../"

#include <vector>
#include <string>

// **************************************************************************
//                             F o r w a r d s
// **************************************************************************
// **************************************************************************
//                  C l a s s    d e c l a r a t i o n
// **************************************************************************
class CPath {
public:
    CPath();
    CPath(const std::string aPath);
    ~CPath();
    void Setup(const std::string aPath);
    void SetBase(const std::string aBase);
    void SetExtension(const std::string aExt);
    size_t DirDepth();
    void ReduceDepth();
    std::string Directory();
    std::string Base();
    std::string Extension();
    operator std::string();
    std::string operator=(const std::string& aPath);
    std::string operator=(const char* aPath);
    
    CPath operator+(const std::string aMore);
    CPath operator+(const char c);
    bool operator==(const char* aB);
    bool operator!=(const char* aB) { return !operator==(aB);}
    void Create();
    std::string operator[](size_t a_index) {
        std::string retval;

        if (a_index < Dirs.size()) {
            retval = Dirs[a_index];
        }
        return retval;
    }
private:
    std::string BaseName;
    std::string Ext;
    std::vector<std::string> Dirs;
};
#endif // __EXTENSIONLIBS_FILESYSTEM_LINUXFILE_CPATH_INC
