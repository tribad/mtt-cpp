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
#include "namespacenode.h"


void NameSpaceNode::add(std::vector<std::string> a_namespace, std::shared_ptr<CClassBase> a_forward) {
    if (a_namespace.empty()) {
        mForwards.push_back(a_forward);
    } else {
        std::string ns = *a_namespace.begin();

        a_namespace.erase(a_namespace.begin());
        mChildren[ns].add(a_namespace, a_forward);
        mChildren[ns].mName = ns;
    }
}

void NameSpaceNode::dump(std::ostream& file) {
    for (auto & f : mForwards) {
        file << "class " << f->name << ";\n";
    }
    for (auto &c : mChildren) {
        file << "namespace " << c.first << "\n{\n";
        c.second.dump(file);
        file << "} // namespace - " << c.first << std::endl;
    }
}