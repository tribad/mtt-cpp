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
#ifndef JSON_H
#define JSON_H

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <variant.h>

typedef enum enumJSON {
    eValue=0x0001,
    eObject,
    eArray
} eJSON;

typedef struct tagJSON {
    struct tagJSON *parent;
    int            state;
    eJSON          type;
    std::string    name;
} tJSON;

typedef struct tagJSONValue {
    tJSON    base;
    tVariant value;
    operator std::string () {return (std::string)value;};
    bool operator==(eVariant vmatch) {return value.isType(vmatch);};
} tJSONValue;

typedef struct tagJSONArray {
    tJSON base;
    std::vector<tJSON*> values;
} tJSONArray;

typedef struct tagJSONObject  {
    tJSON base;
    std::map<std::string, tJSON*> values;
} tJSONObject;

typedef struct tagJSONParser {
    int   state;
    tJSON *root;
    tJSON *now;
} tJSONParser;

tJSON *parse(tJSON *prev, uint8_t*data, size_t &size) ;
tJSON* parse(std::istream& infile);

std::ostream & dump(std::ostream & output, tJSON *root) ;
tJSON *find(tJSON *root, std::string path);
bool completed(tJSON* root);


double        to_double(tJSON*);
int           to_int(tJSON*);
unsigned long to_unsigned_long(tJSON *j) ;
uint8_t       to_uint8_t(tJSON* j);
uint16_t      to_uint16_t(tJSON* j);
uint64_t      to_uint64_t(tJSON* j);
int16_t       to_int16_t(tJSON* j);
int64_t       to_int64_t(tJSON*);
std::string   to_string(tJSON*);
bool          to_bool(tJSON*);
long          to_long(tJSON*);

#endif // JSON_H

