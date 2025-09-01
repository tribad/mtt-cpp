//
// Copyright 2016 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include "mattribute.h"
#include "cattribute.h"
#include "massociationend.h"
#include "cassociationend.h"
#include "cphpclass.h"
#include "mdependency.h"
#include "cdependency.h"
#include "mgeneralization.h"
#include "cgeneralization.h"

std::string CPHPClass::FQN() const {
    std::string fqn = parent->FQN();

    return fqn+"\\"+name;
}


void CPHPClass::SetFromTags(const std::string& name, const std::string&value)
{
}

void CPHPClass::Prepare(void) {
    PrepareBase();

    donelist.clear();

    CollectNeededModelHeader(sharedthis<MElement>());

}

void CPHPClass::CollectNeededModelHeader(std::shared_ptr<MElement> e) {
    auto c = std::dynamic_pointer_cast<MClass>(e);

    if (donelist.find(e->name)==donelist.end()) {
        auto cbe = std::dynamic_pointer_cast<CClassBase>(e);

        donelist.insert(e->name);
        if (this == e.get()) {
            for (auto & bi : cbe->Base) {
                CollectNeededModelHeader(bi.getElement());
            }

            for (auto & i : c->Attribute) {
                auto a= std::dynamic_pointer_cast<CAttribute>(*i);

                if (((a->Aggregation == aNone) || (a->Aggregation == aComposition)) && a->Classifier) {
                    eElementType et=a->Classifier->type;

                    if ((et != eElementType::SimObject)) {
                        CollectNeededModelHeader(a->Classifier);
                    }
                }
            }
            for (auto & i : c->OtherEnd) {
                auto a = std::dynamic_pointer_cast<CAssociationEnd>(*i);

                if ((a->Aggregation != aShared) && a->Classifier && (a->isNavigable())) {
                    eElementType et=a->Classifier->type;

                    if ((et != eElementType::SimObject)) {
                        CollectNeededModelHeader(a->Classifier);
                    }
                }
            }
        }
        if (e.get() != this) {
            neededmodelheader.add(e);
        } else {
            auto ti=e->tags.find("ExtraInclude");

            if (ti != e->tags.end()) {
                optionalmodelheader.add(e);
            }
            for (auto & i : Supplier) {
                if (i.getElement()->type==eElementType::PHPClass)  {
                    CollectNeededModelHeader(i.getElement());
                }
            }
        }
    }
}



void CPHPClass::DumpAttributeDecl(std::ostream& hdr) {
    IndentIn();

    for (auto & ma: Attribute) {
        if (ma->visibility != vPackage) {
            hdr << indent;
            if (ma->visibility == vPublic) {
                hdr << "public ";
            } else if (ma->visibility == vProtected) {
                hdr << "protected ";
            } else if (ma->visibility == vPrivate) {
                hdr << "private ";
            } else {
            }

            auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

            if (a->isStatic) {
                hdr << "static ";
            }
            hdr << "$" << a->name;
            if ((a->Multiplicity == "1") || a->Multiplicity.empty()) {
                if (!a->defaultValue.empty()) {
                    hdr << " = " << a->defaultValue << ";\n";
                } else {
                    hdr << ";\n";
                }
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    hdr << "= array();\n";
                } else {
                    hdr << "= array (" << a->Multiplicity << ");\n";
                }
            }
        }
    }

    for (auto & me : OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*me);

        if (me->visibility != vPackage) {
            if (!a->name.empty()) {
                hdr << indent;
                if (me->visibility == vPublic) {
                    hdr << "public ";
                } else if (me->visibility == vProtected) {
                    hdr << "protected ";
                } else if (me->visibility == vPrivate) {
                    hdr << "private ";
                } else {
                }

                hdr << "$" <<  a->name;
                if ((a->Multiplicity == "1") || a->Multiplicity.empty()) {
                    hdr << ";\n";
                } else {
                    if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {

                    } else {
                        hdr << "[" << a->Multiplicity << "];\n";
                    }

                }
            }
        }
    }
    IndentOut();
}

void CPHPClass::DumpPackageAttributes(std::ostream& outfile) {
    for (auto & ma : Attribute) {
        if (ma->visibility == vPackage) {
            auto a = std::dynamic_pointer_cast<CAttribute>(*ma);

            if (a->isStatic) {
                outfile << "static ";
            }
            outfile << "$" << a->name;
            if ((a->Multiplicity == "1") || a->Multiplicity.empty()) {
                if (!a->defaultValue.empty()) {
                    outfile << " = " << a->defaultValue << ";\n";
                } else {
                    outfile << ";\n";
                }
            } else {
                if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {
                    outfile << "= array();\n";
                } else {
                    outfile << "= array (" << a->Multiplicity << ");\n";
                }
            }
        }
    }
    for (auto & me: OtherEnd) {
        auto a = std::dynamic_pointer_cast<CAssociationEnd>(*me);

        if (me->visibility == vPackage) {
            if (!a->name.empty()) {

                outfile << "$" <<  a->name;
                if ((a->Multiplicity == "1") || a->Multiplicity.empty()) {
                    outfile << ";\n";
                } else {
                    if ((a->Multiplicity == "1..*") || (a->Multiplicity == "*") || (a->Multiplicity == "0..*")) {

                    } else {
                        outfile << "[" << a->Multiplicity << "];\n";
                    }
                }
            }
        }
    }
}


