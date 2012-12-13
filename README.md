# Pythia
A Semantic Web Oracle.  
By Eugen Sawin <esawin@me73.com>.

## Version
Nothing to see here yet.

## Requirements
* POSIX.1b-compliant operating system (support for librt)
* GNU Make
* GNU GCC 4.6+ (`sudo apt-get install build-essential`)
* OpenSSL (`sudo apt-get install openssl libssl-dev`)
* Python 2.7+ (optional, for style checking)

## Dependencies
### Required
* SENNA (<http://ml.nec-labs.com/senna>)
* POCO C++ Libraries (<http://pocoproject.org> or `make poco`)
* gflags (<http://code.google.com/p/gflags> or `make gflags`)
* glog (<http://code.google.com/p/google-glog> or `make glog`)

### Optional
* gtest (<http://code.google.com/p/googletest>, for testing)
* gperftools (<http://code.google.com/p/gperftools>, for profiling)
* cpplint (`make cpplint`, for style checking)

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
To build the included POCO version, use:

    make poco

### Building Pythia
To build Pythia use:

    make

For performance measuring use the more optimised version:

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
Pythia - A Semantic Web Oracle.  
Copyright (C) 2012 Eugen Sawin

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
