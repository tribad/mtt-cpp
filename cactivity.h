//
// Copyright 2018 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#ifndef CACTIVITY_H
#define CACTIVITY_H

#include <string>
#include <list>
#include <vector>

class MElement;
class MNode;
class MActivty;
class CDecisionNode;
class CActionNode;
class CMergeNode;
class MEdge;
class MPin;
class CEdge;
class MToken;

class CActionBlock {
public:
    CActionBlock() = default;
    ~CActionBlock() = default;
public:
    std::shared_ptr<MNode> mEntry;
    std::shared_ptr<MNode> mExit;
    std::list<std::shared_ptr<MElement>> mBlock;
};

class CActivity : public MActivity
{
public:
    CActivity() = default;
    CActivity(const std::string& aId, std::shared_ptr<MElement> e) : MActivity(aId, e) {}
    virtual ~CActivity() = default;
    //
    //  Virtuals from MElement
    virtual std::string FQN(void) const;
    virtual void Prepare(void);
    virtual void Dump(std::shared_ptr<MModel> aModel);

    void DumpCxx(std::ostream& src);
    void DumpCxx(std::ostream& src, std::shared_ptr<MNode> node);
//    void DumpCxx(std::ostream& src, MNode* node, int level = 0);
    void DumpCxx(std::ostream& src, std::shared_ptr<CDecisionNode> node, int level = 0);
    void DumpCxx(std::ostream& src, std::shared_ptr<CActionNode> node, int level = 0);
    void DumpCxx(std::ostream& src, std::shared_ptr<CMergeNode> node, int level = 0);

    void DumpCxxCreate(std::ostream& src, std::shared_ptr<CActionNode> node, int level = 0);
    void DumpCxxWrite(std::ostream& src, std::shared_ptr<CActionNode> node, int level = 0);
    void DumpCxxSendSignal(std::ostream& src, std::shared_ptr<CActionNode> node, int level = 0);
    void DumpCxx(std::ostream &src, std::shared_ptr<MEdge> edge) ;
    void Dump(std::ostream& src, CActionBlock& segment);
    std::vector<std::shared_ptr<MNode>> GetInitialNode();
    std::shared_ptr<MEdge> FindEdge(std::shared_ptr<MPin> pin);
    void FillLevel(std::shared_ptr<MNode> node, std::shared_ptr<MToken> level) ;
    void SendToken(std::shared_ptr<MNode> node, MToken& token);
    void SendToken(std::shared_ptr<MEdge> edge, MToken& token);

    std::list<std::shared_ptr<MElement>> CreateBlock(std::shared_ptr<MElement> aStart);
    std::list<std::shared_ptr<MElement>> GetDumpNodes(std::shared_ptr<MNode> start);
    std::list<std::shared_ptr<MElement>> GetDumpNodes(std::shared_ptr<MEdge> start);
public:
    std::vector<std::shared_ptr<MNode>> activNodes;
    std::list<std::list<std::shared_ptr<MElement> > > mBlock;
};

#endif // CACTIVITY_H
