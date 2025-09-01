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
#ifndef CSTATE_H
#define CSTATE_H


class CState : public MState
{
public:
    CState() = default;
    CState(const std::string& aId, std::shared_ptr<MElement> e) : MState(aId, e) {};
    ~CState() override = default;
    //
    //  Virtuals from MElement
    std::string FQN(void) const override;
    void Prepare(void) override;
    void Dump(std::shared_ptr<MModel> aModel) override;

    std::list<std::string> GetHistoryAttributes(std::string prefix) ;
    void GetStateVars(std::list<std::pair<std::string, std::string> >& aAttrList,
                      std::map<std::string, uint64_t>& aIdMap,
                      std::string prefix);

    void DumpStateEnumerators(std::ostream& output, std::string prefix);
    void DumpSetStateSwitch(std::ostream& output, std::string prefix);
    void DumpGetStateSwitch(std::ostream& output, std::string prefix);
    std::shared_ptr<MState> GetInitialState();
    bool isInitial();
public:
    std::string stateclass;
    std::string statevar;
};


#endif // CSTATE_H
