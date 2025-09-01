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
#ifndef MSTATE_H
#define MSTATE_H

#include <vector>
#include <string>
#include "melement.h"
#include "mtransition.h"

class MTransition;

class MState : public MElement
{
public:
    MState() = default;
    MState(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
    static std::shared_ptr<MState> construct(const std::string&aId, std::shared_ptr<MElement> aParent = nullptr);
public:
    static std::map<std::string, std::shared_ptr<MState>> Instances;
    std::vector<MElementRef> Incoming;
    std::vector<MElementRef> Outgoing;
    std::vector<MElementRef> States;
    std::vector<MElementRef> Transitions;
    std::vector<MElementRef> DoActions;
    std::vector<MElementRef> EntryActions;
    std::vector<MElementRef> ExitActions;
    MElementRef              shallowHistory;
};

#endif // MSTATE_H
