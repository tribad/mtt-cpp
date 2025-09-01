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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include <set>

#include "helper.h"
#include "mattribute.h"
#include "cattribute.h"
#include "massociationend.h"
#include "cassociationend.h"
#include "mparameter.h"
#include "moperation.h"
#include "cclassbase.h"
#include "cjsclass.h"

#include "mjsonmessage.h"
#include "cjsonmessage.h"
#include "cmessageclass.h"

#include "csignalclass.h"

#include "mmodel.h"
#include "cmodel.h"
#include "coperation.h"

//  This method first creates a backup file before opening the stream
void CJSClass::OpenStream(std::ofstream& s, const std::string& fname)
{
    int         err=0;
    struct stat dirstat;
    std::string newname;
    //
    //  First check for an existing file and create a backup.
    err=stat(fname.c_str(), &dirstat);
    if (err == 0) {
        newname=fname+".bak";
        err=stat(newname.c_str(), &dirstat);
        if (err == 0) {
            remove(newname.c_str());
        }
        rename(fname.c_str(), newname.c_str());
    }
    s.open(fname);
}

void CJSClass::SetFromTags(const std::string& name, const std::string&value)
{
}

std::string CJSClass::FQN() const {
    return name;
}

void CJSClass::Prepare(void) {
    std::map<std::string, std::string>::const_iterator i;

    lower_name = helper::tolower(name);
    upper_name = helper::toupper(name);
    basename   = name;

    for (i=tags.begin(); i!= tags.end(); ++i) {
        if (i->first=="BaseName") {
            basename = i->second;
        }
    }

}

