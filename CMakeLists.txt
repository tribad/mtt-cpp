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
##
##  This is the cmake intro
cmake_minimum_required(VERSION 3.15)
##
##  Abort if in source build
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()
##
##  Setting the project name
project(mtt-cpp CXX)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)
string(REGEX REPLACE "-" "_" UPPER_PROJECT_NAME ${UPPER_PROJECT_NAME})

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()
##
##  Load values from the os-release information.
if (EXISTS "/etc/os-release")
    file(STRINGS "/etc/os-release" OSINFO)
    foreach(NameAndValue ${OSINFO})
        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
        # Find variable name
        string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
        # Find the value
        string(REPLACE "${Name}=" "" Value ${NameAndValue})
        string(REPLACE "\"" "" Value ${Value})
        # Set the variable
        set(${Name} "${Value}")
    endforeach()
    set(LIBXML_INCLUDE_DIR "/usr/include/libxml2")
    set(LIBXML_LIBRARY_NAME "xml2")
    add_compile_options (-O0 -g -Wall -Wextra -Wno-unused-parameter -std=c++14)

    set(PLATFORM_TARGET_LIBS -Wl,--start-group meta staruml eauml ${PLATFORM_LIBS} json ${LIBXML_LIBRARY_NAME} -Wl,--end-group)

else()
    message(INFO "os-release info not found")
    ##
    ##  expect a windows os.
    set(ID "Windows")
    add_compile_options (/FS /Zi /D_AFXDLL /execution-charset:utf-8 /UUNICODE /U_UNICODE /D_CRT_SECURE_NO_WARNINGS )
    set(PLATFORM_TARGET_LIBS meta staruml eauml ${PLATFORM_LIBS} json ${LIBXML_LIBRARY_NAME} legacy_stdio_definitions.lib)
endif()
##
##  Load xmi2code version number
if (EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
    file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION VINFO)
    foreach(NameAndValue ${VINFO})
        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
        # Find variable name
        string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
        # Find the value
        string(REPLACE "${Name}=" "" Value ${NameAndValue})
        # Set the variable
        set(${Name} "${Value}")
    endforeach()
    set(${UPPER_PROJECT_NAME}_VERSION_MAJOR "${VERSION_MAJOR}")
    set(${UPPER_PROJECT_NAME}_VERSION_MINOR "${VERSION_MINOR}")
    set(${UPPER_PROJECT_NAME}_VERSION_PATCH "${VERSION_PATCH}")
else()
    message(WARNING "version info not found")
    set(${UPPER_PROJECT_NAME}_VERSION_MAJOR "0")
    set(${UPPER_PROJECT_NAME}_VERSION_MINOR "0")
    set(${UPPER_PROJECT_NAME}_VERSION_PATCH "0")
endif()

message(INFO ${UPPER_PROJECT_NAME})
##add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
##add_link_options(-fsanitize=address)
##
##  Create the version header.
configure_file( ${CMAKE_SOURCE_DIR}/version.h.in  ${CMAKE_SOURCE_DIR}/version.h)

##
##  This is the application install dir. Set fixed for now. Should be changed.
set (XDG_APPS_INSTALL_DIR /usr/local/bin)

##
##  Setup the global compile options for now only debug is supported.

##
##  setting the base dir to be included in the include path list.
include_directories(${CMAKE_SOURCE_DIR} json meta ${LIBXML_INCLUDE_DIR})
##
## set the executable name
set (PROJECT_EXEC_NAME ${PROJECT_NAME}-${${UPPER_PROJECT_NAME}_VERSION_MAJOR}.${${UPPER_PROJECT_NAME}_VERSION_MINOR}.${${UPPER_PROJECT_NAME}_VERSION_PATCH})
##
##  take the source
add_subdirectory(meta)
add_subdirectory(staruml)
add_subdirectory(ea)
add_subdirectory(json)
##
##  Setup the executable name.
add_executable(${PROJECT_NAME} caction.cpp
    cactionnode.cpp
    cactivity.cpp
    cassociation.cpp
    cassociationend.cpp
    cattribute.cpp
    ccclass.cpp
    cclassbase.cpp
    cclass.cpp
    ccollaboration.cpp
    ccxxclass.cpp
	cmoduleclass.cpp
    cdatatype.cpp
    cdecisionnode.cpp
    cdependency.cpp
    cdocument.cpp
    cedge.cpp
    cenumeration.cpp
    cevent.cpp
    cexecutablepackage.cpp
    cexecutablewx.cpp
    cexternclass.cpp
    cexternpackage.cpp
    cfinalnode.cpp
    cfinalstate.cpp
    cforknode.cpp
    cgeneralization.cpp
    cinitialnode.cpp
    cinterface.cpp
    cjoinnode.cpp
    cjsclass.cpp
    cjslifeline.cpp
    cjspackage.cpp
    clibrarypackage.cpp
	cmodulepackage.cpp
    clifeline.cpp
    clink.cpp
    cmergenode.cpp
    cmessageclass.cpp
    cmessage.cpp
    cmodel.cpp
    cmodelpackage.cpp
    cnode.cpp
    cobject.cpp
    coperation.cpp
    cpackagebase.cpp
    cpackage.cpp
    cparameter.cpp
    cphpclass.cpp
    cphppackage.cpp
    cpin.cpp
    cprimitivetype.cpp
    cprofilepackage.cpp
    cpseudostate.cpp
    cqtclass.cpp
    crc64.cpp
    csignalclass.cpp
    csignal.cpp
    csimenumeration.cpp
    csimmessage.cpp
    csimobjectv2.cpp
    csimsignal.cpp
    csimstatemachine.cpp
    csimstruct.cpp
    csimulationpackage.cpp
    cstate.cpp
    cstruct.cpp
    ctransition.cpp
    cunion.cpp
    cusecase.cpp
    cwebpagelifeline.cpp
    chttpifcpackage.cpp
    chtmlpageclass.cpp
    cwxformsclass.cpp
    cinteraction.cpp
    cconnector.cpp
    path.cpp
    main.cpp
    helper.cpp
    variant.cpp
    crequirement.cpp
    cartifact.cpp
    cdiagram.cpp
    cnote.cpp
    cnotelink.cpp
        csubsystempackage.cpp HeaderList.cpp
        configparser.h configparser.cpp systemtime.cpp  mttxml.cpp namespacenode.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}-${${UPPER_PROJECT_NAME}_VERSION_MAJOR}.${${UPPER_PROJECT_NAME}_VERSION_MINOR}.${${UPPER_PROJECT_NAME}_VERSION_PATCH})
##
##  What to link to make this executable.
target_link_libraries(${PROJECT_NAME} ${PLATFORM_TARGET_LIBS} -lsqlite3 )

install ( TARGETS  ${PROJECT_NAME} DESTINATION bin)
