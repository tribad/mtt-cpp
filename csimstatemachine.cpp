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
#include "mstate.h"
#include "cstate.h"
#include "mpseudostate.h"
#include "cpseudostate.h"
#include "mtransition.h"
#include "mstatemachine.h"
#include "msimstatemachine.h"
#include "csimstatemachine.h"
#include "helper.h"

CSimStatemachine::CSimStatemachine()
{
    history=0;
}


std::string CSimStatemachine::FQN() const {
    return name;
}

void CSimStatemachine::Prepare(void) {
    for (auto & i : transitions) {
        i->Prepare();
    }
    for (auto & s : states) {
        s->Prepare();
        if (s->type==eElementType::PseudoState) {
            auto p = std::dynamic_pointer_cast<CPseudoState>(*s);
            if (p->kind=="shallowHistory") {
                history=p;
            }
        }
    }
}

void CSimStatemachine::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}

std::list<std::string> CSimStatemachine::GetExternalTransitions() {
    std::list<std::string> retval;

    for (auto & i : transitions) {
        if ((std::dynamic_pointer_cast<MTransition>(*i)->kind=="external") && (!i->name.empty())) {
            retval.push_back((*i)->name);
        }
    }
    return (retval);
}

std::list<std::string> CSimStatemachine::GetInternalTransitions() {
    std::list<std::string> retval;

    for (auto & i : transitions) {
        if ((std::dynamic_pointer_cast<MTransition>(*i)->kind=="internal")  && (!i->name.empty())) {
            retval.push_back((*i)->name);
        }
    }
    return (retval);
}

bool CSimStatemachine::HasNormalStates() {
    for (auto & i : states) {
        if (i->type == eElementType::State) {
            return (true);
        }
    }
    return (false);
}


//
//  Find the initial state. It is a state without incoming transitions.
std::shared_ptr<MState> CSimStatemachine::GetInitialState() {
    std::shared_ptr<MState> retval;

    for (auto & st : states) {
        if (st->type == eElementType::PseudoState) {
            auto ps = std::dynamic_pointer_cast<MPseudoState>(*st);

            if (helper::tolower(ps->kind) == "initial") {
                retval = ps;
                break;
            }
        } else {
            if (std::dynamic_pointer_cast<MState>(*st)->Incoming.empty()) {
                retval = std::dynamic_pointer_cast<MState>(*st);
                break;
            }
        }
    }
    if (retval) {
        //
        //  Check if this is the initial pseudo state.
        if (retval->type == eElementType::PseudoState) {
            auto ps = std::dynamic_pointer_cast<MPseudoState>(retval);

            if ((helper::tolower(ps->kind) == "initial") && (ps->Outgoing.size() == 1) &&
                (std::dynamic_pointer_cast<MTransition>(*(ps->Outgoing[0]))->guard.empty())) {
                retval = std::dynamic_pointer_cast<MState>(*std::dynamic_pointer_cast<MTransition>(*ps->Outgoing[0])->to);
            }
        }
    }
    return retval;
}

void CSimStatemachine::DumpStateEnumerators(std::ostream &output, std::string prefix) {
    output << "/*\n"
              " *  State Enumerator for " << prefix << std::endl;
    output << " */\n"
              "struct e" << prefix << "State {\n";

    int stateCount = 0;

    for (auto & st : states) {
        if (st->type != eElementType::PseudoState) {
            if (st->type != eElementType::FinalState) {
                output << "    static const uint64_t " << helper::normalize(st->name) << " = " << stateCount++ << ';' << std::endl;
            } else {
                output << "    static const uint64_t " << helper::normalize(st->name) << " = UINT64_MAX;" << std::endl;
            }
        } else {
            if (helper::tolower(std::dynamic_pointer_cast<CPseudoState>(*st)->kind) == "initial") {
                output << "    static const uint64_t " << helper::normalize(st->name) << " = " << stateCount++ << ';' << std::endl;
            }
        }
    }
    output << "};\n\n";
    //
    //  Now dump enumerators for every composite state.
    for (auto & st : states) {
        if (st->type != eElementType::PseudoState) {
            if (!std::dynamic_pointer_cast<MState>(*st)->States.empty()) {
                std::dynamic_pointer_cast<CState>(*st)->DumpStateEnumerators(output, prefix);
            }
        }
    }
}

void CSimStatemachine::GetStateVars(std::list<std::pair<std::string, std::string> >& aAttrList,
                                    std::map<std::string, uint64_t>& aIdMap,
                                    std::string prefix) {    
    //
    //  Could be done in another place. But has been done here before.
    stateclass = "e";
    stateclass += prefix + "State";
    //
    //  Search through the states to find composite states.
    for (auto & st : states) {
        if (st->type != eElementType::PseudoState) {
            if (!std::dynamic_pointer_cast<CState>(*st)->States.empty()) {
                std::dynamic_pointer_cast<CState>(*st)->GetStateVars(aAttrList, aIdMap, prefix);
            }
        }
    }
}
