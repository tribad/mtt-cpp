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

#ifdef WIN32
#include <direct.h>
#endif

#include <iostream>
#include <fstream>
#include "helper.h"
#include "path.h"
#include "main.h"
#include "mgeneralization.h"
#include "cgeneralization.h"
#include "massociation.h"
#include "cassociation.h"
#include "mdependency.h"
#include "cdependency.h"
#include "mstatemachine.h"
#include "mmessage.h"
#include "mlifeline.h"
#include "musecase.h"
#include "cmessage.h"
#include "mparameter.h"
#include "cparameter.h"
#include "mattribute.h"
#include "cattribute.h"
#include "cpackage.h"
#include "cmodel.h"

#include "cclassbase.h"

CModel::CModel()
{

}

void CModel::Prepare(void) {
    Directory="/.";

    for ( auto & i : TaggedValues) {
        if (helper::tolower(i.first)=="directory") {
            Directory=i.second;
            if (Directory.front()!='/') {
                Directory.insert(0, "/");
            }
        } else if (helper::tolower(i.first) == "license") {
            License = i.second;
        } else if (helper::tolower(i.first) == "licensefile") {
            LicenseFile = i.second;
        } else if (helper::tolower(i.first) == "copyright") {
            Copyright = i.second;
        } else if (helper::tolower(i.first) == "appcoreversion") {
            AppCoreVersion = i.second;
        } else if (helper::tolower(i.first) == "sqldirectory") {
            SQLDirectory = "/"+i.second+"/";
        } else if (helper::tolower(i.first) == "testenvironment") {
            TestEnvironment = i.second;
            if (TestEnvironment.empty()) {
                TestEnvironment = "gtest";
            }
        }
    }
    //
    //  Prepare statemachines
    for (auto & msi : MStatemachine::Instances) {
        msi.second->Prepare();
    }
    //
    //  Prepare the lifelines.
    for (auto & li : MLifeLine::Instances) {
        li.second->Prepare();
    }
    //
    //  Prepare the usecases
    for (auto & ui : MUseCase::Instances) {
        ui.second->Prepare();
    }
    //
    //  Prepare the association ends.
    for (auto & ai : MAssociationEnd::Instances) {
        ai.second->Prepare();
    }
    //
    //  Prepare the dependency with no parent.
    for (auto & di : mDependency) {
        if (di != nullptr) {
            di->Prepare();
        }
    }
    //
    //  Prepare Attributes.
    for ( auto & i : MAttribute::Instances) {
        i.second->Prepare();
    }

    //
    //  Prepare Parameter
    for (auto & i : MParameter::Instances) {
        i.second->Prepare();
    }
    //
    //  First do the preparation and then
    //  we put each generalization into the class that is derived.
    for (auto & g : MGeneralization::Instances) {
        g.second->Prepare();

        if (g.second->derived && (g.second->derived->IsClassBased())) {
            std::dynamic_pointer_cast<CClassBase>(*g.second->derived)->Generalization.emplace_back(g.second->id);
        }
    }
    for (auto & a : MAssociation::Instances) {
        auto  assoc = std::dynamic_pointer_cast<CAssociation>(a.second);
        assoc->Prepare();

        if (assoc->ends[0] && (std::dynamic_pointer_cast<MAssociationEnd>(*assoc->ends[0])->Classifier) && (std::dynamic_pointer_cast<MAssociationEnd>(*assoc->ends[0])->Classifier->IsClassBased())) {
            auto c = std::dynamic_pointer_cast<CClassBase>(*std::dynamic_pointer_cast<MAssociationEnd>(*assoc->ends[0])->Classifier);

            c->Add(assoc);
        }
    }

    for (auto & d : MDependency::Instances) {
        d.second->Prepare();

        if (d.second->src && (d.second->src->IsClassBased())) {
            auto c = std::dynamic_pointer_cast<CClassBase>(*d.second->src);

            c->Dependency.emplace_back(d.second->id);
        }
    }
    //
    //  Prepare the messages. They may create lots of internal structures.
    //  We prepare the messages right before the packages and classes starts with their
    //  preparation. This way we can add the messages as message ends into the class.

    for (auto & mi : MMessage::Instances) {
        mi.second->Prepare();
    }

    for (auto & lp : Packages) {
        lp->Prepare();
    }
    for (auto & u : mUnowned) {
        u->Prepare();
    }
}

