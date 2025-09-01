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
#include "melement.h"

#include "mattribute.h"
#include "cattribute.h"

#include "cenumeration.h"

void CEnumeration::SetFromTags(const std::string& name, const std::string&value)
{
}

std::string CEnumeration::FQN() const {
    return name;
}

void CEnumeration::Prepare(void) {
    PrepareBase();
}

void CEnumeration::DumpClassDecl(std::ostream& file, int indent) {
    std::string classfiller;         // This filler is static and used to indent the whole dump.
    classfiller.assign(indent, ' ');

    file << classfiller << "//\n";
    DumpComment(file, 0, 120, "//", "//", "");
    file << classfiller << "enum class " << name ;
    
    if (!Base.empty()) {
        auto cb = std::dynamic_pointer_cast<CClassBase>(Base.begin()->getElement());
        file << " : " << cb->mTypeTree.getFQN();
    } else {
        for (auto & s : Supplier) {
            if ((s.getElement()->IsClassBased()) && s.getConnector()->HasStereotype("use")) {
                auto cb = std::dynamic_pointer_cast<CClassBase>(s.getElement());
                file << " : " << cb->mTypeTree.getFQN();
            }
        }
    }

    file << " {\n";
    //
    // Search the max sizes for default values and names.
    size_t      maxdefault = 0;
    size_t      maxname    = 0;
    std::string namefiller;     // For aligning anything that is dumped behind the name.
    std::string defaultfiller;  // For aligning anything that is dumped behind the default value.

    for (auto& i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        if (a->name.size() > maxname) {
            maxname = a->name.size();
        }
        if (a->defaultValue.size() > maxdefault) {
            maxdefault = a->defaultValue.size();
        }
    }
    //
    //  No dump the enumeration literals with default value and comment.
    for (auto& i : Attribute) {
        auto a = std::dynamic_pointer_cast<CAttribute>(*i);
        auto e = a->GetComment();
        //
        //  If we have a multiline comment we dump it before the literal.
        if (e.size() > 1) {
            file << "    //\n";
            for (auto const& ec : e) {
                file << classfiller << "    // " << ec << std::endl;
            }
        }
        //
        //  Dump the name, start of line. This is always the right way.
        file << classfiller << "    " << a->name;
        //
        //  We need this filler if we have a default value or a one line comment.
        //  This filler is used to align the content behind the name.
        namefiller.assign(maxname - a->name.size() + 1, ' ');
        //
        //  With a default value we dump the value assignment.
        if (!a->defaultValue.empty()) {
            file << namefiller << "= " << a->defaultValue;
            //
            // If it is the last literal we do not dump the ',' behind the default value
            if (i != *std::prev(Attribute.end())) {
                file << ',';
            }
        } else {
            //
            // If it is the last literal we do not dump the ',' behind the name
            if (i != *std::prev(Attribute.end())) {
                file << ',';
            }
        }
        //
        //  Check if we have a one line comment.
        if (e.size() == 1) {
            //
            //  We have any default value. So we need to dump the default value alignment.
            if (maxdefault != 0)  {
                size_t fillersize = (i == *std::prev(Attribute.end()));
                if (a->defaultValue.empty()) {
                    fillersize += maxdefault + 2;
                    defaultfiller.assign(fillersize  , ' ');
                    file << defaultfiller << namefiller;
                } else {
                    fillersize += maxdefault - a->defaultValue.size();
                    defaultfiller.assign(fillersize  , ' ');
                    file << defaultfiller ;
                }
            }
            file << "// " << *e.begin();
        }
        file << std::endl;
    }
    file << classfiller << "};\n";
}


void CEnumeration::DumpNameSpaceIntro(std::ostream &file) {
    for (const auto & ni : mNameSpace.get()) {

        file << "namespace " << ni << std::endl << "{" << std::endl;
    }
}

void CEnumeration::DumpNameSpaceClosing(std::ostream &file) {
    if (!mNameSpace.get().empty()) {
        file << std::endl;
        for (auto ni = mNameSpace.get().rbegin(); ni != mNameSpace.get().rend(); ni++) {
            file << "} // namespace - " << *ni << std::endl;
        }
    }
}


void CEnumeration::Dump(std::shared_ptr<MModel> model) {

    auto cmodel = std::dynamic_pointer_cast<CModel>(model);

    DumpBase(cmodel, name);
    DumpFileHeader(hdr, name, ".h");
    DumpGuardHead(hdr, name);
    DumpNameSpaceIntro(hdr);
    DumpClassDecl(hdr, 0);
    DumpNameSpaceClosing(hdr);

    DumpGuardTail(hdr, name);
    CloseStreams();
}
