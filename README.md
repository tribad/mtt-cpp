# Introduction
This is a code generation tool that transfers UML2 models into various ASCII output files.

It is capable of reading in models from

- StarUML
- Enterprise Architect

and can possibly extended to read in UML2 models from other modelers as well.

While the StarUML modeler uses only a single file in JSON format the EA uses an access database file for local models and 
supports a variety of SQL server for shared repositories.
The generator is ready to use ODBC data sources on Windows OSs and OLEDB providers as well. 
For shared repositories the use of an MSSQL server through the OLE provider has been checked and it works.

## Current Generated Output
The generator produces:
    
* C++ header and source files.
  * Supporting the modeling of Qt applications
  * Supporting the modeling of WxWidgets
  * Support of linking requirements to Model-Elements.
  * Handling of template classes.
  * Support of Namespace.
    * Reducing namespace prefix for types if possible
  * Type parser to reduce need for fqns in the model.
* Makefiles for executables and libraries
* Simulation Objects for [Simulation Core](https://github.com/tribad/simulation-core)
* Creates include statement lists in the source.
* Can produce self-contained headers

There is limited support for

* PHP 
* JScript

It is possible to extent of content output more.

## Manual work for the developer
The generator produces only the skeletons for the classes. So the developer only needs to fill the methods.
A merger stage is embedded that merges existing code into newly generated skeletons.
Leftovers are put as comment to the end of the file.

Some parts of the formatting is configurable through a configuration file. It will be an ongoing efforts to extend the 
configurable features.

While Makefiles are generated as well no cmake support is integrated.

# How to build
For building the generator cmake is used and the code is compilable on Windows and Linux.
On Windows the Visual Studio

## External dependencies

While building on a linux system the only dependencies are

- libxml2

The access to the enterprise architect local files on Linux is realized through the mdbtools.

No library has been integrated but the "mdb-export" command from the mdbtools is used to read-in the data from the access 
database files.

While building on a windows system no specific libraries are needed as everything needed is part of a typical OS 
installation and the build environment. (AFAIK).

# Documentation
I am working on two documents.

* [Architecture](docs/architecture/architecture.md)
* [User Manual](docs/user/manual.md)

