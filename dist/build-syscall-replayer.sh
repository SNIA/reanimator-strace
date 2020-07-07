#!/bin/bash

# TODO: Check for git
# TODO: Check for perl
# TODO: Can we avoid building tests? They slow down the script

# Script variables
numberOfCores="$(nproc --all)"
install=false

# Script functions
function runcmd
{
    echo "CMD: $*"
    sleep 0.2
    "$@"
    ret=$?
    if test $ret -ne 0 ; then
        exit $ret
    fi
}

function printUsage
{
    echo "Usage: ./$0 [--install]"
    exit 0
}

# Parsing script arguments
# TODO: If getopt is portable enough for us, use it.
while [[ $# -gt 0 ]]
do
    key="$1"

    case "${key}" in
        --install)
            install=true
            shift # past argument
            ;;
        -h|--help)
            printUsage
            shift # past argument
            ;;
        *)
            shift # past argument
            ;;
    esac
done

if [[ "${install}" = true ]] ; then
    # Installing programs
    runcmd sudo apt-get install -y autoconf automake cmake gcc g++ perl git

    # Installing packages
    runcmd sudo apt-get install -y libboost-dev libboost-thread-dev \
        libboost-program-options-dev build-essential libxml2-dev libz-dev \
        libaio-dev libtool
fi

# Cloning all repositories
runcmd mkdir build
runcmd cd build
runcmd git clone https://github.com/dataseries/Lintel.git
runcmd git clone https://github.com/dataseries/DataSeries.git
runcmd git clone https://github.com/gperftools/gperftools.git
runcmd git clone https://github.com/oneapi-src/oneTBB.git
runcmd git clone https://github.com/sbu-fsl/trace2model.git
runcmd git clone https://github.com/sbu-fsl/fsl-strace.git

# Building Lintel
runcmd cd Lintel
runcmd cmake .
if [[ "${install}" = true ]] ; then
    runcmd sudo make install
fi
runcmd cd ..

# Building DataSeries
runcmd cd DataSeries
runcmd cmake .
if [[ "${install}" = true ]] ; then
    runcmd sudo make install
fi
runcmd cd ..

# Building tcmalloc
runcmd cd gperftools
runcmd ./autogen.sh
runcmd ./configure
runcmd make -j"${numberOfCores}"
if [[ "${install}" = true ]] ; then
    runcmd sudo make -j"${numberOfCores}" install
fi
runcmd cd ..

# Building tbb
runcmd cd oneTBB
runcmd make tbb_build_prefix=syscall-replayer-build
# TODO: How do you get the exact build directory for tbb? we need it to get 
# tbbvars.sh
runcmd source build/syscall-replayer-build_release/tbbvars.sh
runcmd cd ..

# Building strace2ds-library
runcmd cd trace2model/strace2ds-library
runcmd ./buildall install
if [[ -v STRACE2DS ]]
then
    if [[ -v HOME ]]
    then
        runcmd export STRACE2DS="$HOME/lib/strace2ds"
        # TODO: ask the user what rc file we should append the environment variable
        runcmd echo "export STRACE2DS=\"$HOME/lib/strace2ds\"" >> "$HOME"/.bashrc
    else
        runcmd echo "Could not export STRACE2DS environment variable."
    fi
else
    runcmd echo "Using STRACE2DS=$STRACE2DS"
fi
runcmd cd ../..

# Building fsl-strace
runcmd cd fsl-strace
runcmd ./build-fsl-strace
runcmd cd ..

# Building syscall-replayer
runcmd cd trace2model/syscall-replayer/
runcmd make
runcmd cd ../../..
