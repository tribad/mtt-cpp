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
#include "main.h"
#include "melement.h"
#include "mclass.h"
#include "massociationend.h"
#include "massociation.h"
#include "mparameter.h"
#include "moperation.h"
#include "cclass.h"

std::map<std::string, std::shared_ptr<MClass>> MClass::Instances;
std::map<std::string, std::shared_ptr<MClass>> MClass::mByFQN;
std::map<std::string, std::shared_ptr<MClass>> MClass::mByReverseFQN;
std::map<std::string, std::shared_ptr<MClass>> MClass::mByModelPath;

MClass::MClass(const std::string&aId, std::shared_ptr<MElement> aParent) : MElement(aId, aParent) {
    type=eElementType::Class;
    MClass::Instances.insert(std::pair<std::string, std::shared_ptr<MClass>>(id, sharedthis<MClass>()));
}

std::shared_ptr<MClass> MClass::construct(const std::string&aId, std::shared_ptr<MStereotype> aStereotype, std::shared_ptr<MElement> aParent)
{
    return CClassBase::construct(aId, aStereotype, aParent);
}

std::shared_ptr<MClass> MClass::construct(const std::string&aId, std::string aPackageType,  std::shared_ptr<MElement> aParent)
{
    return CClassBase::construct (aId, aPackageType, aParent);
}

void MClass::AddAssoc(std::shared_ptr<MAssociation> a)
{
    if ((!a->ThisEnd(id)) || (!a->OtherEnd(id))) {
        return;
    }
    //
    //  If the association already is set we return immediatly.
    for (auto & ai : Association) {
        if (a == *ai) {
            return;
        }
    }
    Association.emplace_back(MElementRef(a));
    //
    //  We still rely on this mechanism to find the other/own end.
    //  But if the ends point to the same class we check the navigable flag.
    auto e0 = std::dynamic_pointer_cast<MAssociationEnd>(*a->ends[0]);
    auto e1 = std::dynamic_pointer_cast<MAssociationEnd>(*a->ends[1]);

    if (e0->Classifier == e1->Classifier) {
        //
        //  Sanity check.
        if (e0->Classifier == id) {
            if (e0->isNavigable()) {
                OtherEnd.push_back(a->ends[0]);
            } else {
                OwnEnd.push_back(a->ends[0]);
            }
            if (e1->isNavigable()) {
                OtherEnd.push_back(a->ends[1]);
            } else {
                OwnEnd.push_back(a->ends[1]);
            }
        }
    } else {
        OwnEnd.push_back(MElementRef(a->ThisEnd(id)->id));
        OtherEnd.push_back(MElementRef(a->OtherEnd(id)->id));
    }
}


void MClass::add() {
    std::string fqn = mTypeTree.getFQN();

    mByFQN.insert(std::pair<std::string, std::shared_ptr<MClass>>(fqn, sharedthis<MClass>()));

    std::string rfqn;
    for (auto c = fqn.rbegin(); c != fqn.rend(); c++) {
        rfqn.push_back(*c);
    }
    mByReverseFQN.insert(std::pair<std::string, std::shared_ptr<MClass>>(rfqn, sharedthis<MClass>()));

    std::string modelpath = getModelPath();
    MClass::mByModelPath.insert( std::pair<std::string, std::shared_ptr<MClass>>(modelpath, sharedthis<MClass>()));
}

