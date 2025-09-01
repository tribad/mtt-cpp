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
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "json.h"

static int indentation=0;

static std::ostream & dump(std::ostream &output, tJSONArray *array);

inline static void parser_init(tJSON *p) {
    p->state=0;
    p->parent=0;
}

tJSON* parse(std::istream &infile) {
    tJSON* retval = 0;
    char   c;

    while (infile.good()) {
        infile >> c;
        size_t size = 1;

        retval=(tJSON*)parse(retval, (uint8_t*)(&c), size);
        if ((retval != nullptr) && (completed(retval))) {
            break;
        }
    }
    while ((retval != nullptr) && (retval->parent != nullptr)) {
        retval = retval->parent;
    }
    return (retval);
}

tJSON *parse(tJSON *prev, uint8_t*data, size_t& size) {
    size_t      index=0;
    size_t      insize = size;
    bool        again=false;
    tJSON       *root;
    tJSONValue  *jnewvalue;
    tJSONObject *jnewobject;
    tJSONArray  *jnewarray;
    uint8_t     c;

    if (prev==0) {
        root=prev=(tJSON*)(new tJSONObject);
        root->type=eObject;
        parser_init(prev);
    } else {
        root=prev;
        while (root->parent!=0) {
            root=root->parent;
        }
    }
    for (index=0; index<size;) {
        c=data[index];
        switch (root->state) {
        case 0:    // init
            if (c == '{') {
                root->state=1; // we expect a namestart
            }
            break;
        case 1:   // name start
            if (c == '"') {
                root->state=2; // we now take the name
                root->name.clear();
            } else if (c=='}') {
                root->state=-1;
            }
            break;
        case 2:    //   the name literal
            if (c !='"') {
                root->name.push_back((char)(c));
            } else {
                root->state = 3; // Wait colon
            }
            break;
        case 3:    //  Wait colon
            if (c == ':') {
                root->state = 4;  //  Wait next non blank. This tells us what type of object we have to expect.
            }
            break;
        case 4:
            if (!isblank(c)) {
                if (c=='"') {
                    jnewvalue = new tJSONValue;
                    jnewvalue->base.type=eValue;
                    jnewvalue->base.name=root->name;
                    root->name.clear();
                    jnewvalue->base.parent=prev;
                    prev=(tJSON*)jnewvalue;
                    root->state = 199;               //  Wait for end of string

                } else if (toupper(c)=='T') {
                    jnewvalue = new tJSONValue;
                    jnewvalue->base.type=eValue;
                    jnewvalue->base.name=root->name;
                    root->name.clear();
                    jnewvalue->base.parent=prev;
                    prev=(tJSON*)jnewvalue;
                    jnewvalue->value = true;

                    root->state = 200;               //  Wait for end of value
                } else if (toupper(c)=='F') {
                    jnewvalue = new tJSONValue;
                    jnewvalue->base.type=eValue;
                    jnewvalue->base.name=root->name;
                    root->name.clear();
                    jnewvalue->base.parent=prev;
                    prev=(tJSON*)jnewvalue;
                    jnewvalue->value = false;

                    root->state = 200;               //  Wait for end of value
                } else if (toupper(c)=='N') {
                    jnewvalue = new tJSONValue;
                    jnewvalue->base.type=eValue;
                    jnewvalue->base.name=root->name;
                    root->name.clear();
                    jnewvalue->base.parent=prev;
                    prev=(tJSON*)jnewvalue;

                    root->state = 200;               //  Wait for end of value
                } else if (c=='{') {
                    jnewobject = new tJSONObject;
                    jnewobject->base.type=eObject;
                    jnewobject->base.name=root->name;
                    root->name.clear();
                    jnewobject->base.parent=prev;
                    prev=(tJSON*)jnewobject;

                    root->state=1;
                } else if (c=='[') {                   //  Process array
                    jnewarray = new tJSONArray;
                    jnewarray->base.type=eArray;
                    jnewarray->base.name=root->name;
                    root->name.clear();
                    jnewarray->base.parent=prev;

                    prev=(tJSON*)jnewarray;
                } else if (isdigit(c) || (c=='+') || (c=='-')) {
                    jnewvalue = new tJSONValue;
                    jnewvalue->base.type=eValue;
                    jnewvalue->base.name=root->name;
                    root->name.clear();
                    jnewvalue->base.parent=prev;
                    prev=(tJSON*)jnewvalue;
                    root->name.push_back(c);
                    jnewvalue->value=0;
                    root->state = 197;                        //  Process numeric
                } else if (c==']') {
                    root->state = 200;
                }
            }
            break;
        case 197:   //  processing numeric
            if ((c==',') || (c=='}')) {
                tJSONValue* pv=(tJSONValue*)prev;

                if ((*pv) == eVariant::Null) {
                    pv->value = strtol(root->name.c_str(), 0, 10);
                } else if ((*pv)==eVariant::Int) {
                    pv->value = strtol(root->name.c_str(), 0, 10);
                } else {
                    pv->value = strtold(root->name.c_str(), 0);
                }
                again=true;
                root->state=200;
                root->name.clear();
            } else if (((toupper(c)=='E') || (c=='.')) && (*((tJSONValue*)prev) == eVariant::Null)) {
                root->name.push_back(c);
            } else if ((c==' ') && (*((tJSONValue*)prev) == eVariant::Null)) {
            } else {
                root->name.push_back(c);
            }
            break;
        case 198:   // in escape sequence
            //
            // The default follow-up state is the 199.
            // can be changed for special formats.
            root->state = 199;
            if (c=='\\') {
                ((tJSONValue*)prev)->value.push_back('\\');
            } else {
                if (c=='n') {
                    ((tJSONValue*)prev)->value.push_back('\n');
                } else if (c=='t') {
                    ((tJSONValue*)prev)->value.push_back('\t');
                } else if (c=='"') {
                    ((tJSONValue*)prev)->value.push_back('"');
                } else if (c=='r') {
                    ((tJSONValue*)prev)->value.push_back('\r');
                } else if (c=='b') {
                    ((tJSONValue*)prev)->value.push_back('\b');
                } else if (c=='0') {
                    root->name.push_back(c);
                    root->state = 197;           //  octal value
                } else if (c=='x') {
                    root->state = 196;           // hex value
                } else {
                }
            }
            break;
        case 199:                   //  Wait for end of string
            if (c=='\\') {
                root->state = 198;
            } else if (c=='"') {
                root->state = 200;  //  Wait end of value
            } else {
                ((tJSONValue*)prev)->value.push_back(c);
            }
            break;
        case 200:
            if ((c==',') || (c=='}') || (c==']')) {
                if (prev->parent!=0) {
                    if (prev->parent->type==eObject) {
                        ((tJSONObject*)prev->parent)->values.insert(std::pair<std::string, tJSON*>(prev->name, prev));
                        if (c==',') {
                            root->state = 1;      //  Continue with a name
                        } else if (c=='}') {
#if 0
                            prev=prev->parent;
                            if ((prev->parent != 0) && (prev->parent->type==eObject)) {
                                ((tJSONObject*)prev->parent)->values.insert(std::pair<std::string, tJSON*>(prev->name, prev));
                                root->state = 1;      //  Continue with a name
                            } else if ((prev->parent != 0) && (prev->parent->type==eArray)) {
                                ((tJSONArray*)prev->parent)->values.push_back(prev);
                                root->state = 4;      //  Continue with a value;
                            } else if (prev->parent == 0) {
                                root->state = -1;
                            }
#endif
                        }
                    } else if (prev->parent->type==eArray) {
                        ((tJSONArray*)prev->parent)->values.push_back(prev);
                        if (c==',') {
                            root->state = 4;      //  Continue with a value;
                        } else {

                        }
                    }
                    if (prev->parent!=0) {
                        prev=prev->parent;
                        //
                        //  We are now on the root object.
                        if (prev->parent == 0) {
                            //
                            // If it was the closing brace on the root object it is completed.
                            if (c=='}') {
                                root->state=-1;
                            }
                        }
                    }
                } else {
                    if (c=='}') {
                        root->state=-1;
                    }
                }
            }
            break;
        default:
            break;
        }
        if (again) {
            again=false;
        } else {
            index++;
        }
    }
    size = insize - index;
    return (prev);
}

