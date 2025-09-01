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
#include <set>
#include <iomanip>

#include "helper.h"
#include "crc64.h"
#include "cclassbase.h"
#include "csimenumeration.h"

#include "mmodel.h"
#include "cmodel.h"

#include "mattribute.h"
#include "cattribute.h"

extern long simversion;

void CSimEnumeration::SetFromTags(const std::string& name, const std::string&value)
{
    if (name == "BaseName") {
        basename=value;
    }
}


std::string CSimEnumeration::FQN() const {
    return name;
}

void CSimEnumeration::Prepare(void) {
    PrepareBase();
    //
    //  Create the Basename and the lowercase version.
    if (basename.empty()) {
        basename=name;
    }
    lower_basename=helper::tolower(basename);
    upper_basename=helper::toupper(basename);
}

void CSimEnumeration::Dump(std::shared_ptr<MModel> model) {
    std::list<std::pair<std::string, std::string> > alist;
    size_t                                          alistmax = 0;
    Crc64                                           crc;
    DumpBase(std::dynamic_pointer_cast<CModel>(model), name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, basename);
    //
    //  Create the list of attributes and the values.
    for (auto & i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string defvalue;

        defvalue = "IDE_"+helper::toupper(helper::normalize(a->name.substr(1)));
        if (a->defaultValue.empty()) {
            id_map.insert(std::pair<std::string, uint64_t>(defvalue, crc.calc(defvalue)));
            alist.emplace_back( a->name, defvalue);
        } else {
            id_map.insert(std::pair<std::string, uint64_t>(defvalue, strtoul(a->defaultValue.c_str(), 0, 0)));
            alist.emplace_back( a->name, a->defaultValue);
        }
    }
    //
    //  Find the max size of the attribute name.
    for (auto & ilist: alist) {
        if (ilist.first.size()>alistmax) {
            alistmax = ilist.first.size();
        }
    }
    //
    //  Dump the enumerator macros
    //  We need to go along the attributes themselfs because we need to access
    //  the defaultValue that may be set for an attribute.
    for (auto & i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        std::string defvalue;

        defvalue = "IDE_"+helper::toupper(helper::normalize(a->name.substr(1)));
        std::string filler;
        filler.assign((alistmax+4)-defvalue.size()+1, ' ');

        if (a->defaultValue.empty()) {
            hdr << "#ifndef " << defvalue << "\n";
            hdr << "#define " << defvalue << filler << "(0x" << std::hex << std::setw(16) << std::setfill('0') << crc.calc(defvalue) << ")\n";
            hdr << "#endif\n\n";
        } else {
            hdr << "#ifndef " << defvalue << "\n";
            hdr << "#define " << defvalue << filler << "( " << a->defaultValue << ")\n";
            hdr << "#endif\n\n";
        }
    }

    hdr << "//\n";
    hdr << "//                   S i m E n u m e r a t i o n    d e c l a r a t i o n\n";
    if (simversion == 1) {
        hdr << "typedef enum __" << name << " {";

        for (auto & ilist : alist) {
            std::string filler;
            filler.assign(alistmax-ilist.first.size()+1, ' ');
            if (ilist == *alist.begin()) {
                hdr << "\n    " << ilist.first << filler << " = " << ilist.second;
            } else {
                hdr << ",\n    " << ilist.first << filler << " = " << ilist.second;
            }
        }
        hdr << "\n} " << name << ";\n";
        hdr << "\n";
    } else if (simversion == 2) {
        hdr << "struct " << name << " {\n";
        for (auto & ilist : alist) {
            std::string filler;

            filler.assign(alistmax-ilist.first.size()+1, ' ');
            hdr << "    static const uint64_t " << ilist.first << filler << " = " << ilist.second << ";\n";
        }
        hdr << "};\n";
        hdr << "\n";
    }
    DumpGuardTail(hdr, basename);

    CloseStreams();
}
