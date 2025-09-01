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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include <iomanip>

#include "helper.h"
#include "crc64.h"
#include "cclassbase.h"
#include "csimenumeration.h"
#include "csimobjectv2.h"
#include "cpackagebase.h"
#include "csimulationpackage.h"
#include "clibrarypackage.h"
#include "cexternpackage.h"
#include "cexecutablepackage.h"
#include "cmodelpackage.h"
#include "mmessage.h"
#include "mjsonmessage.h"
#include "cjsonmessage.h"
#include "cmessageclass.h"
#include "csignalclass.h"

#include "cmodel.h"
#include "ccxxclass.h"

extern long simversion;

std::string CSimulationPackage::FQN() const {
    return name;
}

void CSimulationPackage::Prepare(void) {
    if (!m_init_done) {
        //
        //  Add the mIfcClass to the package.
        auto tmp = MClass::construct("__simifc__", "Cxx", sharedthis<MElement>());
        mIfcClass = tmp->sharedthis<CCxxClass>();

        PrepareBase(tags);
        if (HasTaggedValue("simulationname")) {
            SimulationName=GetTaggedValue("simulationname");
        }
        if (HasTaggedValue("modelname")) {
            SimulationName=GetTaggedValue("modelname");
        }
        if (SimulationName.empty()) {
            SimulationName = name;
        }
        if (OutputName.empty()) {
            OutputName = SimulationName;
        }
        for (auto & i : Classes) {
            i->Prepare();
        }
        for (auto & di : Dependency) {
            di->Prepare();
        }

        for (auto & pi : Packages) {
            pi->Prepare();
        }
        m_init_done = true;
    }
}