void CModel::Dump(void) {
    int                            err=0;
    struct stat                    dirstat;
    std::string                    path;
    std::list<MPackage*>::iterator p;
    //
    //  First check for directory and create it if needed.
    path="."+Directory;
    err=stat(path.c_str(), &dirstat);
    if ((err == -1) && (errno==ENOENT)) {
#ifdef __MINGW32__
        mkdir(path.c_str());
#elif WIN32
		_mkdir(path.c_str());
#else
        mkdir(path.c_str(), 0777);
#endif
    } else if ((err == 0) && (!(S_IFDIR & dirstat.st_mode))) {
        std::cerr << "Not a directory " << path << "\n";
        return;
    }
    //
    //  prepare dump by id.
    if (!gDumpId.empty()) {
        auto todump = MElementRef(gDumpId);

        while (todump) {
            if (todump->IsPackageBased()) {
                gDumpList.insert(todump->id);
            }
            todump = todump->parent;
        }
    }

    //
    //  At this point we have a directory to create the generation
    //  results into.
    pathstack.push_back(path);
    for (auto p : Packages) {
        p->Dump(shared_from_this());
    }
    Merge();
}

void CModel::LoadLastGeneratedFiles() {
    if (!pathstack.empty()) {
        char linebuffer[1024];
        std::string   fname = pathstack.front()+"/generatedfiles";
        std::ifstream infiles(fname);

        while ((infiles.good()) && (!infiles.eof())) {
            infiles.getline(linebuffer, sizeof(linebuffer));
            tGenFile igenfile;
            int field = 0;
            auto token = helper::tokenize(linebuffer, ";");

            if (!token.empty()) {
                for (auto & t : token) {
                    switch (field) {
                    case 0:
                        igenfile.ofile = t;
                        break;
                    case 1: 
                        igenfile.id = t;
                        break;
                    case 2:
                        igenfile.filetype = t;
                        break;
                    case 3:
                        igenfile.comment = t;
                        break;
                    default:
                        break;
                    }
                    field++;
                }
            
                //std::cerr << igenfile.ofile << "::" << igenfile.id << "::" << igenfile.filetype << "::" << igenfile.comment << ":\n";
                if (infiles.good()) {
                    lastgeneratedfiles.insert(std::pair<std::string, tGenFile>(igenfile.id+igenfile.filetype, igenfile));
                }
            }
        }
    }
}

void CModel::DumpGeneratedFiles() {
    if (!pathstack.empty()) {
        std::string   fname = pathstack.front()+"/generatedfiles";
        std::ofstream outfiles(fname);

        if (outfiles.good()) {
            for (auto & f : generatedfiles) {
                CPath p(f.ofile);
                outfiles << (std::string)(p) << ";" << f.id << ";" << f.filetype << ";" << f.comment << std::endl;
            }
        }
    }
}

