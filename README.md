fsl-strace - an strace fork compatible with DataSeries
======================================================

fsl-strace extends strace with support for [DataSeries](https://github.com/dataseries/dataseries), an efficient, flexible data format for structured serial data. This modification to strace captures maximal information, including all data buffers and arguments. The output of fsl-strace is designed to be readable by both humans and machines, allowing researchers to use existing DataSeries tools to analyze trace files.

This modification is designed for use with our [syscall-replayer](https://github.com/sbu-fsl/trace2model/tree/master/syscall-replayer), a program able to replay and analyze the system call traces collected in the DataSeries format. For more information on the Re-Animator project, please see our paper [Re-Animator: Versatile High-Fidelity Storage-System Tracing and Replaying](https://doi.org/10.1145/3383669.3398276).

fsl-strace is under development by Ibrahim Umit Akgun of the File Systems and Storage Lab (FSL) at Stony Brook University under Professor Erez Zadok, with assistance from Professor Geoff Kuenning at Harvey Mudd College.

Dependencies
------------

Currently, only Ubuntu 16 is officially supported.

- [Lintel](https://github.com/dataseries/lintel) - general utility library for DataSeries
- [DataSeries](https://github.com/dataseries/dataseries) - data format for structured serial data
- [strace2ds-library](https://github.com/sbu-fsl/trace2model/tree/master/strace2ds-library) - library for outputting traces in DataSeries format
- [tcmalloc](https://github.com/gperftools/gperftools) - high-performance, multi-threaded `malloc()` implementation
- libtool
- libboost-dev (v1.58 only)
- libboost-thread-dev (v1.58 only)
- libboost-program-options-dev (v1.58 only)
- build-essential
- libxml2-dev
- zlib1g-dev

Build Instructions
------------------

### Automated Build

Requires bash.

1. Install the following required programs and libraries:

    ```plaintext
    git cmake perl autoconf automake gcc g++ libtool libboost-dev libboost-thread-dev libboost-program-options-dev build-essential libxml2-dev zlib1g-dev
    ```

    On Ubuntu 16, all the above requirements are available through the APT package manager.

2. Clone this repository and run [`build-fsl-strace.sh`](build-fsl-strace.sh). This will place build files in the current directory under `BUILD/` and install Lintel, DataSeries, tcmalloc, strace2ds-library, and fsl-strace under `dist/fsl_strace_release/`.

    - The script will install include files and libraries under `/usr/local/` if invoked with `build-syscall-replayer.sh --install`. If installed this way, the fsl-strace binary will remain in the build folder so as not to conflict with the strace binary pre-installed on many systems.
    - You may specify custom build and install directories with the command line options `--build-dir DIR` and `--install-dir DIR`. Run `./build-fsl-strace.sh --help` for a full list of options.

### Manual Build

1. Install the following required programs and libraries:

    ```plaintext
    git cmake perl autoconf automake gcc g++ libtool libboost-dev libboost-thread-dev libboost-program-options-dev build-essential libxml2-dev zlib1g-dev
    ```

    On Ubuntu 16, all the above requirements are available through the APT package manager.

2. Install [Lintel](https://github.com/dataseries/lintel) by running `cmake . && make && make install` at the root of the Lintel repository.

3. Install [DataSeries](https://github.com/dataseries/dataseries) by running `cmake . && make && make install` at the root of the DataSeries repository.

4. Install [tcmalloc](https://github.com/gperftools/gperftools) from the gperftools repository. See the gperftools [`INSTALL`](https://github.com/gperftools/gperftools/blob/master/INSTALL) file for detailed instructions.

5. Install [strace2ds-library](https://github.com/sbu-fsl/trace2model/tree/master/strace2ds-library). See the strace2ds-library [`README.md`](https://github.com/sbu-fsl/trace2model/blob/master/strace2ds-library/README.md) file for detailed instructions.

6. In the fsl-strace repository, run the `./bootstrap` script as a wrapper to autotools.

7. Create a directory named `BUILD` in the repository and navigate to it. Run the command

    ```bash
    ../configure --enable-mpers=check --enable-dataseries
    ```

    If you have installed the other libraries in a nonstandard directory, you will need to change the `CPPFLAGS` and `LDFLAGS` environment variables before running `configure`. For example, if you have installed strace2ds-library in `$HOME/lib/strace2ds`, you will need to run

    ```bash
    CPPFLAGS="-I$HOME/lib/strace2ds/include" LDFLAGS="-Xlinker -rpath=$HOME/lib/strace2ds/lib -L$HOME/lib/strace2ds/lib" ../configure --enable-mpers=check --enable-dataseries
    ```

    For more information on `CPPFLAGS` and `LDFLAGS`, run `../configure --help`.

8. Run `make LIBS="-lstrace2ds -lLintel -lDataSeries" CCLD=g++` in the `BUILD` directory. This will build the modified strace binary and place it in the `BUILD` directory.

Usage
-----

To run the modified strace binary with DataSeries support, navigate to the build directory (or wherever the binary was installed) and run

```bash
./strace --dataseries output.ds <program to be traced>
```

This will write all trace output in DataSeries format to `output.ds`. From there, you can read the output in plaintext with `ds2txt output.ds`, analyze the trace with any of the tools provided with DataSeries, or replay the trace with [syscall-replayer](https://github.com/sbu-fsl/trace2model/tree/master/syscall-replayer).

---

Below is the original README file from the [strace](https://strace.io) project.

strace - the linux syscall tracer
=================================

This is [strace](https://strace.io) -- a diagnostic, debugging and instructional userspace utility with a traditional command-line interface for Linux.  It is used to monitor and tamper with interactions between processes and the Linux kernel, which include system calls, signal deliveries, and changes of process state.  The operation of strace is made possible by the kernel feature known as [ptrace](http://man7.org/linux/man-pages/man2/ptrace.2.html).

strace is released under the terms of [the GNU Lesser General Public License version 2.1 or later](LGPL-2.1-or-later); see the file [COPYING](COPYING) for details.
strace test suite is released under the terms of [the GNU General Public License version 2 or later](tests/GPL-2.0-or-later); see the file [tests/COPYING](tests/COPYING) for details.

See the file [NEWS](NEWS) for information on what has changed in recent versions.

Please read the file [INSTALL-git](INSTALL-git.md) for installation instructions.

Please take a look at [the guide for new contributors](https://strace.io/wiki/NewContributorGuide) if you want to get involved in strace development.

The user discussion and development of strace take place on [the strace mailing list](https://lists.strace.io/mailman/listinfo/strace-devel) -- everyone is welcome to post bug reports, feature requests, comments and patches to strace-devel@lists.strace.io.  The mailing list archives are available at https://lists.strace.io/pipermail/strace-devel/ and other archival sites.

The GIT repository of strace is available at [GitHub](https://github.com/strace/strace/) and [GitLab](https://gitlab.com/strace/strace/).

The latest binary strace packages are available in many repositories, including
[OBS](https://build.opensuse.org/package/show/home:ldv_alt/strace/),
[Fedora rawhide](https://apps.fedoraproject.org/packages/strace), and
[Sisyphus](https://packages.altlinux.org/en/Sisyphus/srpms/strace).

[![Build Status](https://travis-ci.org/strace/strace.svg?branch=master)](https://travis-ci.org/strace/strace) [![Code Coverage](https://codecov.io/github/strace/strace/coverage.svg?branch=master)](https://codecov.io/github/strace/strace?branch=master)
