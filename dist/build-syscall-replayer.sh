#!/bin/bash
#
# Build the program syscall-replayer, installing any required dependecies if
# requested.

# TODO: Check for git
# TODO: Check for perl
# TODO: Can we avoid building tests? They slow down the script

# Script variables
readonly numberOfCores="$(nproc --all)"
install=false
installPackages=false
configArgs=""

readonly programDependencies=("autoconf" "automake" "cmake" "gcc" "g++" "perl" "git")
missingPrograms=()

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
    echo "Usage: ./$0 [--install]" >&2
    exit 0
}

# Parsing script arguments
while [[ $# -gt 0 ]]; do
    key="$1"

    case "${key}" in
        --config-args)
            shift # past argument
            "${configArgs}"="$1"
            shift # past value
            ;;
        --install)
            install=true
            shift # past argument
            ;;
        --install-packages)
            if command -v apt-get >/dev/null; then
                installPackages=true
            else
                echo "Could not find apt-get. Missing packages must be \
                installed manually." >&2
            fi
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

if [[ "${install}" = true ]]; then
    if ! command -v sudo &>/dev/null; then
        echo "Script could not find 'sudo' command. Cannot install." >&2
        exit 1
    fi
fi

# Check whether program dependencies are installed
for program in "${programDependencies[@]}"; do
    programPath=$(command -v "${program}")
    if [[ $? == 0 ]]; then
        echo "${program}: Located at ${programPath}"
    elif [[ "${installPackages}" == true ]]; then
        echo "${program}: Not found. Installing..."
        runcmd sudo apt-get install -y "${program}"
    else
        echo "${program}: Not found."
        missingPrograms+=("${program}")
    fi
done

# Check whether the user has all required programs for building
if [[ "${#missingPrograms[@]}" -gt 0 ]]; then
    echo "Could not find all required programs. Not found:"
    for program in "${missingPrograms[@]}"; do
        echo "  ${program}"
    done

    echo "To install on a Debian-based system, run the command"
    echo "  sudo apt-get install ${missingPrograms[*]}"
    exit 1
fi

if [[ "${install}" = true ]]; then
    
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
# TODO: Should each build be done in a subshell to avoid errors with cd?
runcmd cd Lintel
runcmd cmake .
if [[ "${install}" = true ]]; then
    runcmd sudo make install
fi
runcmd cd ..

# Building DataSeries
runcmd cd DataSeries
runcmd cmake .
if [[ "${install}" = true ]]; then
    runcmd sudo make install
fi
runcmd cd ..

# Building tcmalloc
runcmd cd gperftools
runcmd ./autogen.sh
runcmd ./configure "${configArgs}"
runcmd make -j"${numberOfCores}"
if [[ "${install}" = true ]]; then
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
runcmd autoreconf -v -i
runcmd rm -rf BUILD
runcmd mkdir -p BUILD
runcmd mkdir -p xml
runcmd cd tables
runcmd perl gen-xml-enums.pl
runcmd cd ../
runcmd cp -r ./xml BUILD
runcmd cd BUILD
# TODO: On non-root build, where do we install the library? We need the strace
# binary to be able to find these libraries during runtime WITHOUT relying on
# an environment variable
if [[ "${install}" == true ]]; then
    runcmd ../configure --enable-shared --disable-static --prefix=/usr/local/strace2ds
else
    runcmd ../configure --enable-shared --disable-static --prefix="${HOME}"/lib/strace2ds
fi
runcmd make clean
runcmd make -j"${numberOfCores}"
if [[ "${install}" == true ]]; then
    runcmd sudo make install
fi

# if [[ -v STRACE2DS ]]; then
#     if [[ -v HOME ]]; then
#         runcmd export STRACE2DS="$HOME/lib/strace2ds"
#         # TODO: ask the user what rc file we should append the environment variable
#         runcmd echo "export STRACE2DS=\"$HOME/lib/strace2ds\"" >> "$HOME"/.bashrc
#     else
#         runcmd echo "Could not export STRACE2DS environment variable."
#     fi
# else
#     runcmd echo "Using STRACE2DS=$STRACE2DS"
# fi
runcmd cd ../../..

# Building fsl-strace
runcmd cd fsl-strace
# TODO: the build script for fsl-strace expects the strace2ds libraries to be
# at $HOME/lib/strace2ds. Does this still work if the libraries are installed
# as root or do we need to modify this build script too?
runcmd ./build-fsl-strace
runcmd cd ..

# Building syscall-replayer
runcmd cd trace2model/syscall-replayer/
runcmd make -j"${numberOfCores}"
runcmd cd ../../..