void CSimulationPackage::DumpExtraIncludes(std::ofstream &ifc, std::string aHeaders, std::set<std::string> &aDoneIncludes) {
    std::string extra;
    size_t      start       = 0;
    size_t      end         = 0;

    if (!aHeaders.empty()) {
        do {
            end = aHeaders.find_first_of(" ", start);
            if (end != std::string::npos) {
                extra = aHeaders.substr(start, end-start);
            } else {
                extra = aHeaders.substr(start);
            }
            /*
             * trim front
             */
            while (extra[0]==' ') extra.erase(0, 1);
            /*
             * if extraheader has a size after trimming we can add it to the
             * list of extra headers.
             */
            if (extra.size()>0) {
                /*
                 * First check if we have it already.
                 */
                if (aDoneIncludes.find(extra) == aDoneIncludes.end()) {
                    ifc << "#include <" << extra << ">\n";
                    /*
                     * Anyway the header is part of the model or a real extra
                     * we put into the donelist to prevent processing it again.
                     */
                    aDoneIncludes.insert(extra);
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
}

void CSimulationPackage::DumpExtraIncludes(std::ofstream &ifc, std::shared_ptr<CClassBase> aClass, std::set<std::string>& aDoneIncludes) {
    DumpExtraIncludes(ifc, aClass->GetExtraHeader(), aDoneIncludes);

    for (auto em : aClass->extramodelheader) {
        DumpExtraIncludes(ifc, em->GetExtraHeader(), aDoneIncludes);
    }
}

void CSimulationPackage::DumpExtraIncludes(std::ofstream &ifc, std::set<std::string>& aDoneIncludes) {
    ifc << "#include <simifc.h>\n";
    ifc << "#include <stdint.h>\n";
    ifc << "#include <stdlib.h>\n";
    ifc << "#include <simobjfactory.h>\n";
    ifc << "#include <simapi.h>\n";
    ifc << "#include <vector>\n";
    ifc << "#include <map>\n";
    ifc << "#include <string>\n";
    ifc << "#include <sstream>\n";
    ifc << "#ifdef __linux__\n";
    ifc << "#include <pthread.h>\n";
    ifc << "#endif\n";
    ifc << "#include <set>\n";
    ifc << "#include <pointerindex.h>\n";
    ifc << "#include <helper.h>\n";


    aDoneIncludes.insert("pthread.h");
    aDoneIncludes.insert("set");
    aDoneIncludes.insert("string");
    aDoneIncludes.insert("sstream");
    aDoneIncludes.insert("map");
    aDoneIncludes.insert("vector");
    aDoneIncludes.insert("helper.h");
    aDoneIncludes.insert("simobjfactory.h");
    aDoneIncludes.insert("simapi.h");
    aDoneIncludes.insert("stdlib.h");
    aDoneIncludes.insert("stdint.h");
    aDoneIncludes.insert("simifc.h");
    aDoneIncludes.insert("pointerindex.h");
    //
    //  We check all classes for some info we need here
    for (auto & so : content) {
        if (so->type == eElementType::SimObject) {
            DumpExtraIncludes(ifc, std::dynamic_pointer_cast<CClassBase>(so), aDoneIncludes);
        }
    }
    //
    //  All message classes we check for extras.
    for (auto & mc : MClass::Instances) {
        if ((mc.second->type == eElementType::SimMessageClass) || (mc.second->type == eElementType::SimSignalClass)) {
            DumpExtraIncludes(ifc, std::dynamic_pointer_cast<CClassBase>(mc.second), aDoneIncludes);
        }
    }
}
//
//  This is the main-dump function to output all content of a simulation package.
void CSimulationPackage::Dump(std::shared_ptr<MModel> model) {
    std::string                               path;
    std::list<std::string>                    modules;
    std::list<eElementType>                   contenttypes;
    std::list<tConnector<MElement, MElement>> liblist;
    Crc64                                     crc;
    auto                                      cmodel = std::dynamic_pointer_cast<CModel>(model);

    modules.emplace_back("_simifc");
    modules.emplace_back("generated");
    DumpBase(cmodel);
    path=cmodel->pathstack.back()+"/"+".Makefile";
    cmodel->generatedfiles.push_back(tGenFile {path, id, "##", "mk"});
    OpenStream(makefile, path);
    DumpMakefileHeader(makefile, OutputName+".so");
    //
    //  Get all content that is part of this package
    contenttypes.emplace_back(eElementType::SimObject);
    contenttypes.emplace_back(eElementType::SimEnumeration);
    contenttypes.emplace_back(eElementType::CxxClass);
    contenttypes.emplace_back(eElementType::SimMessageClass);
    contenttypes.emplace_back(eElementType::SimSignalClass);
    contenttypes.emplace_back(eElementType::Struct);
    content = GetAllContent(contenttypes);

    for (auto & ci : content) {
        if ((ci->type == eElementType::SimObject) || (ci->type == eElementType::CxxClass)) {
            modules.push_back(ci->name);
        }
        ci->Dump(model);
    }
    makefile << "PROJ=" << OutputName + ".so\n\n";
    DumpMakefileSource(makefile, modules);
    DumpMakefileObjects(makefile, modules);
    //
    //  Find some CXX-Flags to generate into the Makefile.
    std::string cxxFlags;
    if (HasTaggedValue("CxxFlags")) {
        cxxFlags = GetTaggedValue("CxxFlags");
    }
    if (simversion == 2) {
        makefile << "CXXFLAGS+=-DSIMVERSION=2 -std=" << m_cxxstandard << " -fPIC -Wno-unused-function -O0 $(INCPATH) -g " << cxxFlags << "\n\n";
    } else {
        makefile << "CXXFLAGS+=-std=" << m_cxxstandard << " -fPIC -Wno-unused-function -O0 $(INCPATH) -g " << cxxFlags << "\n\n";
    }
    //
    //  First we add the libs.
    liblist = GetLibraryDependency();

    makefile << "STATICLIBS+=-lsimbase-" << cmodel->AppCoreVersion << " -lsimifc-" << cmodel->AppCoreVersion;
    for (auto& di : liblist) {
        if ((di.getConnector()->HasStereotype("static")) || (di.getConnector()->HasStereotype("StaticLinkage"))) {
            if (di.getElement() != nullptr) {
                if (di.getElement()->type == eElementType::LibraryPackage) {
                    makefile << "\\\n" << "     -l" << (std::dynamic_pointer_cast<CPackageBase>(di.getElement()))->OutputName;
                }
                else if (di.getElement()->type == eElementType::ExternPackage) {
                    makefile << "\\\n" << "     -l" << (std::dynamic_pointer_cast<CPackageBase>(di.getElement()))->OutputName;
                }
            }
        }
    }
    makefile << "\n\n";
    makefile << "DYNLIBS+=";
    for (auto& di : liblist) {
        if (!(di.getConnector()->HasStereotype("static")) && !(di.getConnector()->HasStereotype("StaticLinkage"))) {
            if (di.getElement() != nullptr) {
                if (di.getElement()->type == eElementType::LibraryPackage) {
                    makefile << "\\\n" << "     -l" << (std::dynamic_pointer_cast<CPackageBase>(di.getElement()))->OutputName;
                }
                else if (di.getElement()->type == eElementType::ExternPackage) {
                    makefile << "\\\n" << "     -l" << (std::dynamic_pointer_cast<CPackageBase>(di.getElement()))->OutputName;
                }
            }
        }
    }
    makefile << "\n\n";
    //
    //  Than we add the library pathes.
    makefile << "LIBPATH=";
    for (auto& di : liblist) {
        if (di.getElement() != nullptr) {
            if (di.getElement()->type == eElementType::LibraryPackage) {
                std::string libpath = GetPathToPackage(di.getElement());

                if (!libpath.empty()) {
                    makefile << "\\\n" << "     -L" << libpath;
                }
            }
        }
        else {

        }
    }
    makefile << "\n\n";
    //
    //  Than we add the include pathes.
    makefile << "INCPATH=\\\n";
    //
    // prepend the appcore header pathes.
    if (!cmodel->AppCoreVersion.empty()) {
        makefile << "     -I/usr/local/include/appcore/" << cmodel->AppCoreVersion << "\\\n     -I/usr/include/appcore/" << cmodel->AppCoreVersion;
    }
    for (auto& di : liblist) {
        if (di.getElement() != nullptr) {
            auto target = di.getElement();

            if (target->type == eElementType::LibraryPackage) {
                std::string libpath = GetPathToPackage(target);

                if (!libpath.empty()) {
                    makefile << "\\\n" << "     -I" << libpath;
                }
            } else if (target->type == eElementType::ExternPackage) {
                std::string libpath;

                if (target->HasTaggedValue("directory")) {
                    libpath = target->GetTaggedValue("directory");
                }
                if (!libpath.empty()) {
                    makefile << "\\\n" << "     -I" << libpath;
                }

            }
        } else {

        }
    }
    makefile << "\n\n";

    makefile << "all : $(PROJ)\n\n";
    makefile << "$(PROJ) : $(OBJ)\n";
    makefile << "\tg++ -g -shared $(OBJ) $(LDFLAGS) $(LIBPATH) -Wl,-Bstatic -Wl,--start-group $(STATICLIBS) -Wl,--end-group -Wl,-Bdynamic -Wl,--start-group $(DYNLIBS) -Wl,--end-group -o $@\n\n";
    makefile << "clean :\n";
    makefile << "\trm -f $(PROJ)\n";
    makefile << "\trm -f depend\n";
    makefile << "\trm -f $(OBJ)\n\n";
    makefile << "depend : $(SRC)\n";
    makefile << "\tg++ $(CXXFLAGS) -M $(SRC) > depend\n\n";
    makefile << "-include depend\n\n";
    //
    //
    //  delreq
    path=cmodel->pathstack.back()+"/"+".tMsgDeleteReq.h";
    cmodel->generatedfiles.push_back( tGenFile {path, "tMsgDeleteReq",  "//", "c-inc"});
    OpenStream(delreq, path);
    delreq << "#pragma once\n";
    delreq << "#ifndef TMSGDELETEREQ_INC\n";
    delreq << "#define TMSGDELETEREQ_INC\n\n";

    delreq << "#define IDM_DELETEREQ 0x" << std::hex << std::setw(16) << std::setfill('0') << crc.calc("IDM_DELETEREQ") <<  "\n\n";

    delreq << "//\n";
    delreq << "//               M e s s a g e c l a s s     d e c l a r a t i o n\n";
    delreq << "typedef struct __tMsgDeleteReq {\n";
    delreq << "    uint64_t   id;\n";
    delreq << "    void*      src;\n";
    delreq << "    void*      dst;\n";
    delreq << "    uint64_t   type;\n";
    delreq << "} tMsgDeleteReq;\n\n";

    delreq << "#endif\n";

    //
    //
    //  delreply
    path=cmodel->pathstack.back()+"/"+".tMsgDeleteReply.h";
    cmodel->generatedfiles.push_back(tGenFile { path,"tMsgDeleteReply", "//", "c-inc"} );
    OpenStream(delreply, path);
    delreply << "#pragma once\n";
    delreply << "#ifndef TMSGDELETEREPLY_INC\n";
    delreply << "#define TMSGDELETEREPLY_INC\n\n";

    delreply << "#define IDM_DELETEREPLY 0x" << std::hex << std::setw(16) << std::setfill('0') << crc.calc("IDM_DELETEREPLY") <<  "\n\n";

    delreply << "//\n";
    delreply << "//               M e s s a g e c l a s s     d e c l a r a t i o n\n";
    delreply << "typedef struct __tMsgDeleteReply {\n";
    delreply << "    uint64_t   id;\n";
    delreply << "    void*      src;\n";
    delreply << "    void*      dst;\n";
    delreply << "    uint64_t   type;\n";
    delreply << "} tMsgDeleteReply;\n\n";

    delreply << "#endif\n";
    //
    //
    //  delindication
    path=cmodel->pathstack.back()+"/"+".tSigDeleteIndication.h";
    cmodel->generatedfiles.push_back(tGenFile { path,"tSigDeleteIndication", "//", "c-inc"} );
    OpenStream(delindication, path);
    delindication << "#pragma once\n";
    delindication << "#ifndef TSIGDELETEINDICATION_INC\n";
    delindication << "#define TSIGDELETEINDICATION_INC\n\n";

    delindication << "#define IDS_DELETEINDICATION 0x" << std::hex << std::setw(16) << std::setfill('0') << crc.calc("IDS_DELETEINDICATION") <<  "\n\n";

    delindication << "//\n";
    delindication << "//               M e s s a g e c l a s s     d e c l a r a t i o n\n";
    delindication << "typedef struct __tSigDeleteIndication {\n";
    delindication << "    uint64_t   id;\n";
    delindication << "    void*      src;\n";
    delindication << "    void*      dst;\n";
    delindication << "    uint64_t   type;\n";
    delindication << "} tSigDeleteIndication;\n\n";

    delindication << "#endif\n";
    //
    //
    //  delconfirm
    path=cmodel->pathstack.back()+"/"+".tSigDeleteConfirm.h";
    cmodel->generatedfiles.push_back(tGenFile { path,"tSigDeleteConfirm", "//", "c-inc"} );
    OpenStream(delconfirm, path);
    delconfirm << "#pragma once\n";
    delconfirm << "#ifndef TSIGDELETECONFIRM_INC\n";
    delconfirm << "#define TSIGDELETECONFIRM_INC\n\n";

    delconfirm << "#define IDS_DELETECONFIRM 0x" << std::hex << std::setw(16) << std::setfill('0') << crc.calc("IDS_DELETECONFIRM") <<  "\n\n";

    delconfirm << "//\n";
    delconfirm << "//               M e s s a g e c l a s s     d e c l a r a t i o n\n";
    delconfirm << "typedef struct __tSigDeleteConfirm {\n";
    delconfirm << "    uint64_t   id;\n";
    delconfirm << "    void*      src;\n";
    delconfirm << "    void*      dst;\n";
    delconfirm << "    uint64_t   type;\n";
    delconfirm << "} tSigDeleteConfirm;\n\n";

    delconfirm << "#endif\n";
    //
    //
    //
    //  fill ids.h
    //
    //
    //
    path=cmodel->pathstack.back()+"/.ids.h";
    cmodel->generatedfiles.push_back(tGenFile { path,"ids.h", "//", "c-inc"} );
    OpenStream(ids_h, path);
    ids_h << "// *************************************************************************************************************\n";
    ids_h << "//\n";
    ids_h << "//  Modul-Name     : ids.h\n";
    ids_h << "//\n";
    ids_h << "//  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n";
    ids_h << "//\n";
    ids_h << "// *************************************************************************************************************\n";
    ids_h << "#pragma once\n";
    ids_h << "#ifndef __IDS_INC__\n";
    ids_h << "#define __IDS_INC__\n";

    std::map<std::string, uint64_t>    complete_attr_map;
    std::map<std::string, std::string> complete_macro_map;
    std::map<std::string, uint64_t>    complete_obj_map;
    std::map<std::string, uint64_t>    complete_enum_map;
    std::map<std::string, uint64_t>    sql_obj_name_map;
    std::map<std::string, uint64_t>    sql_attr_name_map;
    size_t                             idmaxlen=0;
    std::string                        oname;

    oname=std::string("IDA_STATE");
    complete_attr_map.insert(std::pair<std::string, uint64_t>(oname, crc.calc(oname)));

    for (auto & ci : content) {
        if (ci->type == eElementType::SimObject) {
            auto s = std::dynamic_pointer_cast<CSimObjectV2>(ci);

            complete_attr_map.insert(s->id_map.begin(), s->id_map.end());
            sql_attr_name_map.insert(s->id_name_map.begin(), s->id_name_map.end());

            complete_macro_map.insert(s->macrolist.begin(), s->macrolist.end());
            //
            //  Do the IDs for objects.
            oname=std::string("IDO_")+helper::toupper(s->basename);

            uint64_t name_crc = crc.calc(oname);
            complete_obj_map.insert(std::pair<std::string, uint64_t>(oname, crc.calc(oname)));
            sql_obj_name_map.insert(std::make_pair(s->basename, name_crc));

        } else if (ci->type == eElementType::SimEnumeration) {
            auto s = std::dynamic_pointer_cast<CSimEnumeration>(ci);

            complete_enum_map.insert(s->id_map.begin(), s->id_map.end());
        }
    }
    for (auto & cmm : complete_macro_map) {
        if (cmm.first.size()>idmaxlen) {
            idmaxlen = cmm.first.size();
        }
    }
    for (auto & cmapi : complete_obj_map) {
        if (cmapi.first.size()>idmaxlen) {
            idmaxlen = cmapi.first.size();
        }
    }
    for (auto & cmapi : complete_attr_map) {
        if (cmapi.first.size()>idmaxlen) {
            idmaxlen = cmapi.first.size();
        }
    }
    for (auto & cmapi : complete_enum_map) {
        if (cmapi.first.size()>idmaxlen) {
            idmaxlen = cmapi.first.size();
        }
    }
    for (auto & cmapi : complete_macro_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_h << "#define " << cmapi.first << filler << cmapi.second << "\n";
    }
    ids_h << "//\n";
    ids_h << "//\n";
    ids_h << "//  This is the list of object ids used to identify object types in the DB\n";
    for (auto & cmapi : complete_obj_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_h << "#define " << cmapi.first << filler << "(0x" << std::hex << std::setw(16) << std::setfill('0') << cmapi.second << ")\n";
    }
    ids_h << "//\n";
    ids_h << "//\n";
    ids_h << "//  This is the list of attribute ids used to identify object attributes in the DB\n";
    ids_h << "#define IDA_OBJ_PARENT (0x1c300f13baa65aa9)\n";
    for (auto & cmapi : complete_attr_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_h << "#define " << cmapi.first << filler << "(0x" << std::hex << std::setw(16) << std::setfill('0') << cmapi.second << ")\n";
    }
    ids_h << "//\n";
    ids_h << "//\n";
    ids_h << "//  This is the list of enumeration ids used to identify object enumerators in the DB\n";
    for (auto & cmapi : complete_enum_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_h << "#define " << cmapi.first << filler << "(0x" << std::hex << std::setw(16) << std::setfill('0') << cmapi.second << ")\n";
    }
    ids_h << "\n#endif  // __IDS_INC__\n\n";

    //
    //
    //
    //  fill ids.sql
    //
    //
    //
    if (cmodel->SQLDirectory.empty()) {
        path=cmodel->pathstack.back()+"/.ids.sql";
    } else {
        path="."+cmodel->Directory+"/"+cmodel->SQLDirectory+"/.ids.sql";
    }
    cmodel->generatedfiles.push_back(tGenFile {path, "ids.sql", "//", "pre-sql"});
    OpenStream(ids_sql, path);
    ids_sql << "-- *************************************************************************************************************\n";
    ids_sql << "--\n";
    ids_sql << "--  Modul-Name     : ids.sql\n";
    ids_sql << "--\n";
    ids_sql << "--  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n";
    ids_sql << "--\n";
    ids_sql << "-- *************************************************************************************************************\n";
    ids_sql << "#ifndef __IDS_INC__\n";
    ids_sql << "#define __IDS_INC__\n";

    ids_sql << "--\n";
    ids_sql << "--\n";
    ids_sql << "--  This is the list of object ids used to identify object types in the DB\n";
    for (auto & cmapi : complete_obj_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_sql << "#define " << cmapi.first << filler << "cast (x'" << std::hex << std::setw(16) << std::setfill('0') << cmapi.second << "' as bigint)\n";
    }
    ids_sql << "--\n";
    ids_sql << "--\n";
    ids_sql << "--  This is the list of attribute ids used to identify object attributes in the DB\n";
    ids_sql << "#define IDA_OBJ_PARENT cast(x'1c300f13baa65aa9' as bigint)\n";
    for (auto & cmapi : complete_attr_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_sql << "#define " << cmapi.first << filler << "cast (x'" << std::hex << std::setw(16) << std::setfill('0') << cmapi.second << "' as bigint)\n";
    }
    ids_sql << "--\n";
    ids_sql << "--\n";
    ids_sql << "--  This is the list of enumeration ids used to identify enumerator values in the DB\n";
    for (auto & cmapi : complete_enum_map) {
        std::string filler;
        filler.assign(idmaxlen-cmapi.first.size()+1, ' ');

        ids_sql << "#define " << cmapi.first << filler << "cast (x'" << std::hex << std::setw(16) << std::setfill('0') << cmapi.second << "' as bigint)\n";
    }
    ids_sql << "\n#endif\n\n";

    //
    //
    //
    //  fill createnamemaps.sql
    //
    //
    //
    if (cmodel->SQLDirectory.empty()) {
        path=cmodel->pathstack.back()+"/.createnamemaps.sql";
    } else {
        path="."+cmodel->Directory+"/"+cmodel->SQLDirectory+"/.createnamemaps.sql";
    }
    cmodel->generatedfiles.push_back(tGenFile {path, "createnamemaps.sql", "//", "pre-sql"});
    OpenStream(crmaps_sql, path);
    crmaps_sql << "-- *************************************************************************************************************\n";
    crmaps_sql << "--\n";
    crmaps_sql << "--  Modul-Name     : createnamemaps.sql\n";
    crmaps_sql << "--\n";
    crmaps_sql << "--  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n";
    crmaps_sql << "--\n";
    crmaps_sql << "-- *************************************************************************************************************\n";
    crmaps_sql <<
                  "--\n"
                  "--\n"
                  "--  Here we recreate the tables.\n"
                  "DROP TABLE  if exists objnamemap;\n"
                  "CREATE TABLE objnamemap\n"
                  "(\n"
                  "    typeid bigint NOT NULL UNIQUE,\n"
                  "    basename text   NOT NULL,\n"
                  "CONSTRAINT \"MapTypeIdIdx\" PRIMARY KEY (typeid)\n"
                  ")\n"
                  "WITH (\n"
                  "    OIDS=FALSE\n"
                  ");\n\n"
                  "DROP TABLE  if exists valuenamemap;\n"
                  "CREATE TABLE valuenamemap\n"
                  "(\n"
                  "    valueid bigint NOT NULL UNIQUE,\n"
                  "    name text   NOT NULL,\n"
                  "CONSTRAINT \"MapVNameIdIdx\" PRIMARY KEY (valueid)\n"
                  ")\n"
                  "WITH (\n"
                  "    OIDS=FALSE\n"
                  ");\n"
                  "--\n"
                  "--\n"
                  "--  Here we fill the obj-name-map table.\n";
    for (auto & onm : sql_obj_name_map) {
        crmaps_sql << "insert into objnamemap values(" << "cast (x'" << std::hex << std::setw(16) << std::setfill('0') << onm.second << "' as bigint), '" << onm.first << "');\n";
    }
    crmaps_sql <<
                  "--\n"
                  "--\n"
                  "--  Here we fill the value-name-map table.\n";
    crmaps_sql << "insert into valuenamemap values(cast (x'1c300f13baa65aa9' as bigint), '__parent');\n";
    for (auto & vnm : sql_attr_name_map) {
        crmaps_sql << "insert into valuenamemap values(" << "cast (x'" << std::hex << std::setw(16) << std::setfill('0') << vnm.second << "' as bigint), '" << vnm.first << "');\n";
    }
    crmaps_sql << "\n\n";




    path = cmodel->pathstack.back()+"/.generated.h";
    cmodel->generatedfiles.push_back(tGenFile {path, "-generated-inc", "//", "cxx"});
    OpenStream(generated, path);

    generated << "// *************************************************************************************************************\n";
    generated << "//\n";
    generated << "//  Modul-Name     : generated.h\n";
    generated << "//\n";
    generated << "//  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n";
    generated << "//\n";
    generated << "// *************************************************************************************************************\n";
    generated << "#pragma once\n";
    generated << "#ifndef __GENERATED_INC__\n";
    generated << "#define __GENERATED_INC__\n\n"
                 "#include <stdint.h>\n"
                 "#include <simobj.h>\n"
                 "#include <pointerindex.h>\n"
                 "#include <logger.h>\n"
                 "#include <objectref.h>\n\n"
                 "extern CLogger&       simlog();\n\n"
                 "//\n"
                 "//\n"
                 "tObjectRef NewObjectFromTemplate(uint64_t tid, uint64_t templ, tSimObj* parent = 0);\n"
                 "tObjectRef NewObjectFromTemplate(uint64_t tid, uint64_t templ, const tObjectRef& parent);\n"
                 "//\n"
                 "//  This creates a new object of the given type and returns the pointer\n"
                 "//  to it. The object gets initialized according to the model.\n"
                 "tObjectRef NewObject(uint64_t tid, tSimObj* parent = 0);\n"
                 "tObjectRef NewObject(uint64_t tid, const tObjectRef& parent);\n\n"

                 "bool IsTypeOf(tSimObj* obj, uint64_t tid);\n"
                 "bool IsTypeOf(const tObjectRef& obj, uint64_t tid);\n\n"
                 ;

    generated << "#endif // __GENERATED_INC__\n";

    generated.close();

    path = cmodel->pathstack.back()+"/.generated.cpp";
    cmodel->generatedfiles.push_back(tGenFile {path, "-generated-src", "//", "cxx"});
    OpenStream(generated, path);

    generated << "#include \"generated.h\"\n"
                 "#include <simobjfactory.h>\n\n";
    generated <<
                 "CLogger&       simlog() {\n"
                 "    static CLogger _simlog(ModulId::SimulationLib, \"" << SimulationName << "\");\n"
                 "\n"
                 "    return _simlog;\n"
                 "}\n\n";
    generated <<    "tObjectRef NewObjectFromTemplate(uint64_t tid, uint64_t templ, tSimObj *parent) {\n"
                    "    tObjectRef retval;\n"
                    "    tSimObj*   newobj;\n"
                    "\n"
                    "    uint64_t newobjid = tObjectRef::Reserve();\n"
                    "    newobj = factory_createfromtemplate(tid, templ, newobjid);\n"
                    "    if (newobj != nullptr) {\n"
                    "        if (parent != nullptr) {\n"
                    "            newobj->SetParent(parent->objid);\n"
                    "        }\n"
                    "        newobj->objid  = INVALID_OBJID;\n"
                    "        retval = tObjectRef::Add(newobj);\n"
                    "    } else {\n"
                    "        tObjectRef::Release(newobjid);\n"
                    "    }\n"
                    " \n"
                    "     return retval;\n"
                    " }\n"
                    " \n"

                    "tObjectRef NewObject(uint64_t tid, tSimObj *parent) {\n"
                    "    tObjectRef retval;\n"
                    "    tSimObj*   newobj;\n"
                    " \n"
                    "    uint64_t newobjid = tObjectRef::Reserve();\n"
                    "    newobj = factory_createnewobj(tid, newobjid);\n"
                    "    if (newobj != nullptr) {\n"
                    "        if (parent != nullptr) {\n"
                    "            newobj->SetParent(parent->objid);\n"
                    "        }\n"
                    "        retval = tObjectRef::Add(newobj);\n"
                    "    } else {\n"
                    "        tObjectRef::Release(newobjid);\n"
                    "    }\n"
                    " \n"
                    "     return retval;\n"
                    "}\n\n"

                    "bool IsTypeOf(tSimObj* obj, uint64_t tid) {\n"
                    "    bool retval = false;\n\n"
                    "    if ((obj != nullptr) && (tid == obj->type)) {\n"
                    "        retval = true;\n"
                    "    }\n"
                    "    return retval;\n"
                    "}\n\n"

                    "bool IsTypeOf(const tObjectRef& obj, uint64_t tid) {\n"
                    "    bool retval = false;\n"
                    "\n"
                    "    if (obj.valid() && (tid == obj->type)) {\n"
                    "        retval = true;\n"
                    "    }\n"
                    "    return retval;\n"
                    "}\n"
    ;
    //
    //
    //
    //  fill ids.h
    //
    //
    //
    path=cmodel->pathstack.back()+"/._simifc.cpp";
    cmodel->generatedfiles.push_back(tGenFile {path, "-simifc", "//", "cxx"});
    OpenStream(ifc, path);
    ifc << "// *************************************************************************************************************\n";
    ifc << "//\n";
    ifc << "//  Modul-Name     : _simifc.cpp\n";
    ifc << "//\n";
    ifc << "//  Copyrights by Hans-Juergen Lange <hjl@simulated-universe.de>. All rights reserved.\n";
    ifc << "//\n";
    ifc << "// *************************************************************************************************************\n";

    std::set<std::string>  doneinclude;

    DumpExtraIncludes(ifc, doneinclude);
    ifc << "//\n"
           "//  more headers we generate\n";

    ifc << "#include \"generated.h\"\n";
    ifc << "#include \"ids.h\"\n";
    ifc << "#include \"tMsgDeleteReq.h\"\n";
    ifc << "#include \"tMsgDeleteReply.h\"\n";
    ifc << "#include \"tSigDeleteIndication.h\"\n";
    ifc << "#include \"tSigDeleteConfirm.h\"\n";
    ifc << "#include <tSignalStartCycle.h>\n";
    ifc << "#include <tSignalEndCycle.h>\n";
    ifc << "#include <tMsgInvalidReply.h>\n";
    ifc << "#include <tSigInvalidIndication.h>\n";

    std::set<std::shared_ptr<MElement>>    doneclass;

    ifc << "//\n";
    ifc << "//                       M o d e l   i n c l u d e s\n";
    //
    //  Running along all MClasses. That may be a lot different things. But we are only interested in the
    //  messages and signals.
    for (auto & mm : MClass::Instances) {
        //
        //  Check for Simulation message implementations.
        if (mm.second->type == eElementType::SimMessageClass) {
            //
            //  Convenient temporary
            auto s = std::dynamic_pointer_cast<CMessageClass>(mm.second);
            //
            //  Check if the message class has a basename.
            if (!s->basename.empty()) {
                //
                //  Collect all attributges and assocication ends so that the following functions are getting all needed information.
                s->CollectAttributes();
                s->CollectNeededModelHeader(s, s->neededmodelheader);
                //
                //  Run across the needed headers. As the prepare step is already done
                //  it is a complete list of all needed headers.
                for (auto &inc: s->GetNeededHeader()) {
                    std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(ifc, mIfcClass, doneinclude,
                                                                                   doneclass);
                }
            }
        }
        if (mm.second->type == eElementType::SimSignalClass) {
            auto s = std::dynamic_pointer_cast<CSignalClass>(mm.second);

            if (!s->basename.empty()) {
                s->CollectNeededModelHeader(s, s->neededmodelheader);

                for (auto & inc : s->GetNeededHeader()) {
                    std::dynamic_pointer_cast<CClassBase>(inc)->DumpNeededIncludes(ifc, mIfcClass, doneinclude, doneclass);
                }
            }
        }
    }

    ifc << "\n";
    for (auto & ci : content) {
        if (ci->type == eElementType::SimObject) {
            auto s = std::dynamic_pointer_cast<CSimObjectV2>(ci);

            s->DumpNeededIncludes(ifc, s, doneinclude, doneclass);
        }
    }
    std::set<std::string> done;
    ifc << "class CGeneratedSimIfc : public CSimIfc {\n"
           "public:\n"
           "    CGeneratedSimIfc() {\n"
           "        mainviewport = nullptr;\n";
   done.clear();

   ifc << "\n";
   for (auto & ci : content) {
       if (ci->type == eElementType::SimObject) {
           auto s = std::dynamic_pointer_cast<CSimObjectV2>(ci);

           ifc << "        addsimobjfactory(&" << s->lower_basename << "_factory);\n";
       }
   }


    ifc << "}\n"
           "    virtual ~CGeneratedSimIfc() {}\n"
           "    int         CreateObject(uint64_t oid, uint64_t tid, uint64_t droptime = UINT64_MAX) override;\n"
           "    void        SetValue(uint64_t oid, uint64_t vid, uint64_t vidx, const tVariant& value) override;\n"
           "    void        StartCycle() override;\n"
           "    void        EndCycle() override;\n"
           "    tMsg*       Process(std::shared_ptr<tMsg> aMsg) override;\n"
           "    tMsg*       Process(tCommTarget& src, tJSON* aJson) override;\n"
           "    tNetPack*   Process(tNetPack* aPackage) override;\n"
           "    tNetPack*   ProcessRaw(tNetPack* aPackage) override;\n"
           "    std::string GetMessageName(uint64_t aId) override;\n"
           "    tMsg*       GetMessage(tJSON* aJson) override;\n"
           "public:\n"
           "    tSimObj*                                mainviewport = nullptr;\n"
           "    static std::map<uint64_t, std::string>  gIdToString;\n"
           "private:\n"

           "    std::map<templateid_t, tSimObj*>        templatemap;\n"
           "};\n\n"
    "std::map<uint64_t, std::string>  CGeneratedSimIfc::gIdToString = {";
    char c =' ';
    for (auto & mm : MClass::Instances) {
        if (mm.second->type == eElementType::SimMessageClass) {
            auto s = std::dynamic_pointer_cast<CMessageClass>(mm.second);
            if (!s->basename.empty()) {
    ifc << c << "\n                                                                  {IDM_" << helper::toupper(s->basename) << ", \"" << s->basename << "\"}";
    c =',';
            }
        }
    }
    for (auto & mm : MClass::Instances) {
        if (mm.second->type == eElementType::SimSignalClass) {
            auto s = std::dynamic_pointer_cast<CSignalClass>(mm.second);

            if (!s->basename.empty()) {
    ifc << c << "\n                                                                  {IDS_" << helper::toupper(s->basename) << ", \"" << s->basename << "\"}";
            }
        }
    }
    ifc << "\n                                                                 };\n\n"
    "int CGeneratedSimIfc::CreateObject(uint64_t oid, uint64_t tid, uint64_t droptime) {\n"
        "    //\n"
        "    //  If we have no special droptime set its not an on-demand loading object\n"
        "    if (droptime == UINT64_MAX) {\n"
        "        /*\n"
        "         * We are passing the oid with the creation function into the object.\n"
        "         * This way we can filter the templates and store them into the object module\n"
        "         */\n"
        "        tSimObj* newobj = factory_createobj(tid, oid, droptime);\n"
        "        /*\n"
        "         * If the object could be created from the type id the loading\n"
        "         * can proceed.\n"
        "         */\n"
        "        if (newobj != nullptr) {\n"
        "            /*\n"
        "             * The top two bits are used for templates and other stuff only.\n"
        "             * They must be created but not added to the simulation loop or the\n"
        "             * idmap.\n"
        "             */\n"
        "            if ((oid&0xc0000000) == 0) {\n"
        "                if (oid == 0) {\n"
        "                    mainviewport = newobj;\n"
        "                }\n"
        "                StartObject(tObjectRef::Add(newobj));\n"
        "            } else {\n"
        "                templatemap.insert(std::pair<objectid_t, tSimObj*>(oid, newobj));\n"
        "            }\n"
        "        }\n"
        "    } else {\n"
        "        tObjectRef::Reserve(oid);\n"
        "    }\n"
        "    return (1);\n"

    "}\n"

    "void      CGeneratedSimIfc::SetValue(uint64_t oid, uint64_t vid, uint64_t vidx, const tVariant& value) {\n"
           "    tObjectRef obj = tObjectRef(oid);\n"
           "    obj->InitMember(vid, vidx, value);\n"
    "}\n"
    "void      CGeneratedSimIfc::StartCycle() {\n"
           "    tSignalStartCycle* s = new tSignalStartCycle;\n\n"
           "    s->Cycle = GetCycle();\n"
           "    if (mainviewport != nullptr) {\n"
               "        mainviewport->Process(std::shared_ptr<tSig>(s));\n"
           "    }\n"

    "}\n"
    "void      CGeneratedSimIfc::EndCycle() {\n"
           "    tSignalEndCycle* s = new tSignalEndCycle;\n\n"

           "    s->Cycle = GetCycle();\n"
           "    if (mainviewport != nullptr) {\n"
           "        mainviewport->Process(std::shared_ptr<tMsg>(s));\n"
           "    }\n"
    "}\n"
    "tMsg*     CGeneratedSimIfc::Process(std::shared_ptr<tMsg> aMsg) {\n"
           "    tMsg* retval = nullptr;\n"
           "    auto ref = std::get<tReference>(aMsg->dst);\n"
           "    tObjectRef obj(ref.m_id, (tSimObj*)ref.m_ptr);\n\n"
           "    retval = obj->Process(aMsg);\n"
           "    return retval;\n"
    "}\n"
    "tMsg* CGeneratedSimIfc::Process(tCommTarget& src, tJSON* aJson) {\n"
           "    tMsg* retval = nullptr;\n\n"
           "    if (aJson != nullptr) {\n"
           "        retval = GetMessage(aJson);\n"
           "        if (retval != nullptr) {\n"
           "            auto ref = std::get<tReference>(retval->dst);\n"
           "            retval->src = src;\n"

           "            tObjectRef obj(ref.m_id, (tSimObj*)ref.m_ptr);\n\n"
           "            retval = obj->Process(std::shared_ptr<tMsg>(retval));\n"
           "        } else {\n"
           "            tJSON* j   = find(aJson, \"MsgId\");\n\n"
           "            if (j != nullptr) {\n"
           "                std::string       id      = helper::tolower( ((tJSONValue*)(j))->value );\n"
           "                tMsgInvalidReply* invalid = new tMsgInvalidReply;\n\n"
           "                invalid->dst = src;\n"
           "                invalid->MsgName = id;\n"
           "                retval = invalid;\n"
           "            } else {\n"
           "                j = find(aJson, \"SignalId\");\n"
           "                if (j != nullptr) {\n"
           "                    std::string            id      = helper::tolower( ((tJSONValue*)(j))->value );\n"
           "                    tSigInvalidIndication* invalid = new tSigInvalidIndication;\n\n"
           "                    invalid->dst = src;\n"
           "                    invalid->SignalName = id;\n"
           "                    SendSig(std::shared_ptr<tSig>(invalid));\n"
           "                }\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "    return retval;\n"
           "}\n\n"
           "tNetPack* CGeneratedSimIfc::Process(tNetPack* aPackage) {\n"
           "    return 0;\n"
           "}\n\n"
           "tNetPack* CGeneratedSimIfc::ProcessRaw(tNetPack* aPackage) {\n"
           "    return 0;\n"
           "}\n\n"
           "std::string CGeneratedSimIfc::GetMessageName(uint64_t aId) {\n"
           "    auto i = CGeneratedSimIfc::gIdToString.find(aId);\n"
           "    if (i != CGeneratedSimIfc::gIdToString.end()) {\n"
           "        return i->second;\n"
           "    }\n"
           "    return \"\";\n"
           "}\n\n"
           "tMsg* CGeneratedSimIfc::GetMessage(tJSON* aJson) {\n"
                  "    tMsg* retval = nullptr;\n"
                  "    if (aJson != nullptr) {\n"
                  "         tJSON* j   = find(aJson, \"MsgId\");\n\n"
                  "         if (j != nullptr) {\n"
                  "             tMsg*       msg = nullptr;\n"
                  "             std::string id  = helper::tolower( ((tJSONValue*)(j))->value );\n\n             ";
           for (auto & mm : MClass::Instances) {
               if (mm.second->type == eElementType::SimMessageClass) {
                   auto s = std::dynamic_pointer_cast<CMessageClass>(mm.second);

                   if (!s->basename.empty()) {
           ifc << "if (id == \"" << helper::tolower(s->basename) << "\") {\n";
           ifc << "                 msg = new " << s->name << "(aJson);\n";
           ifc << "             } else ";
                   }
               }
           }
           ifc << "{\n";
           ifc << "             }\n"
                  "             if (msg != nullptr) {\n"
                  "                 retval = msg;\n"
                  "             }\n";
           ifc << "         } else {\n"
                  "             j = find(aJson, \"SignalId\");\n"
                  "             if (j != nullptr) {\n"
                  "                 tSig*       sig = nullptr;\n"
                  "                 std::string id = helper::tolower( ((tJSONValue*)(j))->value );\n\n                 ";
           for (auto & mm : MClass::Instances) {
               if (mm.second->type == eElementType::SimSignalClass) {
                   auto s = std::dynamic_pointer_cast<CSignalClass>(mm.second);

                   if (!s->basename.empty()) {
           ifc << "if (id == \"" << helper::tolower(s->basename) << "\") {\n";
           ifc << "                     sig = new " << s->name << "(aJson);\n";
           ifc << "                 } else ";
                   }
               }
           }
           ifc << "{\n";
           ifc << "                 }\n"
                  "                 if (sig != nullptr) {\n"
                  "                     retval = sig;\n"
                  "                 }\n"
                  "             }\n"
                  "         }\n"
                  "    }\n"
                  "    return retval;\n"
                  "}\n\n"


           ;


    ifc << "CGeneratedSimIfc simifc;\n";
    done.clear();
    ifc << "// **************************************************************************\n";
    ifc << "//\n";
    ifc << "//  Method-Name   : siminit()\n";
    ifc << "//\n";
    ifc << "//  Generated source code.\n";
    ifc << "//\n";
    ifc << "// **************************************************************************\n";
    ifc << "CSimIfc* siminit(void) {\n";
    //
    //  Add the main viewport if it is marked.
    for (auto & ci : content) {
        if (ci->type == eElementType::SimObject) {
            bool mvp = false;
            std::string sname;
            std::string sbase;

            auto s = std::dynamic_pointer_cast<CSimObjectV2>(ci);

            mvp   = s->MainViewPort;
            sname = s->name;
            sbase = s->upper_basename;
            if (mvp) {
                ifc << "    //\n";
                ifc << "    //  Create the main viewport object " << sname << "\n";
                ifc << "    simifc.CreateObject(0ul, IDO_"<< sbase << ");\n";
                //
                //  Only one object can be a main viewport.
                break;
            }
        }
    }
    ifc << "    if (simifc.mainviewport != nullptr) {\n"
           "        simifc.mainviewport->update(0u);\n"
           "    }\n";

    ifc << "    return &simifc;\n";

    ifc << "}\n";
    ifc <<
            "//\n"
            "//  This is the function called after loading the model library\n"
            "extern \"C\" {\n\n"

            "CSimIfc* createsimlibifc(CCoreIfc* aCore) {\n"
            "    //\n"
            "    //  We do save the pointer to the core interface\n"
            "    __coreIfc=aCore;\n"
            "    //\n"
            "    //  This call gets into the implementation of the simulation.\n"
            "    //  If this function is not available at link-time no simulation will be produced.\n"
            "    //  It returns a new CSimIfc based object.\n"
            "    return (siminit());\n"
            "}\n"
            "}\n\n";
    //
    //  Close all open files.
    generated.close();
    ifc.close();
    ids_h.close();
    ids_sql.close();
    makefile.close();
    delreq.close();
    delreply.close();
    delindication.close();
    delconfirm.close();
    crmaps_sql.close();
    //
    //  Remove last element from path-stack
    cmodel->pathstack.pop_back();
}
