# Pythia
A semantic search oracle.  
By Eugen Sawin <esawin@me73.com>.

## Version

## Requirements
* POSIX.1b-compliant operating system (librt)
* GNU GCC 4.6 or newer
* GNU Make
* Python 2.7 or newer (only for style checking)

## Dependencies
### Required
* SENNA (<http://ml.nec-labs.com/senna>)
* gflags (<http://code.google.com/p/gflags> or `$ make gflags`)

### Optional
* gtest (<http://code.google.com/p/googletest>, only for testing)
* gperftools (<http://code.google.com/p/gperftools>, only for profiling)
* cpplint (`$ make cpplint`, only for style checking)

## Building gflags
The repository contains a slightly modified gflags version with less verbose
help output.  
If you want to use the provided version instead, you need to build gflags
locally:

    $ make gflags

and then activate the two lines in the makefile, which are commented out.
Alternatively you can build all dependencies at once:

    $ make depend

## Building Pythia (depends on gflags)
To build Pythia use:

    $ make

For performance measuring use the more optimised version:

    $ make opt

For debugging, use the debug version:

    $ make debug

## Using Pythia
To start ace use:

    $ pythia

To show the full usage and flags help use:

    $ pythia -help

## Testing Pythia (depends on gtest)
To build and run the unit tests use:

    $ make check

## Profiling Pythia (depends on gperftools)
To build Pythia with profiling turned on use:

    $ make profile

## Getting cpplint
Code style checking depends on a modified version of Google's cpplint. Get it via
  
    $ make cpplint

## Checking style (depends on cpplint)
To test code style conformance with the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) use:

    $ make checkstyle

## License
Pythia - a semantic search oracle.  
Copyright (C) 2012  Eugen Sawin

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