void CPHPClass::DumpOperationBuddy(std::ostream& outfile, std::shared_ptr<COperation> op)
{
    std::string rettype;
    std::string pdef = op->GetPHPParameterDefinition();

    rettype=op->GetReturnType(NameSpace());
    if (rettype.at(rettype.size()-1)== '&') {
        rettype=rettype.substr(0, rettype.size()-1);
    }
    if (op->isStatic) {
        outfile << "static ";
    }
    outfile << "function " << op->name << "(" << pdef << ") {\n";

    outfile << "// User-Defined-Code:" << op->id << "\n";
    outfile << "// End-Of-UDC:" << op->id << "\n";
    if ((op->GetReturnType(NameSpace()) != "void") && (op->GetReturnType(NameSpace()) == rettype)) {
        outfile << indent << "    return  ("<< op->GetReturnName() << ");\n";
    }
    outfile << indent << "}\n\n";
}

void CPHPClass::DumpOperations(std::ostream& outfile) {
    IndentIn();
    for (auto & mo : Operation) {
        if (mo->visibility == vPrivate) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);

            op->DumpComment(outfile, indentation);
            outfile << indent << "private ";
            DumpOperationBuddy(outfile, op);
        }
    }

    for (auto & mo : Operation) {
        if (mo->visibility == vProtected) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);

            op->DumpComment(outfile, indentation);
            outfile << indent << "protected ";
            DumpOperationBuddy(outfile, op);
        }
    }
    for (auto & mo : Operation) {
        if (mo->visibility == vPublic) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);

            op->DumpComment(outfile, indentation);
            outfile << indent << "public ";
            DumpOperationBuddy(outfile, op);
        }
    }
    IndentOut();
}

void CPHPClass::DumpPackageOperations(std::ostream& outfile) {
    for (auto & mo : Operation) {
        if (mo->visibility == vPackage) {
            auto op = std::dynamic_pointer_cast<COperation>(*mo);

            op->DumpComment(outfile, indentation);
            DumpOperationBuddy(outfile, op);
        }
    }
}

void CPHPClass::DumpUses(std::ostream &outfile) {
    bool gap = true;

    for (auto & d : Dependency) {
        if (d->HasStereotype("access")) {
            auto dep = std::dynamic_pointer_cast<CDependency>(*d);

            if (dep->target && (dep->target->type == eElementType::PHPClass)) {
                auto s = std::dynamic_pointer_cast<CPHPClass>(*dep->target);
                //
                //  If we do dump a single use line we add a gap to the previous block
                if (gap) {
                    outfile << std::endl;
                    gap = false;
                }
                outfile << "use " << s->FQN() << ";" << std::endl;
            }
        }
    }
}

void CPHPClass::Dump(std::shared_ptr<MModel> model) {
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);
    std::set<std::string> includesdone;

    DumpBase(cmodel, name);
    src << "<?php\n";
    //
    //  Add strict_types if requested
    if (Strict) {
        src << "\ndeclare(strict_types = 1);\n";
    }
    //
    //  Check if we have a namespace defined.
    std::string ns = GetNameSpace();

    if (!ns.empty()) {
        src << "\nnamespace " << ns << ";" << std::endl;
    }
    DumpUses(src);
    src << "\n/*\n";
    src << " *  List of system includes\n";
    src << " */\n";
    for (auto & i : optionalmodelheader) {
        std::string header = i->tags.find("ExtraInclude")->second;
        std::string extra;
        size_t      start       = 0;
        size_t      end         = 0;

        do {
            end=header.find_first_of(' ', start);
            extra=header.substr(start, end-start);
            /*
             * trim front
             */
            while (extra[0]==' ') extra.erase(0, 1);
            /*
             * if proto has a size after trimming we can add it to the
             * list of protos.
             */
            if (!extra.empty()) {
                if (donelist.find(extra) == donelist.end()) {
                    src << "require_once \"" << extra << "\";\n";
                    donelist.insert(extra);
                }
            }
            /*
             * if we are not at end of search we skip a character.
             * At end we do nothing and let the loop condition stop
             * the loop.
             */
            if (end == std::string::npos) {
                start = end;
            } else {
                start = end + 1;
            }
        } while (end!=std::string::npos);
    }


    donelist.clear();

    if (ModelIncludes) {
        src << "/*\n";
        src << " *  List of model includes\n";
        src << " */\n";

        for (auto & i : neededmodelheader) {
            if (includesdone.find(i->name) == includesdone.end()) {
                std::string pathto = GetPathTo(i);
                if (!pathto.empty() && (pathto[pathto.size()-1]!='/')) {
                    pathto+="/";
                }
                if (!pathto.empty()) {
                    src << "require_once dirname(__FILE__).'" << pathto <<  i->name << ".php';\n";
                } else {
                    src << "require_once '" <<  i->name << ".php';\n";
                }
                includesdone.insert(i->name);
            }
        }
    }
    if (comment.empty()) {
        src << "/*\n";
        src << " *  class-definition\n";
        src << " */\n";
    } else {
        DumpComment(src);
    }

    src << "class " << name;
    for (auto & bi : Base) {
        src << " extends " << bi.getElement()->name;
    }

    src << " {\n";
    src << "    /*\n";
    src << "     *  attributes of the class\n";
    src << "     */\n";
    DumpAttributeDecl(src);
    src << "    /*\n";
    src << "     *  operations of class\n";
    src << "     */\n";
    DumpOperations(src);
    src << "}\n";
    DumpPackageAttributes(src);
    DumpPackageOperations(src);
    src << "\n";
    CloseStreams();
}
