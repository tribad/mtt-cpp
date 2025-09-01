//
// Copyright 2018 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include <sstream>
#include "helper.h"
#include "variant.h"

tVariant::tVariant(const tVariant& in){
    type = in.type;
    switch (type) {
    case eVariant::Double:
        dbl = in.dbl;
        break;
    case eVariant::Int:
        il  = in.il;
        break;
    case eVariant::Unsigned:
    case eVariant::Boolean:
        ul  = in.ul;
        break;
    case eVariant::Reference:
        ptr = in.ptr;
        break;
    case eVariant::String:
        ptr = strdup((const char*)(in.ptr));
        break;
    default:
        break;
    }
}


const tVariant& tVariant::operator=(const tVariant& in){
    if (type == eVariant::String) {
        free(ptr);
    }
    type = in.type;
    switch (type) {
    case eVariant::Double:
        dbl = in.dbl;
        break;
    case eVariant::Int:
        il  = in.il;
        break;
    case eVariant::Unsigned:
    case eVariant::Boolean:
        ul  = in.ul;
        break;
    case eVariant::Reference:
        ptr = in.ptr;
        break;
    case eVariant::String:
        ptr = strdup((const char*)(in.ptr));
        break;
    default:
        break;
    }
    return (*this);
}

tVariant::operator uint64_t() const {
    uint64_t retval = 0ul;

    switch (type) {
    case eVariant::Double:
        retval = (uint64_t)dbl;
        break;
    case eVariant::Int:
        retval = il;
        break;
    case eVariant::Unsigned:
        retval = ul;
        break;
    case eVariant::Reference:
        retval = *((uint64_t*)(ptr));
        break;
    case eVariant::String:
        retval = strtoull((char*)ptr, 0, 0);
        break;
    case eVariant::Boolean:
        if (ul == 0) {
            retval = 0;
        } else {
            retval = 1;
        }
        break;
    case eVariant::Null:
        retval = 0ul;
        break;
    }
    return retval;
}

tVariant::operator long double() const {
    long double retval = 0.0;

    switch (type) {
    case eVariant::Double:
        retval = dbl;
        break;
    case eVariant::Int:
        retval = il;
        break;
    case eVariant::Unsigned:
        retval = ul;
        break;
    case eVariant::Reference:
        retval = *((double*)(ptr));
        break;
    case eVariant::String:
        retval = strtod((const char*)ptr, 0);
        break;
    case eVariant::Boolean:
        if (ul == 0) {
            retval = 0.0;
        } else {
            retval = 1.0;
        }
        break;
    case eVariant::Null:
        retval = 0.0;
        break;
    }
    return retval;
}

tVariant::operator bool() const {
    bool retval = false;

    switch (type) {
    case eVariant::Double:
        if (dbl == 0.0) {
            retval = false;
        } else {
            retval = true;
        }
        break;
    case eVariant::Int:
        if (il == 0) {
            retval = false;
        } else {
            retval = true;
        }
        break;
    case eVariant::Unsigned:
    case eVariant::Boolean:
        if (ul == 0) {
            retval = false;
        } else {
            retval = true;
        }
        break;
    case eVariant::Reference:
        if (ptr == 0) {
            retval = false;
        } else {
            retval = true;
        }
        break;
    case eVariant::String:
        if ((helper::tolower((const char*)ptr) == "true") || ((*this) != 0ul)) {
            retval = true;
        } else {
            retval = false;
        }
        break;
    case eVariant::Null:
        retval = false;
        break;
    }
    return retval;
}

tVariant::operator void*() const {
    void* retval = nullptr;

    switch (type) {
    case eVariant::Double:
    case eVariant::Int:
    case eVariant::Unsigned:
    case eVariant::Boolean:
    case eVariant::Null:
        break;
    case eVariant::Reference:
    case eVariant::String:
        retval = ptr;
        break;
    default:
        break;
    }
    return retval;
}

tVariant::operator int64_t () const {
    int64_t retval = 0;

    switch (type) {
    case eVariant::Double:
        retval = (int64_t)dbl;
        break;
    case eVariant::Int:
        retval = il;
        break;
    case eVariant::Unsigned:
        retval = ul;
        break;
    case eVariant::Reference:
        retval = *((int64_t*)(ptr));
        break;
    case eVariant::String:
        retval = strtoll((char*)ptr, 0, 0);
        break;
    case eVariant::Boolean:
        if (ul == 0) {
            retval = 0;
        } else {
            retval = 1;
        }
        break;
    case eVariant::Null:
        retval = 0;
        break;
    }
    return retval;
}

