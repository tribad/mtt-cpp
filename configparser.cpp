//
// Copyright 2023 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#include "helper.h"
#include "configparser.h"
#include "main.h"

extern std::string directory;

bool loadModelConfiguration(std::shared_ptr<MttXmlNode> docRoot) {
    bool success = false;

    std::shared_ptr<MttXmlNode> modelConfig = docRoot->findChild("model");
    if (modelConfig) {
        gModelPath = modelConfig->stringProperty("path");
        gDumpId    = modelConfig->stringProperty("id");
        if (!gDumpId.empty()) {
            gDumpStarted =false;
        }
    }
    std::shared_ptr<MttXmlNode> outputConfig = docRoot->findChild("output");

    if (outputConfig) {
        std::string dir = outputConfig->stringProperty("directory", directory);

        if ((!dir.empty()) && (dir[dir.size()-1] != '/')) {
            dir.push_back('/');
            directory = dir;
        } 
    }
    success = true;

    return success;
}

static bool loadCxxFileHeader(std::shared_ptr<MttXmlNode> aCxxFileHeader) {
    bool success = false;

    if (aCxxFileHeader) {
        gCxxFileHeader  = aCxxFileHeader->content();
        success         = true;
    }
    return success;
}

static bool loadCxxHppFileHeader(std::shared_ptr<MttXmlNode> aCxxHppFileHeader) {
    bool success = false;

    if (aCxxHppFileHeader) {
        gCxxHppFileHeader  = aCxxHppFileHeader->content();
        success         = true;
    }
    return success;
}
static bool loadCxxHFileHeader(std::shared_ptr<MttXmlNode> aCxxHFileHeader) {
    bool success = false;

    if (aCxxHFileHeader) {
        gCxxHFileHeader  = aCxxHFileHeader->content();
        success         = true;
    }
    return success;
}
static bool loadCxxCppFileHeader(std::shared_ptr<MttXmlNode> aCxxCppFileHeader) {
    bool success = false;

    if (aCxxCppFileHeader) {
        gCxxCppFileHeader  = aCxxCppFileHeader->content();
        success         = true;
    }
    return success;
}

static bool loadCxxCodeStyle(std::shared_ptr<MttXmlNode> aCxxCodeStyle) {
    bool success = false;

    if (aCxxCodeStyle) {
        std::shared_ptr<MttXmlNode> nsconfig = aCxxCodeStyle->findChild ("namespace");

        if (nsconfig) {
            gNameSpaceInCPP = nsconfig->boolProperty("inSource" , false);
        }

        std::shared_ptr<MttXmlNode> arguments = aCxxCodeStyle->findChild ("arguments");

        if (arguments) {
            gConstInArgumentRight = arguments->boolProperty("constright", false);
        }

        std::shared_ptr<MttXmlNode> returntype = aCxxCodeStyle->findChild ("returntype");

        if (returntype) {
            gConstInReturnTypeRight = returntype->boolProperty("constright", false);
        }

        std::shared_ptr<MttXmlNode> headerfiles = aCxxCodeStyle->findChild ("headerfiles");

        if (headerfiles) {
            gGenerateSelfContainedHeader = headerfiles->boolProperty("selfcontained", false);

            if (gGenerateSelfContainedHeader) {
                gUseSelfContainedHeaderInCPP = headerfiles->boolProperty("useincpp", false);
            } else {
                gUseSelfContainedHeaderInCPP = false;
            }
        }

        std::shared_ptr<MttXmlNode> headerguards = aCxxCodeStyle->findChild ("headerguards");

        if (headerguards) {
            gUsePragmaOnce = headerguards->boolProperty("usepragma", true);
            gUseNameSpaceInGuards = headerguards->boolProperty("usenamespace", false);
            gHeaderGuardExtension = headerguards->stringProperty("guardextension", "_INC");
        }

        std::shared_ptr<MttXmlNode> member = aCxxCodeStyle->findChild ("member");

        if (member) {
            gInitMemberDefaultInInitializerList = member->boolProperty("initlistdefault", true);
            gStaticMemberPrefix = member->stringProperty("memberprefix", "m");
            gStaticArgumentPrefix = member->stringProperty("argumentprefix", "a");
        }

        std::shared_ptr<MttXmlNode> classconfig = aCxxCodeStyle->findChild ("class");

        if (classconfig) {
            gRuleOfFive = classconfig->boolProperty("ruleoffive", false);
        }

        success         = true;
    }
    return success;
}

static bool loadCxxMembers(std::shared_ptr<MttXmlNode> aCxxMembers) {
    bool success = true;

    if (aCxxMembers) {
    }
    return success;
}

bool loadCxxGeneratorConfiguration(std::shared_ptr<MttXmlNode> docRoot) {
    bool success = false;
    std::shared_ptr<MttXmlNode> cxx = docRoot->findChild("cxx");

    if (cxx) {
        success = loadCxxFileHeader(cxx->findChild("fileheader"));
        success = loadCxxHppFileHeader(cxx->findChild("hpp-fileheader"));
        success = loadCxxHFileHeader(cxx->findChild("h-fileheader"));
        success = loadCxxCppFileHeader(cxx->findChild("cpp-fileheader"));
        success = loadCxxCodeStyle( cxx->findChild( "codestyle"));
        success = loadCxxMembers( cxx->findChild( "members"));
        //
        //  Fill the headers if not complete available.
        if (gCxxCppFileHeader.empty()) {
            gCxxCppFileHeader = gCxxFileHeader;
        }
        if (gCxxHppFileHeader.empty()) {
            gCxxHppFileHeader = gCxxFileHeader;
        }
        if (gCxxHFileHeader.empty()) {
            gCxxHFileHeader = gCxxFileHeader;
        }
        std::shared_ptr<MttXmlNode> packageheader = cxx->findChild ("packageheader");

        if (packageheader) {
            gPackageHeaderDir = packageheader->stringProperty("directory", "./");
            if (!gPackageHeaderDir.empty() && (gPackageHeaderDir[gPackageHeaderDir.size()-1] != '/')) {
                gPackageHeaderDir.push_back('/');
            }
        }

        std::shared_ptr<MttXmlNode> inmodelheader = cxx->findChild ("inmodelheader");

        if (inmodelheader) {
            gUseSelfContainedInModelHeader = inmodelheader->boolProperty("useselfcontained", false);
        }


        std::shared_ptr<MttXmlNode> doxygen = cxx->findChild("doxygen");

        if (doxygen) {
            gDoxygenCommentStart = doxygen->stringProperty("commentstart", "///");
        }
        

    }

    success = true;
    return success;
}


