# Pythia
**A Semantic Web Oracle.**   
By Eugen Sawin <esawin@me73.com>.

## Description
Pythia is an experimental platform for semantic web search with keyword
queries.

The goal is to translate keyword queries, as typically used with full-text
search engines, to semantic queries suitable as input for semantic search
engines with high confidence.

The hybrid approach combines full-text search and natural language
processing to provide the best search experience.

## Required Data
The hybrid approach requires a **full-text index** and **knowledge base**.

Currently, the full-text search is conducted via the *Google Custom Search API*,
avoiding the requirement for an index pipeline. Likewise the *Freebase API* is
used for a lightweight access to entity data, in addition to an offline format
of the *YAGO database*.

Due to its huge size, the ontology index is not included in this repository, but
will be
made available online to allow for reproduction of the system and results.

## Requirements
* POSIX.1b-compliant operating system (support for librt)
* GNU Make
* GNU GCC 4.6+ (`sudo apt-get install build-essential`)
* OpenSSL (`sudo apt-get install openssl libssl-dev`)
* Python 2.7+ (optional, for style checking)

## Dependencies
### Required
* SENNA (`make senna` or from <http://ml.nec-labs.com/senna>)
* POCO C++ Libraries (`make poco` or from <http://pocoproject.org>)
* gflags (`make gflags` or from <http://code.google.com/p/gflags>)
* glog (`make glog` or from <http://code.google.com/p/google-glog>)
* Flow (`make flow` or from <https://github.com/eamsen/flow>)

### Optional
* gtest *for testing* (<http://code.google.com/p/googletest>)
* gperftools *for profiling* (<http://code.google.com/p/gperftools>)
* cpplint *for style checking* (`make cpplint`)

## Building
### Building dependencies
To build all dependencies automatically, use:

    make depend

Alternatively, you can build each dependency separately by following the
instructions here, otherwise you may skip the rest of this section.

#### Building gflags
The repository contains a slightly modified gflags version with less verbose
help output, which is used by default. If you prefer using your installed
version, modify the makefile accordingly (follow the comments). 

    make gflags

#### Building glog
To build to included glog version, use:

    make glog

#### Building POCO C++ Libraries
To build the POCO submodule, use:

    make poco

*This will check out the required POCO version from GitHub and build it, which
can take a while.*

#### Building SENNA
To get the required SENNA version, use:

    make senna

*This will download SENNA from the web (~200MB) and modify the source files, so
that it compiles with `g++`. Downloading the files may take a while.*

#### Building Flow
To build the Flow submodule, use:

    make flow

### Building Pythia
To build Pythia use:

    make

To build the version optimized for speed (`-Ofast`), use:

    make opt

For debugging, use the debug version:

    make debug

## Using
To start Pythia use:

    ./pythia

To show the full usage and flags help use:

    ./pythia -help

## Testing (depends on gtest)
To build and run the unit tests use:

    make check

## Profiling (depends on gperftools)
To build Pythia with profiling turned on use:

    make profile

## Getting cpplint
Code style checking depends on a modified version of Google's cpplint. Get it via
  
    make cpplint

## Checking style (depends on cpplint)
To test code style conformance with the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) use:

    make checkstyle

## License
**Pythia - A Semantic Web Oracle.**   
Copyright (C) 2012, 2013 Eugen Sawin

Pythia is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses>.

The full copyright notice is contained in the file *COPYING*.