void CModel::Merge(void) {
    std::list< tGenFile >::iterator files;

    LoadLastGeneratedFiles();

    for (files=generatedfiles.begin(); files!=generatedfiles.end(); ++files) {
        size_t      basepos;
        std::string gfile=files->ofile;
        std::string ofile;
        std::string lfile;
        //
        //  basepos is the position right before the dot in the generated file name.
        basepos=gfile.find_last_of('/');
        //
        //  Now get the front until the slash
        ofile = gfile.substr(0, basepos+1);
        //
        //  and skip the dot to create the final target file-path.
        ofile += gfile.substr(basepos+2);
        //
        //  Search the file in the last generated file list.
        auto li = lastgeneratedfiles.find(files->id+files->filetype);

        if (li != lastgeneratedfiles.end()) {
            //
            //  basepos is the position right before the dot in the last generated file name.
            basepos = li->second.ofile.find_last_of('/');
            //
            //  Now get the front until the slash
            lfile = li->second.ofile.substr(0, basepos+1);
            //
            //  and skip the dot to create the final target file-path.
            lfile += li->second.ofile.substr(basepos+2);
        } else {
            lfile = ofile;
        }
        if (files->filetype == "mSysHeader") {
            MergeSysHeader(gfile, ofile, files->comment, files->id);
        } else {
            Merge(gfile, ofile, lfile, files->comment);
        }
    }
    DumpGeneratedFiles();
}
//
//  gname   - name of the generated file. prefixed with a dot.
//  oname   - name of the file to output. removed the dot from gname.
//  lname   - name of the file in the last generation without the dot.
//  comment - the comment style
void CModel::Merge(const std::string& gname, const std::string&oname, const std::string& lname, const std::string& comment) {
    int         state=0;          // State variable for a little statemachine.
    size_t      tagpos;           // Where the tag starts.
    char        linebuffer[16384];// Should be large enough to get almost anything read in.
    std::string line;             // Line buffer for readin.
    std::string search;           // Search string.
    std::string mtag;             // This is the tag used to find the modifications.
    std::list<std::string> glist; // List of lines from generated file without modifiable parts.
    std::list<std::string> gtags; // List of tags in generated file.
    std::list<std::string> olist; // List of lines from original file without modifiable parts.
    std::list<std::string> mlist; // List of modified lines from original file.
    std::map<std::string, std::list <std::string> > mods; // Mods by tag

    std::ifstream gfile(gname);   // Generated file input
    std::ifstream ofile(lname);   // Original file input
    std::ofstream nfile;          // New file output.

    search=comment+" User-Defined-Code:";
    //
    //  Create all lists from the generated file.
    while (gfile.good()) {
        gfile.getline(linebuffer, sizeof(linebuffer)-1);
        if (gfile.good()) {
            line=linebuffer;

            tagpos=line.find(search);
            switch (state) {
            case 0:
                //
                //  Check if we have a tag line.
                if (tagpos != std::string::npos) {
                    gtags.push_back(line);
                    search=comment+" End-Of-UDC:";
                    state=1;
                    glist.push_back(line);
                } else {
                    glist.push_back(line);
                }
                break;
            case 1:
                //
                //  Check if we have a end-tag line.
                if (tagpos != std::string::npos) {
                    search=comment+" User-Defined-Code:";
                    glist.push_back(line);
                    state=0;
                } else {
                }
                break;
            default:
                break;
            }
        }
    }
    gfile.close();
    //
    //  Create all lists from the modified file.
    search = comment + " User-Defined-Code:";

    while (ofile.good()) {
        ofile.getline(linebuffer, sizeof(linebuffer)-1);
        if (ofile.good()) {
            line=linebuffer;

            tagpos=line.find(search);
            switch (state) {
            case 0:
                //
                //  Check if we have a tag line.
                if (tagpos != std::string::npos) {
                    //
                    //  we need the tag later.
                    mtag=line;
                    olist.push_back(line);
                    search=comment+" End-Of-UDC:";
                    state=1;
                } else {
                    olist.push_back(line);
                }
                break;
            case 1:
                //
                //  Check if we have a end-tag line.
                if (tagpos != std::string::npos) {
                    search=comment+" User-Defined-Code:";
                    olist.push_back(line);
                    mods.insert(std::pair<std::string, std::list<std::string> >(mtag, mlist));
                    mlist.clear();
                    state=0;
                } else {
                    mlist.push_back(line);
                }
                break;
            default:
                break;
            }
        }
    }
    ofile.close();
    //
    // Now we must check whether we have any diffs between the new file and the original file.
    auto gi = glist.begin();
    auto oi = olist.begin();

    while ((gi != glist.end()) && (oi!=olist.end())) {
        if ((*gi) != (*oi)) {
            break;
        }
        gi++;
        oi++;
    }
    //
    // Check if both run to the end. If so they are equal and nothing has to be done.
    if ((gi != glist.end()) || (oi!=olist.end())) {
        int         err=0;
        struct stat dirstat;
        std::string newname;
        //
        //  First check for an existing file and create a backup.
        err=stat(oname.c_str(), &dirstat);
        if (err == 0) {
            newname=oname+".bak";
            err=stat(newname.c_str(), &dirstat);
            if (err == 0) {
                remove(newname.c_str());
            }
            rename(oname.c_str(), newname.c_str());
        }
        nfile.open(oname);
        //
        //  Generate new file but therefor we must insert the modifications.
        //  So we search the tags in the glist.
        state = 0;
        search=comment+" User-Defined-Code:";
        gi=glist.begin();
        while (gi != glist.end()) {
            tagpos=(*gi).find(search);

            nfile << (*gi) << "\n";

            switch (state) {
            case 0:
                //
                //  Check if we have a tag line.
                if (tagpos != std::string::npos) {
                    auto mi = mods.find((*gi));

                    if (mi != mods.end()) {
                        for (oi=mi->second.begin();oi != mi->second.end(); ++oi) {
                            nfile << (*oi) << "\n";
                        }
                        mods.erase(mi);
                    }
                    //
                    //  we need the tag later.
                    search=comment+" End-Of-UDC:";
                    state=1;
                } else {
                }
                break;
            case 1:
                //
                //  Check if we have a end-tag line.
                if (tagpos != std::string::npos) {
                    search=comment+" User-Defined-Code:";
                    state=0;
                } else {
                }
                break;
            default:
                break;
            }
            gi++;
        }
        auto mi = mods.begin();
        //
        //  Check if any modification has been left over.
        if (mi != mods.end()) {
            nfile << comment << "\n";
            nfile << comment << "\n";
            nfile << comment << " this is a collection of left-over modifications.\n";
        }
        for (;mi!=mods.end(); ++mi) {
            nfile << comment << " " << mi->first << "\n";
            for (oi=mi->second.begin();oi != mi->second.end(); ++oi) {
                nfile << comment << " " << (*oi) << "\n";
            }
            nfile << comment << " end-of-" << mi->first << "\n";
        }
        nfile.close();
    } else {
    }

}

