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
#ifndef MAIN_INC
#define MAIN_INC

#include <string>
#include <set>
#include "types.h"

extern std::string gModelPath;
extern std::string gDumpId;
extern bool        gDumpStarted;
extern std::set<std::string> gDumpList;

extern std::string gPackageHeaderDir;

extern bool        gNameSpaceInCPP;

extern bool        gUseSelfContainedHeaderInCPP;
extern bool        gGenerateSelfContainedHeader;
extern bool        gUseSelfContainedInModelHeader;

extern bool        gUsePragmaOnce;
extern bool        gUseNameSpaceInGuards;
extern std::string gHeaderGuardExtension;

extern int         IndentSize;

extern std::string gCxxFileHeader;
extern std::string gCxxHppFileHeader;
extern std::string gCxxHFileHeader;
extern std::string gCxxCppFileHeader;

extern bool        gRuleOfFive;

extern bool        gConstInArgumentRight;
extern bool        gConstInReturnTypeRight;

extern std::string gDoxygenCommentStart;

extern std::string gStaticMemberPrefix;   // This is not used to automatic extend the member name
extern std::string gStaticArgumentPrefix; //  This is not used to automatic exend the argument name
extern bool        gInitMemberDefaultInInitializerList; // Member defaults shall be set through initializer list.

extern int         gVerboseOutput;

extern std::set<std::string> gCardinalTypes;
//
//  Message and signal specific
extern std::string gMsgPrefix;
extern std::string gSignalPrefix;

bool isCardinalType(const std::string& aType) ;

#endif
