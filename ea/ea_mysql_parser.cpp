//
// Copyright 2019 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#include <errno.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include "helper.h"
#include "ea_mysql_parser.h"
#include "massociation.h"
#include "massociationend.h"
#include "mdependency.h"
#include "mgeneralization.h"
#include "mattribute.h"
#include "mparameter.h"
#include "moperation.h"
#include "mlifeline.h"
#include "mwebpagelifeline.h"
#include "mjslifeline.h"
#include "musecase.h"
#include "mclass.h"
#include "mmessage.h"
#include "mjsonmessage.h"
#include "msimmessage.h"
#include "msignal.h"
#include "msimsignal.h"
#include "mpackage.h"
#include "msimstatemachine.h"
#include "mstate.h"
#include "maction.h"
#include "mpseudostate.h"
#include "mfinalstate.h"
#include "mtransition.h"
#include "mevent.h"
#include "mpin.h"
#include "medge.h"
#include "mnode.h"
#include "minitialnode.h"
#include "mfinalnode.h"
#include "mactionnode.h"
#include "mdecisionnode.h"
#include "mmergenode.h"
#include "mjoinnode.h"
#include "mforknode.h"
#include "mactivity.h"
#include "mobject.h"
#include "mmodel.h"

#include "mstereotype.h"

/*
 * This is the model we construct.
 */
static MModel* model = nullptr;

/*
 * This is the only function that gets used by main program
 */
MModel* ea_mysql_modelparser(const char* filename, const char* directory)
{
    model=MModel::construct();
    std::ifstream      infile(filename);
    int                err=0;
    struct stat        dirstat;
    std::string        path;
    //
    //  Set the directory to the current working directory.
	path = helper::getcwd();
	//
    //  First check for relative directory and create it if needed.
    if (directory[0]!='/') {
        path = path + "/" + directory;
    } else {
        path = directory;
    }

    err=stat(path.c_str(), &dirstat);
    if ((err == -1) && (errno==ENOENT)) {
        helper::mkdir(path, 0777);
    } else if ((err==0) && (!S_ISDIR(dirstat.st_mode))) {
        std::cerr << "Not a directory " << path << "\n";
        return (0);
    }

    helper::chdir(path);

    if (model != 0) {
    }
    return (model);
}
