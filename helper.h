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

#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <list>
#include <algorithm>


#define HELPER_PATH_MAX (32768)

namespace helper
{
    std::string toupper(const std::string& text);
    inline std::string tolower(const std::string& text);
    std::string normalize(const std::string& text);
    std::string escape(const std::string& text);
    std::string escapeHTML(const std::string & text);
    std::string fromHTML(const std::string & text);
    std::list<std::pair<std::string, std::string> > typelist(std::string cname);
    std::list<std::string> tokenize(std::string line, const char* delimiter=" \t");
    std::string trim(const std::string& line);

	std::string getcwd();
	int mkdir(const std::string& aPath, int aMode, bool aParent = true);
	int chdir(const std::string& aPath);
    int rmdir(const std::string& aPath);
    std::string homedir();
    std::string toDash(const std::string& aIn) ;


};


inline std::string helper::tolower(const std::string &text) {
    std::string lower;

    lower.resize(text.size());
    std::transform(text.begin(), text.end(), lower.begin(), ::tolower);

    return lower;
}


#endif // HELPER_H
