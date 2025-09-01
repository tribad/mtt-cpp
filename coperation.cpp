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
#include <iostream>
#include <sstream>
#include "helper.h"
#include "main.h"

#include "namespace.h"

#include "melement.h"

#include "mattribute.h"
#include "cattribute.h"

#include "mparameter.h"
#include "cparameter.h"
#include "moperation.h"
#include "coperation.h"
#include "cclass.h"
#include "cclassbase.h"
#include "ccxxclass.h"

#include "medge.h"
#include "mnode.h"
#include "mparameter.h"
#include "mpin.h"
#include "mactionnode.h"
#include "mactivity.h"

#include "mmessage.h"
#include "cmessage.h"

#include "minteraction.h"
#include "cinteraction.h"

#include "mlifeline.h"
#include "clifeline.h"

#include "mconnector.h"
#include "cconnector.h"

#include "massociationend.h"
#include "cassociationend.h"

#include "ccollaboration.h"
#include "crequirement.h"

std::string COperation::FQN() const {
    return name;
}

void COperation::Prepare(void) {

    if (HasStereotype("Slot")) {
        qtSlot = true;
    }
    if (HasStereotype("Signal")) {
        qtSignal = true;
    }
    if (HasStereotype("inline")) {
        isInline = true;
    }
    //
    //  Need to sort operations and attributes.
    std::multimap<int64_t, MElementRef> sorting;

    for (auto & p : Parameter) {
        sorting.insert(std::pair<int64_t, MElementRef> (p->mPosition, p));
    }
    Parameter.clear();
    for (auto & p : sorting) {
        Parameter.push_back(p.second);
    }

    for (auto & p : Parameter) {
        p->Prepare();
    }

    if (Activity) {
        Activity->Prepare();
    }

    //
    //  Prepare attached collaborations.
    for (auto& col : mCollaboration) {
        auto colab = std::dynamic_pointer_cast<CCollaboration>(*col);

        if (col->IsClassBased()) {
            col->Prepare();
        }
        for (auto & in : colab->mInteraction) {
            if (in) {
                for (auto & msg : std::dynamic_pointer_cast<CInteraction>(*in)->messages) {
                    auto message = std::dynamic_pointer_cast<CMessage>(*msg);
                    //CLifeLine* source      = ((CLifeLine*)(msg->source));
                    //CLifeLine* target      = ((CLifeLine*)(msg->target));
                    std::string signature;

                    if (!signature.empty()) {
                        auto op = std::dynamic_pointer_cast<COperation>(*message->signature);
                        op->qtSlot = true;
                        //
                        // find the signal on the source to mark it as signal.
                        auto con = std::dynamic_pointer_cast<CConnector>(*message->connector);

                        if (op->parent) {
                            MElementRef one = con->ends[0];
                            MElementRef two = con->ends[1];
                            MElementRef signalref;

                            if (one && (one->type == eElementType::Attribute) &&
                                    (one == op->parent->id)) {
                                if (two  && (two->type == eElementType::Attribute)) {
                                    signalref = one;
                                }

                            } else if (two && (two->type == eElementType::Attribute) &&
                                    (two == op->parent->id)) {
                                if (one && (one->type == eElementType::Attribute)) {
                                    signalref = one;
                                }
                            }
                            if (signalref && (signalref->type == eElementType::CxxClass)) {
                                auto sigop =std::dynamic_pointer_cast<CCxxClass>(*signalref)->findBySignature(signature);

                                if (sigop) {
                                    sigop->qtSignal = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void COperation::Dump(std::shared_ptr<MModel> model) {

}

std::string COperation::GetReturnType(const NameSpace& aNameSpace) {
    std::string retval="void";

    for (auto & i : Parameter) {
        bool enforce_const = false;
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction == "return") {
            //
            // we have a return type so we clear the "void" initialization
            retval = "";
            //
            //  Check if need to enforce a const return type.
            if (para->isReadOnly || hasConstReturn) {
                enforce_const = true;
            }

            //
            //  Shorten on namespace.
            std::string classifierName;

            if (para->Classifier) {
                if (para->Classifier->IsClassBased()) {
                    auto cb = std::dynamic_pointer_cast<CClassBase>(*para->Classifier);

                    classifierName = cb->mTypeTree.getFQN(!gConstInReturnTypeRight, enforce_const);
                } else {
                    classifierName = para->Classifier->FQN();
                }
            } else  {
                if (!para->ClassifierName.empty()) {
                    TypeNode temp(TypeNode::parse(para->ClassifierName));

                    temp.fill(aNameSpace.getString());
                    classifierName = temp.getFQN(!gConstInReturnTypeRight, enforce_const);
                }
            }
            if (classifierName == "__simobject__") {
                classifierName = "tObjectRef";
            }
            std::ostringstream tname;

            if ((para->Multiplicity == "1..*") || (para->Multiplicity == "*") || (para->Multiplicity == "0..*")) {
                if (!para->mQualifierType.empty()) {
                    tname << "std::map<" << para->mQualifierType << ", " << aNameSpace.diff(classifierName) << ">";
                } else {
                    tname << "std::vector<" << aNameSpace.diff(classifierName) << ">";
                }
            } else {
                tname << aNameSpace.diff(classifierName);
            }
            retval += tname.str();
            break;
        }
    }
    if (IsCastOperator()) {
        retval = "";
    }
    return retval;
}


std::string COperation::GetParameterDecl(const NameSpace& aNameSpace) {
    std::string retval;
    bool        dump = false;

    for (auto & i : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction != "return") {
            if (dump) {
                retval+=", ";
            } else {
                retval.clear();
            }

            std::string classifierName;

            if (para->Classifier) {
                classifierName = std::dynamic_pointer_cast<CClassBase>(*para->Classifier)->mTypeTree.getFQN(!gConstInArgumentRight, para->isReadOnly);
            } else {
                TypeNode tmptree(TypeNode::parse(para->ClassifierName));

                tmptree.fill(aNameSpace.getString());
                classifierName = tmptree.getFQN(!gConstInArgumentRight, para->isReadOnly);
            }

            retval += aNameSpace.diff(classifierName);
            retval+=" ";

            retval+=i->name;
            if (!para->Multiplicity.empty()) {
                retval+="[";
                if (para->Multiplicity != "*") {
                    retval += para->Multiplicity;
                }
                retval+="]";
            }
            if (!para->defaultValue.empty()) {
                retval += " = ";
                retval += para->defaultValue;
            }
            dump = true;
        }
    }
    return retval;
}

std::string COperation::GetParameterSignature(const NameSpace& aNameSpace) {
    std::string retval;
    bool        dump = false;

    for (auto & i : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction != "return") {
            if (dump) {
                retval+=", ";
            } else {
                retval.clear();
            }
            //
            //  Shorten classifier name on the namespace.
            std::string classifierName;

            if (para->Classifier) {
                classifierName = std::dynamic_pointer_cast<CClassBase>(*para->Classifier)->mTypeTree.getFQN(!gConstInArgumentRight, para->isReadOnly);
            } else {
                TypeNode tmptree(TypeNode::parse(para->ClassifierName));

                tmptree.fill(aNameSpace.getString());
                classifierName = tmptree.getFQN(!gConstInArgumentRight, para->isReadOnly);
            }

            retval += aNameSpace.diff(classifierName);

            if (!para->Multiplicity.empty()) {
                retval+="[";
                if (para->Multiplicity != "*") {
                    retval += para->Multiplicity;
                }
                retval+="]";
            }
            dump = true;
        }
    }
    return retval;
}

std::string COperation::GetParameterDefinition(const NameSpace& aNameSpace) {
    std::string retval;
    bool        dump = false;

    for (auto & i : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction != "return") {
            if (dump) {
                retval+=", ";
            } else {
                retval.clear();
            }

            std::string classifierName;

            if (para->Classifier) {
                classifierName = std::dynamic_pointer_cast<CClassBase>(*para->Classifier)->mTypeTree.getFQN(!gConstInArgumentRight, para->isReadOnly);
            } else {
                TypeNode tmptree(TypeNode::parse(para->ClassifierName));

                tmptree.fill(aNameSpace.getString());
                classifierName = tmptree.getFQN(!gConstInArgumentRight, para->isReadOnly);
            }

            retval += aNameSpace.diff(classifierName);
            retval += " ";

            retval += i->name;

            if ((!para->Multiplicity.empty()) && (helper::trim(para->Multiplicity) != "1")) {
                retval+="[";
                if (para->Multiplicity != "*") {
                    retval += para->Multiplicity;
                }
                retval+="]";
            }
            dump = true;
        }
    }
    return retval;
}

std::string COperation::GetPHPParameterDefinition() {
    std::string retval;
    bool        dump = false;

    for (auto & i : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*i);

        if (para->Direction != "return") {
            if (dump) {
                retval += ", ";
            } else {
                retval.clear();
            }
            if (!para->ClassifierName.empty()) {
                retval += para->ClassifierName;
                retval += " ";
            }
            if ((i->name)[0]!='$') {
                retval += "$";
            }
            retval += i->name;
            if (!para->Multiplicity.empty()) {
                retval += "[";
                if (para->Multiplicity != "*") {
                    retval += para->Multiplicity;
                }
                retval += "]";
            }
            dump = true;
        }
    }
    return retval;
}

bool COperation::IsCastOperator() {
    bool        retval = false;
    size_t      pos    = name.find_first_of(' ');
    std::string op     = name.substr(0, pos);

    if (op == "operator") {
        std::string suffix = name.substr(name.find_first_not_of(' ', pos));
        //
        //  strip blanks.
        while (suffix.at(suffix.size()-1) == ' ') {
            suffix.erase(suffix.size()-1);
        }
        if (!isCardinalType(suffix)) {
            auto matched = MClass::findBestMatch(suffix);

            if (!matched.empty()) {
                retval = true;
            }
        } else {
            retval = true;
        }
    }
    return retval;
}

std::string COperation::GetCTORInitList() {
    std::string retval; 
    std::string ctorinittag;
    std::string ctorinitspec;
    //
    //  Sanity Check.
    if (parent && (parent->IsClassBased())) {
        auto cc = std::dynamic_pointer_cast<CClassBase>(*parent);

        std::list <std::pair<std::string, std::string> > il;
        //
        //  Collect possible member initialization from attributes.
        for (auto & i : cc->Attribute) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*i);
            //
            //  Only if the name is set.
            if (!a->name.empty()) {
                if (!(a->ClassifierName.empty())) {
                    //
                    //  Where to do the initialization with member default values.
                    if (gInitMemberDefaultInInitializerList) {
                        //
                        //  
                        il.emplace_back(a->name, a->defaultValue);
                    } else {
                        il.emplace_back(a->name, "");
                    }
                }
            }
        }
        //
        // Collect possible member initialization from association ends.
        if (cc->type == eElementType::CxxClass) {
            auto cxx = std::dynamic_pointer_cast<CCxxClass>(cc);
            //
            //  Processing the aggregations/compositions that are navigable.
            for (auto & i : cxx->OtherEnd) {
                auto ae = std::dynamic_pointer_cast<CAssociationEnd>(*i);

                if (ae->isNavigable()) {
                    //
                    // only if the name is set for the end.
                    //
                    //  Where to do the initialization with member default values.
                    if (gInitMemberDefaultInInitializerList) {
                        //
                        //  
                        il.emplace_back(ae->name, ae->defaultValue);
                    } else {
                        il.emplace_back(ae->name, "");
                    }
                }
            }
        }
        //
        //  Now check for parameters matching.
        for (auto & pe : Parameter) {
            std::string pbase;   // parameter basename
            std::string mbase;   // member basename
            //
            //  check the parameter name size. It must be the prefix size plus 1 at least. as we have a prefix and 
            //  a least a single character for the Name.
            if (pe->name.size() > gStaticArgumentPrefix.size()) {
                size_t ppos =  pe->name.find(gStaticArgumentPrefix);

                if (ppos == 0) {
                    pbase = pe->name.substr(ppos + gStaticArgumentPrefix.size());
                }
            }
            //
            //  Process the parameter.
            if (!pbase.empty()) {
                //
                //  Go along the initializer list we have.
                for (auto & ie : il) {
                    //
                    // Make sure the member name is longer than the prefix.
                    if (ie.first.size() > gStaticMemberPrefix.size()) {
                        //
                        //  create the member base name for comparison.
                        size_t ppos =  ie.first.find(gStaticMemberPrefix);

                        if (ppos == 0) {
                            mbase = ie.first.substr(ppos + gStaticMemberPrefix.size());
                        }
                        //
                        //  Check the basenames against parameter basename.
                        //  This may overwrite the default value set before.
                        if (pbase == mbase) {
                            ie.second = pe->name;
                        }
                    }
                }
            }
        }
        //
        //  After checking all parameters against the members
        //  We check for handwritten initializations in the operation specification.
        ctorinitspec = getCtorInitFromSpec();
        //
        // Sanity check. Only if we have any handwritten initialization code we need to process it.
        if (!ctorinitspec.empty()) {
            // 
            //  first is the member name. Second the initializer value.
            auto sl = parseCtorInitFromSpec(ctorinitspec);
            //
            //  Go through the members and assign the value.
            //  This works for members easily.
            for (auto & ie : il) {
                if (sl.find(ie.first) != sl.end()) {
                    if (!sl[ie.first].second.empty()) {
                        ie.second = sl[ie.first].second;
                        //
                        //  remove the found member initializers from the init spec list.
                        sl.erase(ie.first);
                    }
                }
            }
            //
            //  process the base classes. Members shall be removed already.
            //  Now we walk through the base classes to find the direct bases that may be
            //  call initializers on.
            for (auto & bc : cc->Base) {
                auto bcc = std::dynamic_pointer_cast<CClassBase>(bc.getElement());
                std::string bcfqnstr = bcc->mTypeTree.getNameSpace();
                
                if (!bcfqnstr.empty()) {
                    bcfqnstr += "::";
                }
                bcfqnstr += bcc->mTypeTree.mName;

                NameSpace bcfqn = bcfqnstr;
                //
                //  As we may have to deal with namespaces and the like we cannot use 
                //  the find method on the map. We do a linear search.
                //  This has no impact on speed as the initializer list has been reduced already before.
                for (auto & sle : sl) {
                    //
                    //  Check if name matches.
                    NameSpace ns = sle.first;
                    //
                    //  Check if the base name matches between the base class and the initializer list entry.
                    if (bcfqn[bcfqn.size()-1] == ns[ns.size() -1]) {
                        //
                        //  Now build the difference from the namespaces of
                        //  the operation parent-class and the baseclass.
                        ns = cc->GetNameSpace();
                        auto m = ns.diff(bcc->mTypeTree.getNameSpace());
                        //
                        //  Create template parameter from binding.
                        std::string templateparameter;

                        //
                        // construct the base-class name to use in initializer.
                        if (m.empty()) {
                            il.emplace_front(bcc->mTypeTree.mName + sle.second.first, sle.second.second);
                        } else {
                            il.emplace_front(m + "::" + bcc->mTypeTree.mName + sle.second.first, sle.second.second);
                        }
                        sl.erase(sle.first);
                        break;
                    }
                }
            }
            if (!sl.empty()) {
                std::cerr << "There something wrong with the initializer list in " << cc->mTypeTree.getFQN() << "::" << name << "(" << GetParameterDefinition(cc->GetNameSpace()) << "); Leftovers:";
                for (auto & sle : sl) {
                    std::cerr << " " << sle.first ;
                }
                std::cerr << std::endl;
            }
        }
        //
        //  Start the output
        std::ostringstream oss;
        bool colon = false;
        //
        //  At this point we have the member initializers filled.
        for (auto & ie : il) {
            if (!ie.second.empty()) {
                if (!colon) {
                    oss << "\n: ";
                    colon = true;
                } else {
                    oss << ", ";
                }
                oss << ie.first << "(" << ie.second << ")\n";
            }
        }
        retval = oss.str();
    }
    return retval;
}

std::string COperation::getCtorInitFromSpec() {
    std::string retval; 

    if (!Specification.empty()) {
        int havecolon = -1;   //  colon position

        for (size_t i = 0; i < Specification.size(); i++) {
            if (!std::isspace(Specification[i])) {
                if (havecolon == -1) {
                    if (Specification[i] == ':') {
                        havecolon = i;
                    }
                } else {

                    if (Specification[i] == '{') {
                        break;
                    } else {
                        retval.push_back(Specification[i]);
                    }
                }
            }
        }
    }
    return retval;
}

std::map <std::string, std::pair<std::string, std::string>>  COperation::parseCtorInitFromSpec(const std::string& aCtorInit) {
    std::map <std::string, std::pair<std::string, std::string>> retval;
    int state = 0;
    std::string mname;
    std::string initializer;
    std::string templateparameter;
    int bracecount = 0;
    int templateopening = 0;

    for (auto c : aCtorInit) {
        switch (state) {
        case 0: //  starting point ignore whitespace but take the first character as the member name.
            if ((std::isalnum(c)) || (c == '_') || (c == ':')) { 
                mname.push_back(c);
            } else if (c == '(') {  // opening parameter to member
                bracecount++;
                state = 1;
            } else if (c == '<') {  // ignore template parameter
                templateopening++;
                templateparameter.push_back(c);
                state = 2;
            } else {  // ignore any other character.
            }
            break;
        case 1:  //  Now we are in the parameter to member
            if (c == ')') {
                bracecount--;
                if (bracecount != 0) {
                    initializer.push_back(c);
                } else { //  completed the parameter to member.
                    retval.insert(std::pair<std::string, std::pair<std::string,std::string>>(mname, {templateparameter,initializer}));
                    mname.clear();
                    initializer.clear();
                    state = 0;
                }
            } else {
                if (c == '(') {
                    bracecount++;
                }
                initializer.push_back(c);
            }
            break;
        case 2: // ignore all template parameter content
            if (c == '<') {
                templateopening++;
            } else if (c == '>') {
                templateopening--;
                if (!templateopening) {
                    state = 0;
                }
            }
            templateparameter.push_back(c);
            break;
        default:
            break;
        }
    }

    return retval;
}

std::string COperation::getHeader(int indent) {
    std::ostringstream oss;
    std::string filler;
    filler.assign(indent+IndentSize, ' ');

    oss << filler << gDoxygenCommentStart << std::endl;
    if (comment.empty()) {
        oss << filler << gDoxygenCommentStart << " @brief TODO\n";
    } else {
        auto com = GetComment();
        //
        // Dump the brief line.
        std::string brief = *com.begin();

        oss << filler << gDoxygenCommentStart << " @brief";
        if (!brief.empty()) {
            oss << " " << brief << std::endl;
        } else {
            oss << std::endl;
        }
        if (com.size() > 1) {
            oss << filler << gDoxygenCommentStart << std::endl;

            for (auto i = ++com.begin() ; i != com.end(); ++i) {
                oss << filler << gDoxygenCommentStart << ' ' << *i << std::endl;
            }
        }
    }
    oss << filler << gDoxygenCommentStart << std::endl;
    for (auto & p : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*p);

        if (para->Direction != "return") {
            oss << filler << gDoxygenCommentStart << " @param[" << para->Direction << "] " << p->name;
            if (!p->comment.empty()) {
                oss << " " << p->comment << std::endl;
            } else {
                oss << std::endl;
            }
        }
    }
    /// @param[in] aName The property value to be set to the builder
    for (auto & p : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*p);

        if ((para->Direction == "return") && (para->ClassifierName != "void") && (!para->ClassifierName.empty())) {
            oss << filler << gDoxygenCommentStart << " @return";
            if (!p->comment.empty()) {
                oss << " " << p->comment << std::endl;
            } else {
                oss << std::endl;
            }
        }
    }
    //
    //  Check for requirements.
    bool req=false;

    for (auto & s : Supplier) {

        if ((s.getConnector()->type == eElementType::Dependency) && (s.getElement()->type == eElementType::Requirement)) {
            auto requirement = std::dynamic_pointer_cast<CRequirement>(s.getElement());

            if (!req) {
                oss << filler << gDoxygenCommentStart << std::endl;
                oss << filler << gDoxygenCommentStart << " @coversreqs\n";
                req = true;
            }
            //
            //  requirement id
             oss << filler << gDoxygenCommentStart << " @req{" << helper::toDash(helper::tolower(requirement->name)) << "}" << std::endl;
        }
    }
    if (req) {
        oss << filler << gDoxygenCommentStart << " @endcoversreqs\n";
    }

    return oss.str();
}

