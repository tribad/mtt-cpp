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
#include <iostream>
#include <sstream>
#include "helper.h"
#include "melement.h"

std::map<std::string, std::shared_ptr<MElement>> MElement::Instances;


MElement::MElement()
{
    type       = eElementType::Element;
    visibility = vNone;
}

MElement::MElement(const std::string & aId, std::shared_ptr<MElement> aParent)
{
    if (aParent != nullptr) {
        parent = aParent;
    }
    id         = aId;
    type       = eElementType::Element;
    visibility = vNone;
    auto result = MElement::Instances.insert(std::pair<std::string, std::shared_ptr<MElement>>(id, this));

    if (!result.second) {
        std::cerr << "Insert failed for " << id << std::endl;
    }
}

void MElement::Delete(std::shared_ptr<MElement> aElement)
{
    //
    //  Need an iterator here to erase the list element in between.
    for (auto oi = owned.begin(); oi != owned.end();++oi) {
        if (*oi == aElement) {
            owned.erase(oi);
            break;
        }
    }
}

std::list<std::string> MElement::GetComment(size_t length) {
    std::list<std::string> lines;

    std::string l;
    size_t nl_pos = 0;
    std::istringstream iss;

    if (!comment.empty()) {
        //
        //  Replace special characters
        do {
            nl_pos=comment.find("\\n", nl_pos);
            if (nl_pos != std::string::npos) {
                comment.replace(nl_pos, 2, "\n");
            }
        } while (nl_pos != std::string::npos);

        iss.str(comment);
        //
        //  Do the line by line processing.
        do {
            std::getline(iss, l);
            //
            //  Remove trailing newlines from the comment.
            while ((l.size() > 0) && ((l.back() == '\n') || l.back() == '\r')) {
                l = l.substr(0, l.size() - 1);
            }
            //
            //  Need to process only non empty lines.
            if (!l.empty()) {
                //
                //  As the comment may contain HTML special characters we replace them.
                l = helper::fromHTML(l);
                //
                // if line is longer than the max line length we need to split them.
                if (l.size() > length) {
                    //
                    //  Split line.
                    size_t position;
                    //
                    //  Start at the possible max position requested from length.
                    size_t start = length;

                    do {
                        //
                        //  From the split-position (start) on, we look for a gap that we can use
                        //  to split the line without splitting within a word.
                        position = l.find_first_of(" \n\r", start);
                        //
                        // Found the gap.
                        if (position != std::string::npos) {
                            //
                            //  replace the blank and a possible CR character with a newline.
                            if ((l[position] == ' ') || (l[position] == '\r')) {
                                l.replace(position, 1, "\n");
                            }
                            else {

                            }
                            start = position+length;
                        }
                    } while ((position != std::string::npos) && (start < l.size()));
                } else {
                }
                //
                //  At this point we have \n characters where we split the line in segments.
                std::string line;
                size_t      start       = 0;
                size_t      end         = 0;
                //
                //  Split loop starts here.
                do {
                    end = l.find_first_of('\n', start);
                    if (end != std::string::npos) {
                        line = l.substr(start, end-start);
                    } else {
                        line = l.substr(start);
                    }
                    /*
                        * trim front
                        */
                    while ((!line.empty()) && (line[0]==' ')) line.erase(0, 1);
                    /*
                        * if the line has content after trimming we add the line to our lines.
                        */
                    if (!line.empty()) {
                        lines.push_back(line);
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
            } else {
                lines.push_back("");
            }
        } while (iss.good());
        //
        //  Remove empty lines from the back
        for (auto ol = lines.rbegin(); ol != lines.rend(); ++ol) {
            if (ol->empty()) {
                lines.pop_back();
            } else {
                break;
            }
        }
    }

    return lines;
}

void MElement::DumpComment(std::ostream &output, int spacer, size_t length,
                           std::string commentintro, std::string commentcontinue,
                           std::string commentend) {
    std::string filler;
    std::istringstream iss;
    std::list<std::string> outputlines;

    if (!comment.empty()) {
        filler.assign(spacer, ' ');
        outputlines = GetComment(length);
        if (!outputlines.empty()) {
            output << filler << commentintro << "\n";
            //
            // Dump the lines.
            for (auto ol = outputlines.begin(); ol != outputlines.end(); ++ol) {
                if (commentcontinue.empty()) {
                    output << *ol << std::endl;
                } else {
                    output << filler << commentcontinue << ' ' << *ol << std::endl;
                }
            }
            if (!commentend.empty()) {
                output << filler << commentend << "\n";
            }
        }
    } else {
    }
}


bool MElement::HasTaggedValue(const std::string& aName) const {
    bool retval = false;

    for (auto t : tags) {
        if (helper::tolower(t.first) == helper::tolower(aName)) {
            retval = true;
            break;
        }
    }
     return retval;
}

std::string MElement::GetTaggedValue(const std::string& aName) const {
    std::string retval;
    for (auto t : tags) {
        if (helper::tolower(t.first) == helper::tolower(aName)) {
            retval = t.second;
            break;
        }
    }
    return retval;
}

void MElement::Add(std::shared_ptr<MElement> aElement) {
    if (aElement->mPosition != -1) {
        //
        //  Check for quick append.
        if ((owned.empty()) || ((*owned.rbegin())->mPosition < aElement->mPosition)) {
            owned.emplace_back(aElement);
        } else {
            //
            //  Insert at position.
            auto o = owned.begin();

            while (o != owned.end()) {
                if (aElement->mPosition < (*o)->mPosition) {
                    owned.insert(o,aElement);
                    break;
                 }
                 o++;
            }
        }
    } else {
        owned.emplace_back(aElement);
    }
}

std::list<std::shared_ptr<MElement>> MElement::getOwned(eElementType aType) {
    std::list<std::shared_ptr<MElement>> retval;

    for (auto & o : owned) {
        if (o->type == aType) {
            retval.push_back(*o);
        }
    }
    return retval;
}

std::string MElement::FQN() const {
    return name;
}
void MElement::Prepare(void) {

}
void MElement::Dump(std::shared_ptr<MModel> aModel) {

}

std::string MElement::getModelPath() {
    std::string retval;

    if (parent) {
        retval = parent->getModelPath() + '/' + name;
    } else {
        retval = "/" + name;
    }

    return retval;
}
