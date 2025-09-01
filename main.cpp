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
#include <stdio.h>
#include <limits.h>
#include <set>

#ifdef __linux__
#include <unistd.h>
#endif

#include "version.h"

#include "helper.h"

#include "staruml/parser.h"
#include "ea/ea_eap_parser.h"
#include "mstate.h"
#include "mmodel.h"
#include "cmodel.h"

#include "path.h"

#include "configparser.h"
#include "main.h"
#include "mevent.h"
#include "mgeneralization.h"
#include "minteraction.h"
#include "moperation.h"
#include "mparameter.h"
#include "mpin.h"
#include "mstatemachine.h"
#include "maction.h"
#include "mactionnode.h"
#include "mactivity.h"
#include "martifact.h"
#include "mdecisionnode.h"
#include "mcollaboration.h"
#include "mfinalnode.h"
#include "mforknode.h"
#include "minitialnode.h"
#include "mjoinnode.h"
#include "mjslifeline.h"
#include "mmergenode.h"
#include "musecase.h"
#include "umldiagram.h"

std::shared_ptr<CModel> loadedmodel;
//
//  C++ specific configuration.
bool        gNameSpaceInCPP = true;
bool        gUseSelfContainedHeaderInCPP = false;
bool        gGenerateSelfContainedHeader = false;
bool        gUseSelfContainedInModelHeader = false;

bool        gUsePragmaOnce = true;
bool        gUseNameSpaceInGuards = true;
std::string gHeaderGuardExtension = "_INC";

bool        gRuleOfFive = false;

bool        gConstInArgumentRight   = false;
bool        gConstInReturnTypeRight = false;

std::string gCxxFileHeader;
std::string gCxxHppFileHeader;
std::string gCxxHFileHeader;
std::string gCxxCppFileHeader;

std::string gStaticMemberPrefix = "m";   // This is not used to automatic extend the member name
std::string gStaticArgumentPrefix = "a"; //  This is not used to automatic exend the argument name
bool        gInitMemberDefaultInInitializerList= false;
//
//  Global configuration
int         IndentSize=4;
std::string gModelPath;
std::string gDumpId;
bool        gDumpStarted = true;
std::set<std::string> gDumpList;
std::string directory;
std::string gPackageHeaderDir = "./";

std::string gDoxygenCommentStart="///";
int         gVerboseOutput = 0;

std::set<std::string> gCardinalTypes = {"int", "long", "bool", "float", "double", "short", "char", "unsigned", "void"};

bool isCardinalType(const std::string& aType) {
    bool retval = true;

    if (gCardinalTypes.find(aType) == gCardinalTypes.end()) {
        retval = false;
    }
    return retval;
}
//
//  Simulation specific configuration.
long  simversion=2;

bool doDump = false;

static bool loadConfig(const std::string& aConfigFile) {
    bool success = false;

    MttXmlDoc   xmlConfig;

    if (xmlConfig.readFile(aConfigFile.c_str())) {
        std::shared_ptr<MttXmlNode> docRoot = xmlConfig.getRoot();

        if (docRoot) {
            loadModelConfiguration(docRoot);
            loadCxxGeneratorConfiguration(docRoot);
            std::shared_ptr<MttXmlNode> modelConfig = docRoot->findChild("model");
            if (modelConfig) {
                gModelPath = modelConfig->stringProperty("path");
            }
            success = true;
        }

    }
    return success;
}

