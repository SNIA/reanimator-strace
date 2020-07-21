#!/bin/bash
#
# Build the program syscall-replayer, installing any required dependecies if
# requested.

# TODO: Can we avoid building tests? They slow down the script

# Script variables
readonly numberOfCores="$(nproc --all)"
install=false
installPackages=false
configArgs=""
buildDir="$(pwd)/syscall_replayer_release"

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

# TODO: Expand `printUsage` to use a here document
function printUsage
{
    echo "Usage: $0 [--install]" >&2
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
            # TODO: Should be able to set $buildDir to /usr/local to reduce
            # redundant code
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

if [[ "${install}" == true ]]; then
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
        echo "${program}: Not found. Queuing for installation..."
        missingPrograms+=("${program}")
    else
        echo "${program}: Not found."
        missingPrograms+=("${program}")
    fi
done

# Check whether the user has all required programs for building
if [[ "${#missingPrograms[@]}" -gt 0 ]]; then
    if [[ "${installPackages}" == true ]]; then
        echo "Installing missing programs."
        runcmd sudo apt-get install -y "${missingPrograms[*]}"
    else
        echo "Could not find all required programs. Not found:"
        for program in "${missingPrograms[@]}"; do
            echo "  ${program}"
        done

        echo "To install on a Debian-based system, run the command"
        echo "  sudo apt-get install ${missingPrograms[*]}"
        exit 1
    fi
fi

# # TODO: Either check the system for these libraries or trust the user
# # to install them before running the script
# if [[ "${install}" == true ]]; then
    
#     # Installing packages
#     runcmd sudo apt-get install -y libboost-dev libboost-thread-dev \
#         libboost-program-options-dev build-essential libxml2-dev libz-dev \
#         libaio-dev libtool
# fi

# Cloning all repositories
runcmd mkdir -p build
runcmd cd build
[[ -d "Lintel" ]] || runcmd git clone https://github.com/dataseries/Lintel.git
[[ -d "DataSeries" ]] || runcmd git clone https://github.com/dataseries/DataSeries.git
[[ -d "gperftools" ]] || runcmd git clone https://github.com/gperftools/gperftools.git
[[ -d "oneTBB" ]] || runcmd git clone https://github.com/oneapi-src/oneTBB.git
[[ -d "trace2model" ]] || runcmd git clone https://github.com/sbu-fsl/trace2model.git
[[ -d "fsl-strace" ]] || runcmd git clone https://github.com/sbu-fsl/fsl-strace.git

# Building Lintel
# TODO: Should each build be done in a subshell to avoid errors with cd?
runcmd cd Lintel
if [[ "${install}" == true ]]; then
    runcmd cmake .
    runcmd sudo make -j"${numberOfCores}" install
else
    runcmd cmake -DCMAKE_INSTALL_PREFIX="${buildDir}" .
    runcmd make -j"${numberOfCores}" install
fi
runcmd cd ..

# Building DataSeries
runcmd cd DataSeries
if [[ "${install}" == true ]]; then
    runcmd cmake .
    runcmd sudo make -j"${numberOfCores}" install
else
    runcmd cmake -DCMAKE_INSTALL_PREFIX="${buildDir}" .
    runcmd make -j"${numberOfCores}" install
fi
runcmd cd ..

# Building tcmalloc
runcmd cd gperftools
runcmd ./autogen.sh
if [[ "${install}" == true ]]; then
    runcmd ./configure "${configArgs}"
    runcmd make -j"${numberOfCores}"
    runcmd sudo make -j"${numberOfCores}" install
else
    runcmd ./configure --prefix="${buildDir}" "${configArgs}"
    runcmd make -j"${numberOfCores}"
    runcmd make -j"${numberOfCores}" install
fi
runcmd cd ..

# Building tbb
runcmd cd oneTBB
if [[ "${install}" == true ]]; then
    runcmd sudo make tbb_build_dir=/usr/local/lib tbb_build_prefix=one_tbb
    runcmd sudo cp -r ./include/. /usr/local/include
else
    runcmd make tbb_build_dir="${buildDir}/lib" tbb_build_prefix=one_tbb
    runcmd cp -r ./include/. "${buildDir}/include"
fi
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
if [[ "${install}" == true ]]; then
    runcmd ../configure --enable-shared --disable-static \
        --prefix=/usr/local/strace2ds
    runcmd make clean
    runcmd make -j"${numberOfCores}"
    runcmd sudo make -j"${numberOfCores}" install
else
    runcmd export CXXFLAGS="-I${buildDir}/include"
    runcmd export LDFLAGS="-L${buildDir}/lib"
    runcmd ../configure --enable-shared --disable-static \
        --prefix="${buildDir}/lib/strace2ds"
    runcmd make clean
    runcmd make -j"${numberOfCores}"
    runcmd make -j"${numberOfCores}" install
fi

runcmd cd ../../..

# Building fsl-strace
runcmd cd fsl-strace
runcmd ./bootstrap
runcmd mkdir -p BUILD
runcmd cd BUILD
if [[ "${install}" == true ]]; then
    runcmd export CPPFLAGS="-I/usr/local/strace2ds/include"
    runcmd export LDFLAGS="\
        -Xlinker -rpath=/usr/local/strace2ds/lib -L/usr/local/strace2ds/lib"
else
    runcmd export CPPFLAGS="-I${buildDir}/lib/strace2ds/include"
    runcmd export LDFLAGS="\
        -Xlinker -rpath=${buildDir}/lib:${buildDir}/lib/strace2ds/lib \
        -L${buildDir}/lib -L${buildDir}/lib/strace2ds/lib"
fi

libs="-lstrace2ds -lLintel -lDataSeries"
runcmd ../configure --enable-mpers=check --enable-dataseries

runcmd make clean

if [[ "${install}" == true ]]; then
    runcmd sudo make LIBS="${libs}" CCLD=g++
else
    runcmd make LIBS="${libs}" CCLD=g++
fi

runcmd cd ../..

# Building syscall-replayer
runcmd cd trace2model/syscall-replayer/
if [[ "${install}" == true ]]; then
    runcmd export CPPFLAGS="-I/usr/local/strace2ds/include"
    runcmd export LDFLAGS="\
        -Xlinker -rpath=/usr/local/strace2ds/lib:/usr/local/lib/one_tbb_release \
        -L/usr/local/strace2ds/lib -L/usr/local/lib/one_tbb_release"
    runcmd sudo make -j"${numberOfCores}"
else
    runcmd export CPPFLAGS="-I${buildDir}/lib/strace2ds/include \
        -I${buildDir}/include"
    runcmd export LDFLAGS="\
        -Xlinker -rpath=${buildDir}/lib:${buildDir}/lib/strace2ds/lib:${buildDir}/lib/one_tbb_release \
        -L${buildDir}/lib -L${buildDir}/lib/strace2ds/lib -L${buildDir}/lib/one_tbb_release"
    runcmd make -j"${numberOfCores}"
fi

runcmd cd ../../..