//
//  gname   - name of the generated file. prefixed with a dot.
//  oname   - name of the file to output. removed the dot from gname.
//  lname   - name of the file in the last generation without the dot.
//  comment - the comment style
//  a_id    - the class id to get the hdr file to use as code-input
void CModel::MergeSysHeader(const std::string& gname, const std::string&oname, const std::string& comment, const std::string& a_id) {
    int         state=0;          // State variable for a little statemachine.
    size_t      tagpos;           // Where the tag starts.
    char        linebuffer[16384];// Should be large enough to get almost anything read in.
    std::string line;             // Line buffer for readin.
    std::string search;           // Search string.
    std::string mtag;             // This is the tag used to find the modifications.
    std::list<std::string> glist; // List of lines from generated file without modifiable parts.
    std::list<std::string> gtags; // List of tags in generated file.
    std::list<std::string> olist; // List of lines from original file without modifiable parts.
    std::list<std::string> mlist; // List of modified lines from original file.
    std::map<std::string, std::list <std::string> > mods; // Mods by tag

    std::ifstream gfile(gname);   // Generated file input
    std::ofstream nfile;          // New file output.
    //
    //  get the complete path to the header file from the id.
    auto found = lastgeneratedfiles.find(a_id + "hdr");

    if (found == lastgeneratedfiles.end()) {
        return;
    }
    CPath lname(found->second.ofile);
    //
    //  using lname as a temporary to remove the dot at the beginning of the basename.
    lname.SetBase(lname.Base().erase(0, 1));

    std::ifstream ofile(lname);   // Original/Last generated file input

    search=comment+" User-Defined-Code:";
    //
    //  Create all lists from the generated file.
    while (gfile.good()) {
        gfile.getline(linebuffer, sizeof(linebuffer)-1);
        if (gfile.good()) {
            line=linebuffer;

            tagpos=line.find(search);
            switch (state) {
            case 0:
                //
                //  Check if we have a tag line.
                if (tagpos != std::string::npos) {
                    gtags.push_back(line);
                    search=comment+" End-Of-UDC:";
                    state=1;
                    glist.push_back(line);
                } else {
                    glist.push_back(line);
                }
                break;
            case 1:
                //
                //  Check if we have a end-tag line.
                if (tagpos != std::string::npos) {
                    search=comment+" User-Defined-Code:";
                    glist.push_back(line);
                    state=0;
                } else {
                }
                break;
            default:
                break;
            }
        }
    }
    gfile.close();
    //
    //  Create all lists from the modified file.
    search = comment + " User-Defined-Code:";

    while (ofile.good()) {
        ofile.getline(linebuffer, sizeof(linebuffer)-1);
        if (ofile.good()) {
            line=linebuffer;

            tagpos=line.find(search);
            switch (state) {
            case 0:
                //
                //  Check if we have a tag line.
                if (tagpos != std::string::npos) {
                    //
                    //  we need the tag later.
                    mtag=line;
                    olist.push_back(line);
                    search=comment+" End-Of-UDC:";
                    state=1;
                } else {
                    olist.push_back(line);
                }
                break;
            case 1:
                //
                //  Check if we have a end-tag line.
                if (tagpos != std::string::npos) {
                    search=comment+" User-Defined-Code:";
                    olist.push_back(line);
                    mods.insert(std::pair<std::string, std::list<std::string> >(mtag, mlist));
                    mlist.clear();
                    state=0;
                } else {
                    mlist.push_back(line);
                }
                break;
            default:
                break;
            }
        }
    }
    ofile.close();
    //
    // Now we must check whether we have any diffs between the new file and the original file.
    auto gi = glist.begin();
    auto oi = olist.begin();

    while ((gi != glist.end()) && (oi!=olist.end())) {
        if ((*gi) != (*oi)) {
            break;
        }
        gi++;
        oi++;
    }
    //
    // Check if both run to the end. If so they are equal and nothing has to be done.
    if ((gi != glist.end()) || (oi!=olist.end())) {
        int         err=0;
        struct stat dirstat;
        std::string newname;
        //
        //  First check for an existing file and create a backup.
        err=stat(oname.c_str(), &dirstat);
        if (err == 0) {
            newname=oname+".bak";
            err=stat(newname.c_str(), &dirstat);
            if (err == 0) {
                remove(newname.c_str());
            }
            rename(oname.c_str(), newname.c_str());
        }
        nfile.open(oname);
        //
        //  Generate new file but therefor we must insert the modifications.
        //  So we search the tags in the glist.
        state = 0;
        search=comment+" User-Defined-Code:";
        gi=glist.begin();
        while (gi != glist.end()) {
            tagpos=(*gi).find(search);

            switch (state) {
            case 0:
                //
                //  Check if we have a tag line.
                if (tagpos != std::string::npos) {
                    auto mi = mods.find((*gi));

                    if (mi != mods.end()) {
                        for (oi=mi->second.begin();oi != mi->second.end(); ++oi) {
                            nfile << (*oi) << "\n";
                        }
                        mods.erase(mi);
                    }
                    //
                    //  we need the tag later.
                    search=comment+" End-Of-UDC:";
                    state=1;
                } else {
                    nfile << (*gi) << "\n";
                }
                break;
            case 1:
                //
                //  Check if we have a end-tag line.
                if (tagpos != std::string::npos) {
                    search=comment+" User-Defined-Code:";
                    state=0;
                } else {
                    nfile << (*gi) << "\n";
                }
                break;
            default:
                nfile << (*gi) << "\n";
                break;
            }
            gi++;
        }
        auto mi = mods.begin();
        //
        //  Check if any modification has been left over.
        if (mi != mods.end()) {
            nfile << comment << "\n";
            nfile << comment << "\n";
            nfile << comment << " this is a collection of left-over modifications.\n";
        }
        for (;mi!=mods.end(); ++mi) {
            nfile << comment << " " << mi->first << "\n";
            for (oi=mi->second.begin();oi != mi->second.end(); ++oi) {
                nfile << comment << " " << (*oi) << "\n";
            }
            nfile << comment << " end-of-" << mi->first << "\n";
        }
        nfile.close();
    } else {
    }

}