static std::ostream & dump(std::ostream &output, tVariant *value) {
    if (value->isType(eVariant::Double)) {
        output << (long double)(*value);
    } else if (value->isType (eVariant::Int)) {
        output << (long int)(*value);
    } else if (value->isType(eVariant::String)) {
        output << "\"" << (std::string)(*value) << "\"";
    } else if (value->isType(eVariant::Boolean)) {
        if ((bool)(*value) == true) {
            output << "true";
        } else {
            output << "false";
        }
    } else if (value->isType(eVariant::Null)) {
        output << "null";
    }

    return (output);
}
static std::ostream & dump(std::ostream &output, tJSONObject *obj) {
    std::map<std::string, tJSON*>::iterator i;
    bool delimiter=false;

    if (obj->base.name.size() > 0) {
        output << obj->base.name << " {\n";
    } else {
        output << "{\n";
    }
    indentation+=4;
    for (i=obj->values.begin(); i!=obj->values.end(); ++i) {
        if (delimiter) {
            output << ",\n";
        } else {
            delimiter=true;
        }
        for (int x=0;x<indentation;x++) output << ' ';
        output << "\"" << i->first << "\" : ";
        switch(i->second->type) {
        case eValue:
            dump(output, &((tJSONValue*)(i->second))->value);
            break;
        case eArray:
            dump(output, ((tJSONArray*)(i->second)));
            break;
        case eObject:
            dump(output, (tJSONObject*)i->second);
            delimiter=false;
            break;
        default:
            break;
        }
    }
    indentation-=4;
    output << "\n";
    for (int x=0;x<indentation;x++) output << ' ';
    output << "}\n";
    return (output);
}