void CJSClass::Dump(std::shared_ptr<MModel> model) {
#if 0
    std::string path;
    auto cmodel = std::dynamic_pointer_cast<CModel>(model);


    path=cmodel->pathstack.back()+"/"+"."+basename+".js";
    cmodel->generatedfiles.push_back( tGenFile {path, id, "//", "jscript"});
    OpenStream(out, path);

    out << "/*\n"
           " */\n";


    if (!Attribute.empty()) {
        out << "/*\n"
               " *  These are some global vars\n"
               " */\n";
        for (auto & ai : Attribute) {
            auto attr = std::dynamic_pointer_cast<CAttribute>(*ai);

            out << "var " << attr->name;
            if (!attr->defaultValue.empty()) {
                out << " = " << attr->defaultValue;
            }

            out << ";\n";
        }
    }
    if (!Operation.empty()) {
        out << "/*\n"
               " *  These are the operations.\n"
               " */\n";
        for (auto & oi : Operation) {
            auto operation = std::dynamic_pointer_cast<COperation>(*oi);

            if (operation->name == name) {
                out << "function " << operation->name << "(" ;
            } else {
                out << name << ".prototype." << operation->name << " = function (" ;
            }
            for (auto & pi : operation->Parameter) {
                if (pi != *operation->Parameter.begin()) {
                    out << ", ";
                }
                out << (*pi)->name;
            }

            out << ") {\n";
            out << "// User-Defined-Code:" << (*oi)->id << "\n";
            out << "// End-Of-UDC:" << (*oi)->id << "\n";
            out << "}\n\n";
        }
    }
    //
    //  Create operations for outgoing JSON messages
    std::set<std::string>            donelist;

    for (auto & mo : Outgoing) {
        if (mo->type == eElementType::JSONMessage) {

            auto s = std::dynamic_pointer_cast<CJSONMessage>(*mo);
            s->FillClass();
            if (s->Class && (s->Class->type == eElementType::SimMessageClass)) {
                auto sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());

                if (sc) {
                    if (donelist.find(sc->name) == donelist.end()) {
                        out << "function send_" << s->name << "(";
                        bool firstattr=true;

                        for (auto & ai : sc->Attribute) {
                            if (ai->visibility == vPublic) {
                                if (!firstattr) {
                                    out << ", ";
                                } else {
                                    firstattr = false;
                                }
                                out << helper::tolower(ai->name);
                            }
                        }
                        out << ") {\n";
                        out << "    var msg = {\n";
                        out << "              MsgId: \"" << s->name << "\",\n";
                        out << "              Destination: 0";

                        for (auto & ai : sc->Attribute) {
                            if (ai->visibility == vPublic) {
                                out << ",\n";
                                out << "              " << ai->name << ": " << helper::tolower(ai->name);
                            }
                        }
                        out << "\n              };\n\n";
                        out << "// User-Defined-Code: send-" << s->id << "\n";
                        out << "// End-Of-UDC: send-" << s->id << "\n";
                        out << "}\n\n";
                        donelist.insert(sc->name);
                    }
                } else {
                    std::cerr << "No Class definition found for outgoing JSON-Message:" << s->name << ":\n";
                }
            } else {
                if (s->Class && (s->Class->type == eElementType::SimSignalClass)) {
                    auto sc = std::dynamic_pointer_cast<CSignalClass>(s->GetClass());

                    if (sc) {
                        if (donelist.find(sc->name) == donelist.end()) {
                            out << "function send_" << s->name << "(";
                            bool firstattr=true;
                            for (auto & ai : sc->Attribute) {
                                if (ai->visibility == vPublic) {
                                    if (!firstattr) {
                                        out << ", ";
                                    } else {
                                        firstattr = false;
                                    }
                                    out << helper::tolower(ai->name);
                                }
                            }
                            out << ") {\n";
                            out << "    var msg = {\n";
                            out << "              SignalId: \"" << s->name << "\"";

                            for (auto & ai : sc->Attribute) {
                                if (ai->visibility == vPublic) {
                                    out << ",\n";
                                    out << "              " << ai->name << ": " << helper::tolower(ai->name);
                                }
                            }
                            out << "\n              };\n\n";
                            out << "// User-Defined-Code: send-" << s->id << "\n";
                            out << "// End-Of-UDC: send-" << s->id << "\n";
                            out << "}\n\n";
                            donelist.insert(sc->name);
                        }
                    } else {
                        std::cerr << "No Class definition found for outgoing JSON-Message:" << s->name << ":\n";
                    }
                } else {

                }
            }
        } else {
        }
    }

    donelist.clear();

    for (auto & mi : Incoming) {
        if (mi->type == eElementType::JSONMessage) {
            auto s = std::dynamic_pointer_cast<CJSONMessage>(*mi);
            s->FillClass();

            auto sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());
            if (sc) {
                if (donelist.find(sc->name)==donelist.end()) {
                    out << "function recv_" << s->name << "(msg) {\n";
                    out << "// User-Defined-Code: recv-" << s->id << "\n";
                    out << "// End-Of-UDC: recv-" << s->id << "\n";
                    out << "}\n\n";
                    donelist.insert(sc->name);
                } else {
                }
            } else {
                std::cerr << "No Class definition found for incoming JSON-Message:" << s->name << ":\n";
            }
        } else {
        }
    }

    donelist.clear();

    out << "function __msg_dispatcher(evt) {\n";
    out << "    var msg = JSON.parse(evt.data);\n\n";
    out << "    if (\"MsgId\" in msg) {\n";
    out << "        switch(msg.MsgId) {\n";
    for (auto & mi : Incoming) {
        if (mi->type == eElementType::JSONMessage) {
            auto s = std::dynamic_pointer_cast<CJSONMessage>(*mi);
            s->FillClass();
            if (s->Class && (s->Class->type == eElementType::SimMessageClass)) {
                auto sc = std::dynamic_pointer_cast<CMessageClass>(s->GetClass());
                if (sc) {
                    if (donelist.find(sc->name)==donelist.end()) {
                        out << "        case \"" << s->name << "\":\n";
                        out << "            recv_" << s->name << "(msg);\n";
                        out << "            break;\n";
                        donelist.insert(sc->name);
                    } else {
                    }
                } else {
                    std::cerr << "No Class definition found for incoming JSON-Message:" << s->name << ":\n";
                }
            }
        } else {
        }
    }
    out << "        }\n";
    out << "        return;\n";
    out << "    }\n";
    out << "    if (\"SignalId\" in msg) {\n";
    out << "        switch(msg.SignalId) {\n";
    for (auto & mi : Incoming) {
        if (mi->type == eElementType::JSONMessage) {
            auto s = std::dynamic_pointer_cast<CJSONMessage>(*mi);
            s->FillClass();
            if (s->Class && (s->Class->type == eElementType::SimSignalClass)) {
                auto sc = std::dynamic_pointer_cast<CSignalClass>(s->GetClass());
                if (sc) {
                    if (donelist.find(sc->name)==donelist.end()) {
                        out << "        case \"" << s->name << "\":\n";
                        out << "            recv_" << s->name << "(msg);\n";
                        out << "            break;\n";
                        donelist.insert(sc->name);
                    } else {
                    }
                } else {
                    std::cerr << "No Class definition found for incoming JSON-Message:" << s->name << ":\n";
                }
            }
        } else {
        }
    }
    out << "        }\n";
    out << "        return;\n";
    out << "    }\n";
    out << "}\n";
    out.close();
#endif
}
