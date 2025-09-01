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
#include "helper.h"
#include "crc64.h"
#include "mstate.h"
#include "cstate.h"
#include "mtransition.h"
#include "mpseudostate.h"
#include "cpseudostate.h"

std::string CState::FQN() const {
    return name;
}

void CState::Prepare(void) {
    for (auto & i : Transitions) {
        i->Prepare();
    }
    for (auto & s : States) {
        s->Prepare();
        if (s->type==eElementType::PseudoState) {
            auto p = std::dynamic_pointer_cast<CPseudoState>(*s);
            if (p->kind=="shallowHistory") {
                shallowHistory = s;
            }
        }
    }
}

void CState::Dump(std::shared_ptr<MModel> model) {
    (void)model;
}


std::list<std::string> CState::GetHistoryAttributes(std::string prefix) {
    std::list<std::string>          smlist;

    prefix+=helper::normalize(helper::tolower(name)+"_");
    if (shallowHistory) {
        smlist.push_back(prefix+"state");
    }

    for (auto & s : States) {
        if (s->type == eElementType::State) {
            auto cs = std::dynamic_pointer_cast<CState>(*s);
            std::list<std::string>   sslist;

            sslist = cs->GetHistoryAttributes(prefix);
            smlist.insert(smlist.end(), sslist.begin(), sslist.end());
        }
    }
    return (smlist);
}


void CState::DumpSetStateSwitch(std::ostream &output, std::string prefix) {
    prefix += name;
    //
    //  The state contains a statemachine.
    if (!States.empty()) {
        output << "    case IDA_" << helper::toupper(prefix + "State") << ":\n"
               << "        " << prefix << "State = value;\n"
               << "        break;\n";
        for (auto & st : States) {
            auto substates = std::dynamic_pointer_cast<CState>(*st);

            substates->DumpSetStateSwitch(output, prefix);
        }
    }
}

void CState::DumpGetStateSwitch(std::ostream &output, std::string prefix) {
    prefix += name;
    //
    //  The state contains a statemachine.
    if (!States.empty()) {
        output << "    case IDA_" << helper::toupper(prefix + "State") << ":\n"
               << "        retval = " << prefix << "State;\n"
               << "        break;\n";
        for (auto & st : States) {
            auto substates = std::dynamic_pointer_cast<CState>(*st);

            substates->DumpGetStateSwitch(output, prefix);
        }
    }
}

void CState::DumpStateEnumerators(std::ostream &output, std::string prefix) {
    prefix = prefix+helper::normalize(name);

    output << "/*\n"
              " *  State Enumerator for " << prefix << std::endl;
    output << " */\n"
        "struct e" << prefix << "State {\n";

    int stateCount = 0;

    for (auto & st : States) {
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
    for (auto & st : States) {
        if (st->type != eElementType::PseudoState) {
            if (!std::dynamic_pointer_cast<CState>(*st)->States.empty()) {
                std::dynamic_pointer_cast<CState>(*st)->DumpStateEnumerators(output, prefix);
            }
        }
    }
}

void CState::GetStateVars(std::list<std::pair<std::string, std::string> >& aAttrList,
                          std::map<std::string, uint64_t>& aIdMap,
                          std::string prefix) {
    prefix+=helper::normalize(name);
    //
    //  Add the state attribute for the toplevel statemachine
    std::string lStateVar;

    if (prefix.empty()) {
        lStateVar = "state";
    } else {
        lStateVar = prefix+"State";
    }
    Crc64 crc;
    aAttrList.emplace_front("tMemberValue", lStateVar);
    std::string uidx=std::string("IDA_")+helper::toupper(lStateVar);
    aIdMap.insert(std::pair<std::string, uint64_t>(uidx, crc.calc(uidx) ));

    statevar   = lStateVar;
    stateclass = "e";
    stateclass+= lStateVar;

    //
    //  Search through the states to find composite states.
    for (auto & st : States) {
        if (st->type != eElementType::PseudoState) {
            if (!std::dynamic_pointer_cast<CState>(*st)->States.empty()) {
                std::dynamic_pointer_cast<CState>(*st)->GetStateVars(aAttrList, aIdMap, prefix);
            }
        }
    }
}

std::shared_ptr<MState> CState::GetInitialState() {
    std::shared_ptr<MState> retval;
    std::vector<MState*>::iterator st;

    //
    //  The initial state is one of two styles.
    //  A pseudo state of kind "initial"
    //  A state with no incoming transitions.
    for (auto & st : States) {
        if (st->type == eElementType::PseudoState) {
            auto ps = std::dynamic_pointer_cast<MPseudoState>(*st);

            if (helper::tolower(ps->kind) == "initial") {
                retval = ps;
                break;
            }
        } else {
            auto state = std::dynamic_pointer_cast<MState>(*st);

            if (state->Incoming.empty()) {
                retval = state;
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
                retval = std::dynamic_pointer_cast<MState>(*std::dynamic_pointer_cast<MTransition>(*(ps->Outgoing[0]))->to);
            }
        }
    }
    return retval;
}


bool CState::isInitial() {
    bool result = false;

    if ((type == eElementType::PseudoState) && (helper::tolower(static_cast<CPseudoState*>(static_cast<MState*>(this))->kind) == "initial")) {
        result = true;
    }
    return result;
}