static std::ostream & dump(std::ostream &output, tJSONArray *array) {
    std::vector<tJSON*>::iterator i;
    output << "[ ";

    for (i=array->values.begin(); i!=array->values.end(); ++i) {
        if (i!=array->values.begin()) {
            output << ", ";
        }
        switch ((*i)->type) {
        case eValue:
            dump(output, &((tJSONValue*)(*i))->value);
            break;
        case eArray:
            dump(output, ((tJSONArray*)(*i)));
            break;
        case eObject:
            dump(output, (tJSONObject*)(*i));
            break;
        default:
            break;
        }
    }
    output << " ]";
    return (output);
}



std::ostream & dump(std::ostream & output, tJSON *root) {
    indentation=0;
//    while (root->parent != 0) root=root->parent;
    return (dump(output, (tJSONObject*)root));
}


tJSON *find(tJSON *root, std::string path) {
    tJSON* retval = nullptr;
    size_t end;
    std::map<std::string, tJSON*>::iterator i;

    if ((root != nullptr) && (root->type==eObject)) {
        end=path.find_first_of('/');

        tJSONObject *o = (tJSONObject*)(root);
        //
        // This is the name of the element in root
        std::string  s = path.substr(0, end);
        //
        //  Go find it in your subs
        i=o->values.find(s);
        if (i!=o->values.end()) {
            //
            //  got it.
            retval = i->second;
            if ((end != std::string::npos) && (end+1 < path.size())) {
                std::string more = path.substr(end+1);

                if (!more.empty()) {
                    retval = find(retval, more);
                }
            }
        } else {
        }
    } else if (root->type==eArray) {
        end=path.find_first_of(':');
    } else {
        retval = root;
    }
    return retval;
}

int to_int(tJSON *j) {
    if (j != nullptr) {
        return ((tJSONValue*)j)->value;
    }
    return 0;
}

unsigned long to_unsigned_long(tJSON *j) {
    if (j != nullptr) {
        return ((((tJSONValue*)j)->value));
    }
    return 0u;
}


long to_long(tJSON *j) {
    if (j != nullptr) {
        return ((long)(((tJSONValue*)j)->value));
    }
    return 0;
}

int64_t to_int64_t(tJSON *j) {
    if (j != nullptr) {
        return ((long)(((tJSONValue*)j)->value));
    }
    return 0;
}

uint8_t to_uint8_t(tJSON *j) {
    uint64_t retval = 0u;

    if (j != nullptr) {
        retval = (((tJSONValue*)j)->value);
    }
    return (uint8_t)retval;
}

uint16_t to_uint16_t(tJSON *j) {
    if (j != nullptr) {
        return (((tJSONValue*)j)->value);
    }
    return 0u;
}

uint64_t to_uint64_t(tJSON *j) {
    if (j != nullptr) {
        return (((tJSONValue*)j)->value);
    }
    return 0u;
}

int16_t to_int16_t(tJSON *j) {
    if (j != nullptr) {
        return (((tJSONValue*)j)->value);
    }
    return 0;
}

bool to_bool(tJSON* j) {
    if (j != nullptr) {
        return ((bool)(((tJSONValue*)j)->value));
    }
    return false;
}

std::string to_string(tJSON *j) {
    if (j != nullptr) {
        return ((std::string)(((tJSONValue*)j)->value));
    }
    return std::string();
}

double to_double (tJSON* j) {
    if (j != nullptr) {
        return ((double)(((tJSONValue*)j)->value));
    }
    return 0.0;
}

bool completed(tJSON *root) {
    if (root != nullptr) {
        return (root->state == -1);
    }
    return (false);
}

#ifdef TEST
#include <stdio.h>

FILE *infile=stdin;

int main(int argc, char **argv) {
    uint8_t buffer[512];
    size_t dataread;
    tJSON *pp=0;

    if (argc > 1) {
        infile=fopen(argv[1], "r+t");
    }
    if (infile != 0) {
        while (!feof(infile)) {
            dataread=fread(buffer, 1, 512, infile);
            pp=parse(pp, buffer, dataread);
        }
        dump(std::cout, pp);
    } else {
    }
    return (0);
}

#endif
