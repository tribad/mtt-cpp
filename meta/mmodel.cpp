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
#include "mmodel.h"
#include "cmodel.h"

#include "helper.h"

std::shared_ptr<MModel> MModel::construct()
{
    auto newmodel = std::make_shared<CModel>();

    return newmodel;
}

void MModel::Add(std::shared_ptr<MStereotype> s)
{
    Stereotype.emplace_back(MElementRef(s->id));
}

std::shared_ptr<MStereotype> MModel::StereotypeById(const std::string& id)
{
    std::shared_ptr<MStereotype> retval;

    for (auto & stereotype : Stereotype) {
        if (stereotype->id == id) {
            retval = std::dynamic_pointer_cast<MStereotype>(*stereotype);
            break;
        }
    }
    return (retval);
}

std::shared_ptr<MStereotype> MModel::StereotypeByName(const std::string& name, const std::string& etype)
{
    std::shared_ptr<MStereotype> retval;

    for (auto & stereotype : Stereotype ) {
        std::shared_ptr<MStereotype> s = std::dynamic_pointer_cast<MStereotype>(*stereotype);

//        if ((s->name == name) && (etype.empty() || (s->elementtype.empty()) || helper::tolower(etype) == helper::tolower(s->elementtype))) {
        if (helper::tolower(s->name) == helper::tolower(name)) {
            retval = s;
            break;
        } else {
//            std::cerr << "No match of stereotype for : " << name << std::endl;
        }
    }
    return (retval);
}

void MModel::Add(std::shared_ptr<MPackage> p)
{
    Packages.emplace_back(p);
}

void MModel::Add(std::shared_ptr<MAssociation> a)
{
    Associations.emplace_back(a->id);
}

void MModel::Add(std::shared_ptr<MAssociationEnd> e)
{
    AssociationEnds.emplace_back(e->id);
}

void MModel::AddTag(const std::string &name, const std::string &value)
{
    TaggedValues.insert(std::pair<std::string, std::string>(name, value));
}

void MModel::Complete(void) {
    //
    //  Go through the associations and fill the ends with the pointers to the classifiers.
    //  Associations shall be put into both ends classifiers.
    for (auto & ae : AssociationEnds) {
        switch (ae->type) {
            //
            //  Explicit dont look on objects.
        case eElementType::Object:
            break;
        default :

            if ((*ae)->parent->type == eElementType::Association) {
                auto c = std::dynamic_pointer_cast<MClass>(*std::dynamic_pointer_cast<MAssociationEnd>(*ae)->Classifier);

                c->Add(std::dynamic_pointer_cast<MAssociation>(*(*ae)->parent));
            }
            break;
        }
    }
    //
    //  Prepare the unowned elements.
    for (auto & u : mUnowned) {
        u->Prepare();
    }
}


void MModel::Add(std::shared_ptr<MElement> e) {
    if ((!e->parent) && (!e->IsPackageBased())) {
        mUnowned.emplace_back(e);
    }
}
