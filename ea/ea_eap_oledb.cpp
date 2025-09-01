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
#include <atldbcli.h>

#include "helper.h"
#include "ea_eap_oledb.h"

#define MAX_RETURN_BUFFER 65000

CDataSource ds;
CSession    dbsession;

bool oledbinit = false;

static bool connect(const char* aConnectionString) {
    bool success = false;
    HRESULT hr;
    CEnumerator oProviders;

    std::string fname = aConnectionString;
    size_t conpos = fname.find("OLEDB=");
    std::string con = fname.substr(6);

    // The following macro is to initialize the conversion routines
    USES_CONVERSION;

    hr = ds.OpenFromInitializationString(A2W(con.c_str()));
    if (SUCCEEDED(hr)) {
        std::cerr << "DB open\n";
        hr = dbsession.Open(ds);
        if (SUCCEEDED(hr)) {
            std::cerr << "DB session opened\n";
        }
        else {
            std::cerr << "Can not create db session\n";
        }
        success = true;
    }
    else {
        std::cerr << "DB could not be opened\n";

    }
    return success;
}

void oledb_close() {
    dbsession.Close();
    ds.Close();
}

tSQLRecordSet* oledb_read_table(const char* aFileName, const char* aTableName) {
    tSQLRecordSet*     retval = nullptr;

    if (!oledbinit) {
        oledbinit = connect(aFileName);
    }
    if (oledbinit) {
        // The following macro is to initialize the conversion routines
        USES_CONVERSION;

        HRESULT hr;
        CTable<CDynamicStringAccessor, CBulkRowset> dbtable;

//        std::ostringstream oss ;

//        oss << "select * from " << aTableName;
        dbtable.SetRows(1000);
        hr = dbtable.Open(dbsession, A2W(aTableName));
        if (SUCCEEDED(hr)) {
            // std::cerr << "Got the table : " << aTableName << "with " << dbtable.GetColumnCount() <<  std::endl;
            retval = new tSQLRecordSet(dbtable.m_nRows+10) ;
            auto fieldcount = dbtable.GetColumnCount();

            retval->FieldCount = fieldcount;

            for (ULONG fc = 0 ; fc < fieldcount; ++fc) {
                auto fieldname = dbtable.GetColumnName(fc+1);

                if (fieldname) {
                    retval->FieldNames.push_back(W2A(fieldname)); 
                }
            }
            if (retval->FieldNames.empty()) {
                std::cerr << "Could not read the column names from the table : " << aTableName << std::endl;
            } else {
                //std::cerr << "Got the column names from table : " << aTableName << std::endl;
            }
            //
            //  Now we read the the rows.
            hr = dbtable.MoveFirst();

            while (SUCCEEDED(hr) && (hr != DB_S_ENDOFROWSET)) {
               //
               
                tRecord rec(retval->FieldCount);
                std::string value;

                for (ULONG fc = 0; fc < fieldcount; ++fc) {
                    if (dbtable.GetString(fc+1)) {
                        value = dbtable.GetString(fc+1);
                    } else {
                        value.clear();
                    }
                    rec.Fields.push_back(value);
                }
                rec.Number = retval->Count++;
                if (rec.Fields.size() != fieldcount) {
                    std::cerr << "not all columns read\n";
                }
                retval->Records.push_back(rec);
                dbtable.ClearRecordMemory();

                hr = dbtable.MoveNext();
            }
            dbtable.Close();
        } else {
            std::cerr << "Cannot read table : " << aTableName << std::endl;
        }
    }
    return retval;
}