int main(int argc, char** argv) {
#ifdef __linux__
    char        path[PATH_MAX];
    char        dest[PATH_MAX];
#endif
    std::string application;
    std::string libpath;
    std::string configurationfile;
    //
    //  This is the only way on a linux system to get the real path to the application even if
    //  it gets started through various links.
#ifdef __linux
    pid_t pid = getpid();
    sprintf(path, "/proc/%d/exe", pid);
    if (readlink(path, dest, PATH_MAX) == -1) {
        application=argv[0];
    } else {
        application=dest;
    }
#else
    application=argv[0];
#endif
    int   i = 1;
    char* s;

    while (i<argc) {
        s = argv[i];
        if (*s == '-') {
            s++;
            switch (*s) {
            case 'c':       // xml configuration file
                s++;
                if (*s != 0) {
                    configurationfile = s;
                } else {
                    i++;
                    if (argv[i] != 0) {
                        configurationfile = argv[i];
                    } else {
                    }
                }
                break;
            case 'd':       //  directory
                s++;
                if (*s != 0) {
                    directory = s;
                } else {
                    i++;
                    if (argv[i] != 0) {
                        directory = argv[i];
                    } else {
                    }
                }
                if ((!directory.empty()) && (directory[directory.size()-1] != '/')) {
                    directory.push_back('/');
                }
                break;
            case 's':       //  simulation object version
                s++;
                if (*s != 0) {
                    simversion = strtol(s, 0, 0);
                } else {
                    i++;
                    if (argv[i] != 0) {
                        simversion = strtol(argv[i], 0, 0);
                    } else {
                    }
                }
                break;
            case 'v':
                std::cerr << "mtt-cpp-" << MTT_CPP_VERSION << std::endl;
                std::cerr << "Copyright by The Simulated-Universe. Hans-J�rgen Lange <hjl@simulated-universe.de>" << std::endl;
                break;
            case '?':
            case 'h':
                std::cerr << "usage:\n"
                             "\t-d : Set the directory where to start the output in.\n"
                             "\t-c : The name of an configuration file.\n"
                             "\t-v : Show the version information on startup.\n"
                             "\t-?\n"
                             "\t-h : Show this help\n";
                exit(0);
            default:
                break;
            }
        } else {
            gModelPath = argv[i];
        }
        i++;
    }
    if (!configurationfile.empty()) {
        if (!loadConfig(configurationfile)) {
            std::cerr << "Cannot load configuration file '" << configurationfile << "' ... aborting" << std::endl;
            exit(-1);
        }
    }
#ifdef NDEBUG
    std::string wd = helper::getcwd();
    std::string lockpath = "./" + directory + ".lock";
    while (helper::mkdir(lockpath, 0766, true) == -1) {
        std::cerr << "locked .... waiting\n";
#ifdef __linux
        sleep(2);
#else
        Sleep(1000);
#endif
    }
#endif

    if (!gModelPath.empty()) {
        CPath fname(gModelPath);
        std::cerr << "Generating into :" << helper::getcwd() << "::" << directory << ":" << std::endl;
        //
        //  We are ready to generate.
#ifndef __linux
        CoInitialize(NULL);
#endif
        if (fname.Extension() == "mdj") {
            loadedmodel=std::dynamic_pointer_cast<CModel>(staruml_modelparser(gModelPath.c_str(), directory.c_str()));
        } else if ((fname.Extension() == "eap") || (fname.Extension() == "eapx")) {
            loadedmodel = std::dynamic_pointer_cast<CModel>(ea_eap_modelparser(gModelPath.c_str(), directory.c_str()));
        } else {
            //
            //  Not a file. Checking for DSN=
            if (gModelPath.find("DSN=") != std::string::npos) {
                std::cerr << "Found DSN " << gModelPath.c_str() << " using odbc" << std::endl;
                loadedmodel = std::dynamic_pointer_cast<CModel>(ea_eap_modelparser(gModelPath.c_str(), directory.c_str()));
            } else if (gModelPath.find("OLEDB=") != std::string::npos) {
                std::cerr << "Found OLEDB " << gModelPath.c_str() << " using oledb" << std::endl;
                loadedmodel = std::dynamic_pointer_cast<CModel>(ea_eap_modelparser(gModelPath.c_str(), directory.c_str()));
            } else {
                std::cerr << "Cannot determine the model file format from the extension " << gModelPath.c_str() << " ....exiting\n";
#ifndef __linux
                CoUninitialize();
#endif
                exit(-1);
            }
        }
#ifndef __linux
        CoUninitialize();
#endif
        loadedmodel->Prepare();
        loadedmodel->Dump();
        loadedmodel = std::shared_ptr<CModel>();
        MAction::Instances.clear();
        MActionNode::Instances.clear();
        MActivity::Instances.clear();
        MArtifact::Instances.clear();
        MAssociation::Instances.clear();
        MAssociationEnd::Instances.clear();
        MAttribute::Instances.clear();
        MCollaboration::Instances.clear();
        MDecisionNode::Instances.clear();
        MDependency::Instances.clear();
        MEdge::Instances.clear();
        MElement::Instances.clear();
        MEvent::Instances.clear();
        MFinalNode::Instances.clear();
        MForkNode::Instances.clear();
        MGeneralization::Instances.clear();
        MInitialNode::Instances.clear();
        MInteraction::Instances.clear();
        MJoinNode::Instances.clear();
        MJSLifeLine::Instances.clear();
        MLifeLine::Instances.clear();
        MMergeNode::Instances.clear();
        MMessage::Instances.clear();
        MNode::Instances.clear();
        MObject::Instances.clear();
        MOperation::Instances.clear();
        MPackage::Instances.clear();
        MParameter::Instances.clear();
        MPin::Instances.clear();
        MStatemachine::Instances.clear();
        MUseCase::Instances.clear();
        UmlDiagram::Instances.clear();
        MClass::Instances.clear();
        MState::Instances.clear();
    } else {
        std::cerr << "No model file set\n";
    }
#ifdef NDEBUG
    helper::chdir(wd);
    //std::cerr << "WD: " << helper::getcwd() << std::endl;
    helper::rmdir(lockpath);
#endif
    return (0);
}