tVariant & tVariant::push_back(char c) {
    char cc[2] = {c, '\0'};
    if (type != eVariant::String) {
        (*this) = cc;
    } else {
        char* longerstring = (char*)malloc(strlen((const char*)(ptr))+2);
        strcpy(longerstring ,(const char*)ptr);
        free (ptr);
        strcat(longerstring, cc);
        ptr = longerstring;
    }

    return *this;
}

tVariant::tVariant(const char* t) {
    type = eVariant::String;
    dbl = 0;
    ul = 0;
    il = 0;
    if (t != 0) {
        ptr = malloc(std::strlen(t)+1);
#ifdef DEBUG
        memset(ptr, 0, std::strlen(t)+1);
#endif
        std::strcpy((char*)ptr, t);
    } else {
        ptr = 0;
        type = eVariant::Null;
    }
}

bool tVariant::operator==(const tVariant& aCheck) const {
    bool retval = false;

    switch (type) {
    case eVariant::Double:
        retval = (dbl == (double)aCheck);
        break;
    case eVariant::Int:
        retval = (il == (int64_t)aCheck);
        break;
    case eVariant::Unsigned:
        retval = (ul == (uint64_t)aCheck);
        break;
    case eVariant::Boolean:
        retval = ((bool)ul == (bool)aCheck);
        break;
    case eVariant::Reference:
        if (ptr != nullptr) {
            retval = (ptr == (void*)aCheck);
        } else {
        }
        break;
    case eVariant::String:
        if ((ptr == nullptr) && (aCheck.ptr == nullptr)) {
            retval = true;
        } else {
            if ((ptr == nullptr) || (aCheck.ptr == nullptr)) {
                retval = false;
            } else {
                retval = !strcmp((const char*)ptr, ((std::string)(aCheck)).c_str());
            }
        }
        break;
    case eVariant::Null:
        break;
    }
    return retval;
}

bool tVariant::operator!=(const tVariant& aCheck) const {
    bool retval = false;

    switch (type) {
    case eVariant::Double:
        retval = (dbl != (double)aCheck);
        break;
    case eVariant::Int:
        retval = (il != (int64_t)aCheck);
        break;
    case eVariant::Unsigned:
        retval = (ul != (uint64_t)aCheck);
        break;
    case eVariant::Boolean:
        retval = ((bool)ul != (bool)aCheck);
        break;
    case eVariant::Reference:
        if (ptr != nullptr) {
            retval = (ptr != (void*)aCheck);
        } else {
        }
        break;
    case eVariant::String:
        if ((ptr == nullptr) && (aCheck.ptr == nullptr)) {
            retval = false;
        } else {
            if ((ptr == nullptr) || (aCheck.ptr == nullptr)) {
                retval = true;
            } else {
                retval = strcmp((const char*)ptr, ((std::string)(aCheck)).c_str());
            }
        }
        break;
    case eVariant::Null:
        retval = (type != aCheck.type);
        break;
    }
    return retval;
}


tVariant tVariant::operator ++(int) {
    tVariant retval;
    switch (type) {
    case eVariant::Double:
        retval = dbl++;
        break;
    case eVariant::Int:
        retval = il++;
        break;
    case eVariant::Unsigned:
        retval = ul++;
        break;
    default:
        retval = *this;
        break;
    }
    return retval;
}
tVariant tVariant::operator ++() {
    tVariant retval;
    switch (type) {
    case eVariant::Double:
        retval = ++dbl;
        break;
    case eVariant::Int:
        retval = ++il;
        break;
    case eVariant::Unsigned:
        retval = ++ul;
        break;
    default:
        retval = *this;
        break;
    }
    return retval;
}
tVariant tVariant::operator --(int) {
    tVariant retval;
    switch (type) {
    case eVariant::Double:
        retval = dbl--;
        break;
    case eVariant::Int:
        retval = il--;
        break;
    case eVariant::Unsigned:
        retval = ul--;
        break;
    default:
        retval = *this;
        break;
    }
    return retval;
}

tVariant tVariant::operator --() {
    tVariant retval;
    switch (type) {
    case eVariant::Double:
        retval = --dbl;
        break;
    case eVariant::Int:
        retval = --il;
        break;
    case eVariant::Unsigned:
        retval = --ul;
        break;
    default:
        retval = *this;
        break;
    }
    return retval;
}

unsigned long operator-(unsigned long a, const tVariant& value) {
    return (-((unsigned long)(value) - a));
}

bool operator>(uint64_t a, const tVariant& value) {
    return ((unsigned long)(value) < a);
}

bool operator<(const tVariant& value, unsigned long a) {
    return ((unsigned long)(value) < a);
}
