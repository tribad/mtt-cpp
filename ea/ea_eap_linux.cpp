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
#include <sstream>

#include "ea_eap_linux.h"

tSQLRecordSet* ea_eap_read_table(const char* aFileName, const char* aTableName) {
    tSQLRecordSet*     retval = nullptr;
    std::ostringstream cmd;

    cmd << "mdb-export " << aFileName << " " << aTableName;
    FILE* mdbcommand = popen(cmd.str().c_str(), "r");

    if (mdbcommand != nullptr) {
        //
        //
        int error = ferror(mdbcommand);
        if (error == 0) {
            std::string   buffer;
            bool          readheader = true;
            bool          inescape   = false;
            bool          intext     = false;
            unsigned long reccount = 0;
            tRecord       rec;

            retval = new tSQLRecordSet;


            while (!feof(mdbcommand)) {
                char c = fgetc(mdbcommand);

                switch (c) {
                case ',':       // end-of-field
                    if (!intext) {
                        if (readheader) {
                            retval->FieldNames.push_back(buffer);
                            retval->FieldCount++;
                        } else {
                            rec.Fields.push_back(buffer);
                        }
                        buffer.clear();
                    } else {
                        buffer.push_back(c);
                    }
                    break;
                case '"':
                    if (intext) {
                        intext = false;
                    } else {
                        intext = true;
                    }
                    break;
                case '\xff':    //  early end-of-file detection
                    break;
                case '\n':
                case '\0':      //  end-of-line
                    if (!intext) {
                        if (readheader) {
                            retval->FieldNames.push_back(buffer);
                            retval->FieldCount++;
                            readheader = false;
                        } else {
                            rec.Fields.push_back(buffer);
                            //
                            //  anyways we move on to the next record.
                            rec.Number = reccount++;
                            retval->Records.push_back(rec);

                            rec.Fields.clear();
                        }
                        buffer.clear();
                    } else {
                        buffer.push_back(c);
                    }
                    break;
                case '\\':
                    if (inescape) {
                        buffer.push_back('\\');
                        inescape = false;
                    } else {
                        inescape = true;
                    }
                    break;
                default:
                    if (inescape) {
                        inescape = false;
                        switch (c) {
                        case 'n':
                            buffer.push_back('\n');
                            break;
                        case 't':
                            buffer.push_back('\t');
                            break;
                        case '"':
                            buffer.push_back('\"');
                            break;
                        case '\'':
                            buffer.push_back('\'');
                            break;
                        case 'r':
                            buffer.push_back('\r');
                            break;
                        default:
                            break;
                        }
                    } else {
                        buffer.push_back(c);
                    }
                    break;
                }
            }
        }
        pclose(mdbcommand);
        retval->Count = retval->Records.size();
    }

    return retval;
}

tSQLRecordSet* ea_eap_load_table_object(const char* aFileName) {
    tSQLRecordSet* retval = nullptr;

    retval = ea_eap_read_table(aFileName, "t_object");
    return retval;
}