std::shared_ptr<MClass> MClass::findBestMatch(const std::string& aName, const std::string& aClassNameSpace) {
    std::shared_ptr<MClass> retval;
    auto matchList = findBestMatch(aName);

    if (matchList.size() > 1) {
        auto ns = NameSpace(aClassNameSpace).combine(aName);
        //std::cerr << aName << " : +++++++" << std::endl;
        for (auto & l : matchList) {
//            std::cerr << l->getTypeName() << std::endl;
            if (l->getTypeName() == ns.getString()) {
                retval = l;
            }
        }
        //std::cerr << "+++++++" << std::endl;
        if (!retval) {
            retval = *matchList.begin();
        }
    } else if (!matchList.empty()) {
        retval = *matchList.begin();
    }
    return retval;
}
//
//  aName is a fragment. So it cannot contain more scope-parts than the stored ones.
std::list<std::shared_ptr<MClass>> MClass::findBestMatch(const std::string& aName) {
    std::list<std::shared_ptr<MClass>> retval;
    std::string rname;
    //
    //  reverse the search string.
    for (auto c = aName.rbegin(); c != aName.rend(); c++) {
        rname.push_back(*c);
    }
    //
    //  best is the first element that is greater or equal to rname.
    //  This need not be the correct match for the name.
    auto best = mByReverseFQN.lower_bound(rname);
    //
    //  I do not want to think about it.
    //  Check if anything could be found.
    if (best != mByReverseFQN.end()) {
        //
        //  We use the namespace class to check if the scope fragments match.
        //  All scope fragments from partial must match.
        NameSpace partial(rname);
        NameSpace found(best->first);
        //
        //  Check that partial is not larger than the found one.
        //  This is always a mismatch.
        if (partial.size() <= found.size()) {
            size_t i;
            for (i = 0; i<partial.size(); ++i) {
                if (partial[i] != found[i]) {
                    break;
                }
            }
            //
            //  Is a complete match. If prior loop runs until its end.
            if (i == partial.size()) {
                retval.push_back(best->second);
            }
        }
        //
        //  Search for ambigous matches.
        if (!retval.empty()) {
            //
            //  Move to the next entry before entering the loop
            ++best;
            while (best != mByReverseFQN.end()) {
                //
                //  setup found from next in map.
                found = best->first;
                //
                //  Check that partial is not larger than the found one.
                //  This is always a mismatch.
                if (partial.size() <= found.size()) {
                    size_t i;
                    for (i = 0; i<partial.size(); ++i) {
                        if (partial[i] != found[i]) {
                            break;
                        }
                    }
                    //
                    //  Is a complete match. If prior loop runs until its end.
                    if (i == partial.size()) {
                        bool haveit = false;
                        for (auto &r : retval) {
                            if (best->second == r) {
                                haveit = true;
                                break;
                            }
                        }
                        if (!haveit) {
                            retval.push_back(best->second);
                            if (gVerboseOutput) {
                                std::cerr << "Found ambigous match " << best->second->getTypeName() << " for " << aName << std::endl;
                            }
                        }
                    } else {
                        //
                        // No match at all. So we break the loop here.
                        if (i == 0) {
                            break;
                        } else {
                            //
                            //  As this is a partial match we add it to our result.
                            retval.push_back(best->second);
                        }
                    }
                }
                ++best;
            }
        } else {
//            std::cerr << "Type " << aName << " has no match" << std::endl;
        }
    } else {
        //std::cerr << "Type " << aName << " has no match" << std::endl;
    }

    return retval;
}
//
//  This is needed in the parser to get the package type if a class is enclosed from another class.
eElementType MClass::getPackageType() {
    eElementType retval = eElementType::ExternPackage;

    return retval;
}

std::string MClass::GetDefaultStereotype() {
    return std::string();
}

std::string MClass::getEnclosingScope() {
    std::string retval;

    auto p = parent;

    while ((p) && (p->IsClassBased())) {
        if (visibility != eVisibility::vPackage) {
            auto c = std::dynamic_pointer_cast<MClass>(*p);
            std::string scope;
            if (c->mTypeTree.mName.empty()) {
                TypeNode tmp = TypeNode::parse(c->name);
                scope = tmp.mName;
            } else {
                scope = c->mTypeTree.mName;
            }
            if (retval.empty()) {
                retval = scope;
            } else {
                retval = scope + "::" + retval;
            }
        }
        p = p->parent;
    }
    return retval;
}