std::string COperation::getSourceHeader(int indent) {
    std::ostringstream oss;
    std::string filler;
    filler.assign(indent, ' ');

    oss << filler << "///\n";
    if (comment.empty()) {
        oss << filler << gDoxygenCommentStart << " @brief TODO\n";
    }
    else {
        auto com = GetComment();
        //
        // Dump the brief line.
        std::string brief = *com.begin();

        oss << filler << gDoxygenCommentStart << " @brief";
        if (!brief.empty()) {
            oss << " " << brief << std::endl;
        }
        else {
            oss << std::endl;
        }
        if (com.size() > 1) {
            oss << filler << gDoxygenCommentStart << std::endl;

            for (auto i = ++com.begin(); i != com.end(); ++i) {
                oss << filler << gDoxygenCommentStart << ' ' << *i << std::endl;
            }
        }
    }
    oss << filler << gDoxygenCommentStart << std::endl;
    for (auto& p : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*p);

        if (para->Direction != "return") {
            oss << filler << gDoxygenCommentStart << " @param[" << para->Direction << "] " << p->name;
            if (!p->comment.empty()) {
                oss << " " << p->comment << std::endl;
            }
            else {
                oss << std::endl;
            }
        }
    }
    /// @param[in] aName The property value to be set to the builder
    for (auto& p : Parameter) {
        auto para = std::dynamic_pointer_cast<CParameter>(*p);

        if ((para->Direction == "return") && (para->ClassifierName != "void") && (!para->ClassifierName.empty())) {
            oss << filler << gDoxygenCommentStart << " @return";
            if (!p->comment.empty()) {
                oss << " " << p->comment << std::endl;
            }
            else {
                oss << std::endl;
            }
        }
    }
    //
    //  Check for requirements.
    bool req = false;

    for (auto& s : Supplier) {

        if ((s.getConnector()->type == eElementType::Dependency) && (s.getElement()->type == eElementType::Requirement)) {
            auto requirement = std::dynamic_pointer_cast<CRequirement>(s.getElement());

            if (!req) {
                oss << filler << gDoxygenCommentStart << std::endl;
                req = true;
            }
            //
            //  requirement id
            oss << filler << gDoxygenCommentStart << " [" << helper::toDash(helper::tolower(requirement->name)) << "]" << std::endl;

            auto lines = requirement->GetComment();

            for (auto& l : lines) {
                oss << filler << gDoxygenCommentStart << " " << l << std::endl;
            }
            oss << filler << gDoxygenCommentStart << std::endl;
        }
    }

    return oss.str();
}
void COperation::DumpTemplateOperationPrefix(std::ostream& file, bool strip_default, int a_indent) {
    std::string filler;
    filler.assign(a_indent, ' ');
    std::string para = GetTaggedValue("parameter");

    if (strip_default) {
        size_t startpos;

        while ((startpos = para.find_first_of('=')) != std::string::npos) {
            while (startpos < para.size()) {
                char c = para[startpos];

                if ((c != ',') && (c != '>') && (c != ';')) {
                    para.erase(startpos, 1);
                    para = helper::trim(para);
                } else {
                    break;
                }
            }
        }
    }
    file << filler << "template<" << para << ">\n";
}
