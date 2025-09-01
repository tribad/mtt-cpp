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
#include <sys/stat.h>
#include <iostream>

#if !defined(__linux__)
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <cctype>
#include <algorithm>
#include <cstring>
#include "helper.h"
#include "path.h"

std::string helper::toupper(const std::string &text) {
    std::string upper;

    if (!text.empty()) {
        upper.resize(text.size());
        std::transform(text.begin(), text.end(), upper.begin(), ::toupper);
    }
    return upper;
}

std::string helper::normalize(const std::string &text) {
    std::string norm;
    std::string to_be_replaced("_.,;:/() -");

    for (size_t i=0; i<text.size(); ++i) {
        if (to_be_replaced.find_first_of(text[i]) != std::string::npos) {
            norm.push_back('_');
        } else {
            norm.push_back(text[i]);
        }
    }


    return norm;
}

std::string helper::escapeHTML(const std::string &text) {
    std::string retval = text;
    size_t position;

    while ((position = retval.find_first_of("&<>\"'")) != std::string::npos) {
        switch (retval[position]) {
        case '>':
            retval.replace(position, 1, "&gt;");
            break;
        case '<':
            retval.replace(position, 1, "&lt;");
            break;
        case '"':
            retval.replace(position, 1, "&quot;");
            break;
        case '\'':
            retval.replace(position, 1, "&apos;");
            break;
        case '&':
            retval.replace(position, 1, "&amp;");
            break;
        default:
            return retval;
        }
    }
    return retval;
}

std::string helper::fromHTML(const std::string & text) {
    static std::vector<std::string> html =        {"&gt;", "&lt;", "&quot;", "&apos;", "&amp;"};
    static std::vector<std::string> replacement = {">",    "<",    "\"",     "'",      "&"};
    std::string result = text;

    for (size_t h = 0 ; h < html.size(); ++h) {
        size_t position;

        while ((position = result.find(html[h])) != std::string::npos) {
            result.replace(position, html[h].size(), replacement[h]);
        }
    }

    return result;
}

std::string helper::escape(const std::string &text) {
    size_t      pos;
    std::string escaped;
    std::string to_be_replaced("\n\t\\\"\b\r");

    for (size_t i=0; i<text.size(); ++i) {
        pos = to_be_replaced.find_first_of(text[i]);
        if ( pos!= std::string::npos) {
            switch (text[i]) {
            case '\n':
                escaped.push_back('\\');
                escaped.push_back('n');
                break;
            case '\t':
                escaped.push_back('\\');
                escaped.push_back('t');
                break;
            case '\\':
                escaped.push_back('\\');
                escaped.push_back('\\');
                break;
            case '"':
                escaped.push_back('\\');
                escaped.push_back('"');
                break;
            case '\r':
                escaped.push_back('\\');
                escaped.push_back('r');
                break;
            case '\b':
                escaped.push_back('\\');
                escaped.push_back('b');
                break;
            default:
                escaped.push_back(text[i]);
                break;
            }
        } else {
            escaped.push_back(text[i]);
        }
    }


    return escaped;
}

std::list<std::pair<std::string, std::string> > helper::typelist(std::string cname) {
    int         state     = 0;
    int         paralevel = 0;
    std::string ident;
    std::string ns;
    std::string typ;
    std::list<std::pair<std::string, std::string> > retval;

    for (std::string::iterator ci=cname.begin(); ci != cname.end(); ++ci) {
        switch (state) {
        case 0:     // start of some item must start with a alpha.
            if (isalpha(*ci)) {
                state = 1;
                ident.push_back(*ci);
            }
            break;
        case 1:     //  now wait until end. can be any alnum
            if ((isalnum(*ci)) || ((*ci)== '*') || ((*ci) == '&') || ((*ci) == '_')) {
                ident.push_back(*ci);
            } else {
                if ((*ci)==':') {
                    ns = ident;
                    ident.clear();
                    state = 2;      //  Wait for other colon
                } else if ((*ci) == '<') {   //  Template parameter follow. Starting with state 0.
                    paralevel++;
                    state = 0;
                    typ   = ident;
                    ident.clear();

                    retval.push_back(std::pair<std::string, std::string>(ns, typ));
                    ns.clear();
                } else if ((*ci)==',') {  //  More parameter to templates.
                    state = 0;
                    typ = ident;
                    ident.clear();

                    retval.push_back(std::pair<std::string, std::string>(ns, typ));
                    ns.clear();
                } else if ((*ci) == '>') {
                    paralevel --;
                    state = 0;
                    typ   = ident;
                    ident.clear();

                    retval.push_back(std::pair<std::string, std::string>(ns, typ));
                    ns.clear();
                }

            }
            break;
        case 2:     //  Wait for second colon.
            if ((*ci) == ':') {
                state = 3;  //  the typename follows.
            } else {
                return retval;   //  Abort because of syntax error.
            }
            break;
        case 3:
            if (isalpha(*ci)) {
                ident.push_back(*ci);
                state = 4;   // more typename
            } else {
            }
            break;
        case 4:     //  More typename.
            if ((isalnum(*ci)) || ((*ci)== '*') || ((*ci) == '&') || ((*ci) == '_')) {
                ident.push_back(*ci);
            } else {
                if ((*ci) == '<') {   //  Template parameter follow. Starting with state 0.
                    paralevel++;
                    state = 0;
                    typ   = ident;
                    ident.clear();

                    retval.push_back(std::pair<std::string, std::string>(ns, typ));
                    ns.clear();
                } else if ((*ci)==',') {  //  More parameter to templates.
                    state = 0;
                    typ = ident;
                    ident.clear();

                    retval.push_back(std::pair<std::string, std::string>(ns, typ));
                    ns.clear();
                } else if ((*ci) == '>') {
                    paralevel --;
                    state = 0;
                    typ   = ident;
                    ident.clear();

                    retval.push_back(std::pair<std::string, std::string>(ns, typ));
                    ns.clear();
                }
            }
            break;
        default:
            break;
        }
    }
    if ((state == 4) || (state == 1)) {
        retval.push_back(std::pair<std::string, std::string>(ns, ident));
    }
    return retval;
}

