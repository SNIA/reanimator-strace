#!/bin/bash
#
# Build the program reanimator-strace, installing any required dependencies if
# requested.

####################
# Script variables #
####################
readonly numberOfCores="$(nproc --all)"
install=false
installPackages=false
configArgs=""
installDir="$(pwd)/dist/reanimator_strace_release"
repositoryDir="$(pwd)/BUILD/repositories"
straceDir="$(pwd)"

readonly programDependencies=("autoconf" "automake" "cmake" "gcc" "g++" "perl" "git")
missingPrograms=()

####################
# Script functions #
####################
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
    (
    cat << EOF
Usage: $0 [options...]
Options:
     --build-dir DIR        Download repositories and place build files in DIR
     --config-args ARGS     Append ARGS to every ./configure command
     --install              Install libraries and binaries under /usr/local
     --install-dir DIR      Install libraries and binaries under DIR
     --install-packages     Automatically use apt-get to install missing packages
 -h, --help                 Print this help message
EOF
    ) >&2
    exit 0
}

##################
# Script startup #
##################

# Parse script arguments
while [[ $# -gt 0 ]]; do
    key="$1"

    case "${key}" in
        --build-dir)
            shift # past argument
            repositoryDir="$(realpath "$1" || exit $?)"
            shift # past value
            ;;
        --config-args)
            shift # past argument
            configArgs="$1"
            shift # past value
            ;;
        --install)
            install=true
            installDir="/usr/local"
            shift # past argument
            ;;
        --install-dir)
            shift # past argument
            installDir="$(realpath "$1" || exit $?)"
            shift # past value
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

# Check system for sudo command
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
        runcmd sudo apt-get install -y "${missingPrograms[@]}"
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

#################
# Build process #
#################

# Cloning all repositories
runcmd mkdir -p "${repositoryDir}"
runcmd cd "${repositoryDir}"
[[ -d "reanimator-library" ]] || runcmd git clone https://github.com/SNIA/reanimator-library.git

# Building reanimator-library
# ---------------------------
runcmd cd reanimator-library
runcmd chmod +x build-reanimator-library.sh
if $install; then
    runcmd ./build-reanimator-library.sh --install
else
    runcmd mkdir -p "${installDir}"
    runcmd ./build-reanimator-library.sh --install-dir "${installDir}"
fi
runcmd cd "${repositoryDir}"

# Building reanimator-strace
# --------------------------
runcmd cd "${straceDir}"
runcmd ./bootstrap
runcmd mkdir -p BUILD
runcmd cd BUILD
runcmd export CPPFLAGS="-I${installDir}/strace2ds/include"
runcmd export LDFLAGS="\
    -Xlinker -rpath=${installDir}/lib:${installDir}/strace2ds/lib \
    -L${installDir}/lib -L${installDir}/strace2ds/lib"
libs="-lstrace2ds -lLintel -lDataSeries"
runcmd ../configure --enable-mpers=check --enable-dataseries
runcmd make clean
runcmd make LIBS="${libs}" CCLD=g++
if [[ "${install}" == false ]]; then
    runcmd cp ./strace "${installDir}/bin/"
fi
runcmd cd "${repositoryDir}"
