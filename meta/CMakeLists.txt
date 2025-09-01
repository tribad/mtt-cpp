##
## Copyright 2020 Hans-Juergen Lange <hjl@simulated-universe.de>
##
## Permission is hereby granted, free of charge, to any person obtaining a copy of
## this software and associated documentation files (the “Software”), to deal in the
## Software without restriction, including without limitation the rights to use, copy,
## modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
## and to permit persons to whom the Software is furnished to do so, subject to the
## following conditions:
##
## The above copyright notice and this permission notice shall be included in all
## copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
## INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
## PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
## HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
## CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
## OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("../threads")

add_library(meta
maction.cpp
mactionnode.cpp
mactivity.cpp
massociation.cpp
massociationend.cpp
mconnector.cpp
mattribute.cpp
mclass.cpp
mdecisionnode.cpp
mdependency.cpp
mdocument.cpp
medge.cpp
melement.cpp
mevent.cpp
mfinalnode.cpp
mfinalstate.cpp
mforknode.cpp
mgeneralization.cpp
minitialnode.cpp
mjoinnode.cpp
mjslifeline.cpp
mjsonmessage.cpp
mlifeline.cpp
mlink.cpp
mmergenode.cpp
mmessage.cpp
mmodel.cpp
mnode.cpp
mobject.cpp
moperation.cpp
mpackage.cpp
mparameter.cpp
mpin.cpp
mpseudostate.cpp
msignal.cpp
msimmessage.cpp
msimsignal.cpp
msimstatemachine.cpp
mstate.cpp
mstatemachine.cpp
mstereotype.cpp
mtransition.cpp
musecase.cpp
mwebpagelifeline.cpp
mcollaboration.cpp
minteraction.cpp
token.cpp
mrequirement.cpp
martifact.cpp
umldiagram.cpp
mnote.cpp
mnotelink.cpp
namespace.h namespace.cpp
typenode.cpp)