std::list<std::string> helper::tokenize(std::string line, const char* delimiter) {
    std::list<std::string> retval;
    size_t                 start = 0;
    size_t                 lpos;

    while ((lpos = line.find_first_of(delimiter, start)) != std::string::npos) {
        std::string token = line.substr(start, lpos-start);

        token = trim(token);
        if (!token.empty()) {
            retval.push_back(token);
        }

        start = lpos+1;
    }
    if (start < line.size()) {
        std::string token = line.substr(start);

        token = trim(token);
        if (!token.empty()) {
            retval.push_back(token);
        }
    }

    return retval;
}

std::string helper::trim(const std::string& line) {
    std::string retval;
// User-Defined-Code:AAAAAAFxaDbcJg+TZEk=
    retval = line;
    while ((retval.size()>0) && (isspace(retval[0]))) {
        retval.erase(0,1);
    }
    while ((retval.size() > 0) && (isspace(retval[retval.size()-1]))) {
        retval.erase(retval.size()-1);
    }
// End-Of-UDC:AAAAAAFxaDbcJg+TZEk=
    return  (retval);
}
std::string helper::getcwd() {
	CPath retval;
	char  wd[HELPER_PATH_MAX];
	//
	//  Set the directory to the current working directory.
#if defined(WIN32) || defined(WIN64)
	 retval = _getcwd(wd, sizeof(wd));
#else
    retval = ::getcwd(wd, sizeof(wd));
#endif
	return retval;
}

int helper::mkdir(const std::string& aPath, int aMode, bool aParent) {
    int retval = -1;

    if (aParent) {
        CPath dir(aPath);
        std::string path;
        size_t i;

        for (i = 0; i < dir.DirDepth(); ++i) {
            path += dir[i];
            retval = mkdir(path, aMode, false);
            if ((retval != 0) && (errno != EEXIST)) {
                break;
            }
        }
        if (i == dir.DirDepth()) {
            retval = 0;
        }
    } else {
#ifdef __MINGW32__
        retval = ::mkdir(aPath.c_str());
        (void)aMode;
#elif defined(WIN32) || defined(WIN64)
        retval = _mkdir(aPath.c_str());
        (void)aMode;
#else
        retval = ::mkdir(aPath.c_str(), aMode);
#endif
    }
    return retval;
}

int helper::chdir(const std::string &aPath) {
    int retval = -1;

#ifdef __linux__
    retval = ::chdir(aPath.c_str());
#else
    retval = _chdir(aPath.c_str());
#endif

    return retval;
}

int helper::rmdir(const std::string& aPath) {
    int retval = -1;

#ifdef __MINGW32__
    retval = ::rmdir(aPath.c_str());
#elif defined(WIN32) || defined(WIN64)
    retval = _rmdir(aPath.c_str());
#else
    retval = ::rmdir(aPath.c_str());
#endif
    return retval;
}


std::string helper::homedir() {
    std::string retval;
#ifdef __linux__
    int            result;
    struct passwd  pwentry;
    struct passwd* pwresult;
    char           buffer[4096];

    result = getpwuid_r(geteuid(), &pwentry, buffer, sizeof(buffer), &pwresult);
    if ((result == 0) && (pwresult != 0)) {
        retval.replace(0, 1, pwresult->pw_dir);
    } else {
        retval.replace(0, 1, "/tmp/");
    }
#else

#endif
    return retval;
}

std::string helper::toDash(const std::string& aIn) {
    std::string retval = aIn;
    size_t      pos = 0;

    while ((pos = retval.find_first_of(" /")) != std::string::npos) {
        retval[pos] = '-';
    }
    return retval;
}
