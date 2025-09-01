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

#include <cstdio>
#include <iostream>
#include <sstream>
#include <afxdb.h>
#include <odbcinst.h>


#include "helper.h"
#include "ea_eap_windows.h"
#include "ea_eap_oledb.h"

#define MAX_RETURN_BUFFER 65000

static bool DSNAvailable = false;

static bool setupDSN(const char* aFileName) {
    char* returnBuffer = new CHAR[MAX_RETURN_BUFFER];
    WORD  readBytes;
    std::string driverName;

    bool result = ::SQLGetInstalledDrivers(returnBuffer, MAX_RETURN_BUFFER, &readBytes);
    if (result == true) {
        size_t len;
        LPSTR  s = returnBuffer;
#if 1
        while ((len = strlen(s)) > 0) {
            driverName = s;
            if (helper::tolower(driverName).find("microsoft access driver") != std::string::npos) {
                break;
            } else {
                driverName.clear();
            }
            s+=(len+1);
        }
#else
        driverName = "Microsoft Access Driver";
#endif

        if (!driverName.empty()) {
            sprintf(returnBuffer, "DSN=xmi2code%c" "Description=MTT-DSN%c" "DBQ=%s%c\0", 0, 0 ,aFileName , 0);

            result = ::SQLConfigDataSource(nullptr, ODBC_ADD_DSN, driverName.c_str(), returnBuffer);

        }
#if 0
        NULL, ODBC_ADD_DSN, "Excel Files (*.xls)", "DSN=New Excel Data Source\0"                   "Description=New Excel Data Source\0"                   "FileType=Excel\0"                   "DataDirectory=C:\\EXCELDIR\0"
#endif
    }
    return result;
}

tSQLRecordSet* ea_eap_read_table(const char* aFileName, const char* aTableName) {
    tSQLRecordSet*     retval = nullptr;
    std::string        localFileName = aFileName;
    std::string        localDSN;

    if ((localFileName.find("DSN=") == std::string::npos) && (localFileName.find("OLEDB=") == std::string::npos)) {
        if (!DSNAvailable) {
            DSNAvailable = setupDSN(aFileName);
        }
        localDSN = "DSN=xmi2code;UID=";
    } else if (localFileName.find("OLEDB=") != std::string::npos) {
        return oledb_read_table(aFileName, aTableName);
    } else {
        localDSN = localFileName + ";UID=";
    }
    CDatabase db;
//    if (db.OpenEx("DSN=xmi2code;UID=", CDatabase::openReadOnly)) {
    if (db.OpenEx(localDSN.c_str(), CDatabase::openReadOnly)) {
        CRecordset recordset(&db);
        std::ostringstream cmd;
        cmd << "select * from " << aTableName << ";";

        if (recordset.Open(AFX_DB_USE_DEFAULT_TYPE, cmd.str().c_str(), CRecordset::readOnly)) {
            retval = new tSQLRecordSet;
            //
            //  Load field info
            CODBCFieldInfo fieldinfo;
            short fieldcount = recordset.GetODBCFieldCount();

            retval->FieldCount = fieldcount;

            for (short fc = 0 ; fc < fieldcount; ++fc) {
                recordset.GetODBCFieldInfo(fc, fieldinfo);
                retval->FieldNames.push_back(fieldinfo.m_strName.GetString());
            }
            // 
            //  Load records.
            CString value;
            size_t  reccount = 0;
            //
            //  TODO: Set capacity of retval.Records to number of records in recordset.
            while (!recordset.IsEOF()) {
                //
                //  TODO: set capacity of record.Fields to number of fields in record.
                tRecord rec(retval->FieldCount);

                for (short fc = 0; fc < fieldcount; ++fc) {
                    recordset.GetFieldValue(fc, value);
                    rec.Fields.push_back(value.GetString());
                }
                rec.Number = retval->Count++;
                retval->Records.push_back(rec);
                recordset.MoveNext();
            }
        }
    }
    return retval;
}
