# MPI Web Parser

This is a forkable repository that handles the boilerplate of building and integrating this library into a networked application. I forked this repository as recommended by (https://github.com/whoshuu/cpr) 

## Building

This project and C++ Requests both use CMake. The first step is to make sure all of the submodules are initialized:

```
git submodule update --init --recursive
```

Then make a build directory and do a typical CMake build from there:

```
mkdir build
cd build
cmake ..
make
```

This should produce a binary in the build directory called `mpi-parser` and another called `seq-parser`. The former is an MPI program, and should be run as such. The latter can simply be run by giving it a URL as argument. For now, both only accept URLs from (www.kabum.com.br)!

An example: 

```
mpiexec -n 10 mpi-parser https://www.kabum.com.br/computadores            
&&
seq-parser https://www.kabum.com.br/computadores            
```

## Documentation



## Requirements

Requires:
    CPR
    GUMBO-PARSER
    JSON FOR MODERN C++
    MPI
    BOOST LIBRARIES
    CURL