## Notes for PnetCDF developers
---

### Tasks immediately before a new release (must run in the following order)

 1. Sync with NetCDF4 header file, `netcdf.h`, for defined constants and error
    codes.  PnetCDF uses the same constants and error codes as NetCDF. See
    "Conform with netCDF" below.

 2. Set the release version
    In file `configure.ac`, variable `$PNETCDF_VERSION` indicates the version.
    It is automatically generated from the 2nd argument of `AC_INIT` call.
    Revise that string to the right release version. For example,
    ```
      AC_INIT([pnetcdf],[1.11.0],[parallel-netcdf@mcs.anl.gov])
                         ^^^^^
    ```
    For setting shared library version (ABI versioning), see below.

 3. Update file `RELEASE_NOTES`
    Copy the contents of file `sneak_peek.md` to the top of file `RELEASE_NOTES`.
    Clear up file `sneak_peek.md` (reset all items to none).
    + 1.9.0 and later - the release version and date in `RELEASE_NOTES` will
      be automatically set when running command `make dist`.
    + 1.8.1 and priors - add the release version and date at the top
      of file `RELEASE_NOTES`.

 4. Update the release date (1.8.1 and priors)
    For more info, see below section "Setting PnetCDF software release date".
    Run command `svn commit` to get the svn property \$LastChangedDate\$
    updated in file `configure.in`. That string will be used as the official
    release date.
    ```
    svn commit -m "set release date of version 1.6.1 to today"
    ```

 5. Commit all changes to repo servers
    * 1.8.1 and priors -- run `svn commit` to upload changes to SVN server.
    * 1.9.0 and after -- run `git push origin master` to upload changes to
      github.com.

 6. Generate release tar ball (use it for testing)
    * Generate a new `configure` file
      + Run command `autoreconf` to generate file `configure` to be included
        in the release (but ignored by the SVN/Git repo).
      + Must use the bug-fixed autotools of version 2.69 or higher to run
        `autoreconf`. See [README.Fujitsu](doc/README.Fujitsu) for the bug in
        autoconf 2.69 and patch.
    * Create all Makefiles
      + Run command `./configure` to create all Makefiles, so in the next step
        we can run command `make dist` to create the tar balls.
    * Create tar ball
      + Run command `make dist` to produce the tar ball for PnetCDF release,
        such as `pnetcdf-1.11.0.tar.gz`.  (Version 1.9.0 and later will only
        create .gz file.)
      + Starting from 1.9.0, the release date will be the date running command
        `make dist`. This setting will be done automatically, unlike step 4
        above that manually update the release date.

 7. Test the new release tar ball (not source codes from the repo)
    * build under the same directory as source
      - run `configure` (with command-line options: `--enable-strict`,
        `--enable-coverage`, `--disable-cxx`, `--disable-fortran` and their
        combinations)
      - For C only, run address sanitizer build (gcc and clang) by adding
        ```
        --disable-fortran \
        CFLAGS="-O0 -Wall -fsanitize=address -fno-omit-frame-pointer" \
        CXXFLAGS="-O0 -Wall -fsanitize=address -fno-omit-frame-pointer"
        ```
      - run `make check`
      - test with valgrind to check memory leak.
        * when built with static libraries (default), use commands:
            ```
            make check \
            TESTSEQRUN="valgrind --quiet --leak-check=full" \
            TESTMPIRUN="mpiexec -n NP valgrind --quiet --leak-check=full"
            ```
        * when built with --enable-shared, use commands:
            ```
            make check \
            TESTSEQRUN="libtool --mode=execute valgrind --quiet --leak-check=full" \
            TESTMPIRUN="mpiexec -n NP libtool --mode=execute valgrind --quiet --leak-check=full"
          ```
      - run `make ptests` to test programs in parallel runs
      - run `make ptests` with valgrind, by setting TESTSEQRUN and TESTMPIRUN
        as described above.
      - run `make distcheck`
        ```
        make -s V=1 LIBTOOLFLAGS=--silent distcheck \
                    DISTCHECK_CONFIGURE_FLAGS="--silent --enable-shared"
        ```
      - check Building Binary Packages Using DESTDIR (automake chapter 2.2.10)
        ```
        make install DESTDIR=$HOME/inst prefix=
        ```
        This creates a folder under $HOME/inst without prefix.
        The purpose of using DESTDIR is to install a library in a temporary
        folder, so they can be tared and copied to multiple hosts (i.e. install
        the same copy of library on different machines without running make
        install each there.)

      - test if file system type prefix added to file name is acceptable.
        For example, "ufs:" added to file name. Note this ROMIO convention may
        not be portable (probably only MPICH, will not work for OpenMPI), so
        it is not added to the regular test programs. Try
        ```
        make check TESTOUTDIR="ufs:."
        ```
    * build and test on different platforms with various compilers:
      - login.mcs.anl.gov : Intel, NAG Fortran, PGI, Solaris Studio compilers
      - cori.nersc.gov : Cray, Intel compilers
      - mira.alcf.anl.gov : IBM BGQ, XL compilers, cross-compiling, Big Endian
      - k.aics.riken.jp : Fujitsu compilers, cross-compiling, Big Endian

    * build benchmarks/FLASH-IO separately from PnetCDF (it has its own build
      script, i.e. configure.ac)

    * build NetCDF4 latest release with --enable-pnetcdf using this newly build
      of PnetCDF

    * Folder test/test_installed contains makefile and run scripts for a subset
      of test and example programs that can be run on a parallel computer with
      job submission system. See test/test_installed/README.md for compile and
      run instructions. Remember to add new test/example programs added in the
      new release.

 8. Create a checkpoint
    * For 1.9.0 and priors - Create a new SVN tag on svn repo, by running
      command below to duplicate the current trunk to a new tag:
      ```
      svn copy https://svn.mcs.anl.gov/repos/parallel-netcdf/trunk \
               https://svn.mcs.anl.gov/repos/parallel-netcdf/tags/v1-8-1 \
               -m "Tagging release version 1.8.1"
      ```
    * For 1.10.0 and later - Create a new tag on GitHub, by running command
      below to duplicate the current master branch to a new tag:
      ```
      git tag -a checkpoint.1.11.0 -m "Checkpoint right before 1.11.0 release"
      git push origin checkpoint.1.11.0
      ```
 9. Generate SHA1 checksums
    * Run command:
      ```
      openssl sha1 pnetcdf-1.11.0.tar.gz`
      ```
    * Example command-line output:
      ```
      SHA1(pnetcdf-1.11.0.tar.gz)= 495d42f0a41abbd09d276262dce0f7c1c535968a
      ```
10. Update PnetCDF Web Page
    * https://github.com/Parallel-NetCDF/Parallel-NetCDF.github.io
    * Create a new file of release note Parallel-NetCDF.github.io/Release_notes/1.11.0.md.
    * Add a news item in index.html to announce the new release version.
    * Copy the release tar ball into folder Release.
    * Add a new item to wiki/Download.html with link to tar ball, release note, tar ball size, and SHA1 checksum.

---
### Convention of setting version numbers
* For software release versioning
  * See http://semver.org/
    * Given a version number MAJOR.MINOR.PATCH, increment the:
      1. MAJOR version when you make incompatible API changes,
      2. MINOR version when you add functionality in a backwards-compatible
         manner, and
      3. PATCH version when you make backwards-compatible bug fixes.
    * Additional labels for pre-release and build metadata are available as
      extensions to the MAJOR.MINOR.PATCH format.

* For shared library versioning
  1. For libtool ABI versioning rules see:
     http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
  2. Update the version information only immediately before a public release.
  3. In configure.ac, change/set variable ABIVERSION to the new version.
  ```
   Here are a set of rules to help you update your library version information:
   1. Start with version information of '0:0:0' for each libtool library.
   2. Update the version information only immediately before a public release
      of your software. More frequent updates are unnecessary, and only
      guarantee that the current interface number gets larger faster.
   3. If the library source code has changed at all since the last update, then
      increment revision ('c:r:a' becomes 'c:r+1:a').
   4. If any interfaces have been added, removed, or changed since the last
      update, increment current, and set revision to 0.
   5. If any interfaces have been added since the last public release, then
      increment age.
   6. If any interfaces have been removed or changed since the last public
      release, then set age to 0.

   libtool Chapter 7.1 What are library interfaces?
      Interfaces for libraries may be any of the following (and more):
      global variables: both names and types
      global functions: argument types and number, return types, and function
      names standard input, standard output, standard error, and file formats
      sockets, pipes, and other inter-process communication protocol formats

      The following explanation may help to understand the above rules a bit
      better: consider that there are three possible kinds of reactions from
      users of your library to changes in a shared library:
      1. Programs using the previous version may use the new version as drop-in
         replacement, and programs using the new version can also work with the
         previous one. In other words, no recompiling nor relinking is needed.
         In this case, bump revision only, don't touch current nor age.
      2. Programs using the previous version may use the new version as drop-in
         replacement, but programs using the new version may use APIs not
         present in the previous one. In other words, a program linking against
         the new version may fail with 'unresolved symbols' if linking against
         the old version at runtime: set revision to 0, bump current and age.
      3. Programs may need to be changed, recompiled, and relinked in order to
         use the new version. Bump current, set revision and age to 0.
  ```
---
### Note on adding new MPI compiler candidates
 * In `configure.ac`, check the following variables
   * `CANDIDATE_MPICC`
   * `CANDIDATE_MPICXX`
   * `CANDIDATE_MPIF77`
   * `CANDIDATE_MPIF90`
 * To make configure command automatically detect MPI compilers,
   add new MPI compiler names to the list of the above variables.

---
### Note on autotools version used for software development
* When seeing the error message below when running command 'autoreconf -i',
  delete file 'm4/ltversion.m4' first. This is because ltversion.m4 is residue
  from a previous build using an earlier version of libtool.
* Starting from 1.9.0, in order to support building shared libraries,
  the following minimum versions are required.
  * autoconf 2.69
  * automake 1.15
  * libtool  2.4.6
* Even these fairly recent versions can still fail to build on some platforms.
  See Section "Hacking autoconf and automake" below for patches to
  customize autoconf and automake.
* Prior to 1.8.1, configure.in is developed based on autotools v2.59
  I, Wei-keng, tend to test it using v2.59, in case PnetCDF users have
  autotools as old as 2.59.
* However, 2.59 could generate a buggy configure file and failed on processing
  libraries from linker command line, for example when running on Carver
  at NERSC
* For official release of PnetCDF, I use autotools 2.69 with the patch for
  Fujitsu compilers to generate file "configure". See README.Fujitsu for the
  bug in autoconf 2.69 and patch.

---
### Working on configure.in or configure.ac
* Debugging: add "pnc_ac_debug=yes" to the configure command line to print
  debugging messages on screen.

---
### Conform with netCDF
 * PnetCDF uses the same following constants as netCDF
   * data types: nc_type
   * file open/create modes
   * error codes
   * in principle, all constants should conform with NetCDF

 * Make sure file pnetcdf.h.in is updated with the latest netCDF header, by
   building the latest netCDF with --enable-pnetcdf option.

 * Similarly, check Fortran error codes defined as parameters in
   ```
     src/libf/pnetcdf.inc.in           or src/binding/f77/pnetcdf.inc.in
     src/libf90/nf90_constants.f90     or src/binding/f90/nf90_constants.fh
     src/libf90/nfmpi_constants.f90.in or src/binding/f90/nfmpi_constants.fh.in
   ```
 * PnetCDF error codes start from -201
 * PnetCDF error codes for inconsistent header data start from -250
 * PnetCDF developers are reminded to periodically sync the followings with
   netCDF release.
   1. Error codes (pnetcdf.h v.s. netcdf.h)
   2. Error strings (error.c v.s. netcdf-c/libdispatch/derror.c)
   3. `src/drivers/include/utf8proc.h     v.s. netcdf-c/libdispatch/utf8proc.h`
   4. `src/drivers/common/utf8proc.c      v.s. netcdf-c/libdispatch/utf8proc.c`
   5. `src/drivers/common/utf8proc_data.h v.s. netcdf-c/libdispatch/utf8proc_data.h`

 * utf8proc URL: https://github.com/JuliaLang/utf8proc

---
### Note on netCDF text APIs and variables of external data type NC_CHAR
* All netCDF external data types are considered numerical data types, except
  for NC_CHAR. Numerical data types can be converted to different numerical
  data types. However, no numerical datatype is allowed to converted to NC_CHAR
  and vice versa. Given these limitations, note the followings.

* For attribute APIs, only text API, ncmpi_put_att_text(), can create/write
  attributes in NC_CHAR data type. Note that ncmpi_put_att_text() does not take
  an argument of external data type like other attribute APIs, because the
  attribute to be created/written will be of NC_CHAR type. For reading, only
  text API, ncmpi_get_att_text(), can read attributes of NC_CHAR type from
  file, otherwise NC_ECHAR error code will return.  Non-text APIs are not
  allowed to put/get attributes of NC_CHAR type.

* For variable get/put APIs, only text APIs, for example nc_put_vara_text(),
  can read/write a variable defined as NC_CHAR type. Trying to use non-text
  APIs to read/write a NC_CHAR variable will result in NC_ECHAR error code
  returned.

* There is no NC_ERANGE error code possibly returned from text APIs.

* In netCDF, NC_CHAR is designed purely for storing text data.  NC_CHAR is
  considered by netCDF as unsigned 8-bit integer, but not for used to store a
  numerical value.  Others netCDF external numerical data types and their
  numerical meanings:
  ```
  NC_BYTE   is considered as a   signed 1-byte integer
  NC_UBYTE  is considered as a unsigned 1-byte integer
  NC_SHORT  is considered as a   signed 2-byte integer
  NC_USHORT is considered as a unsigned 2-byte integer
  NC_INT    is considered as a   signed 4-byte integer
  NC_UINT   is considered as a unsigned 4-byte integer
  NC_INT64  is considered as a   signed 8-byte integer
  NC_UINT64 is considered as a unsigned 8-byte integer
  NC_FLOAT  is considered as a   signed 4-byte floating point
  NC_DOUBLE is considered as a   signed 8-byte double precision floating point
  ```
  All external data types, their byte sizes, sign-ness, and numerical ranges
  are independent from the systems running PnetCDF/netCDF.

---
### config.guess, config.sub, install-sh in directory scripts
Copy config.guess, config.sub, and install-sh from GNU libtool.
http://www.gnu.org/software/libtool/

Or get the latest version by running the following commands.
```
wget -O config.guess 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'

wget -O config.sub 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
```

---
### Extra files used for idea development, not for release
This file "DEVELOPER_NOTES".

src/lib (1.8.1 and prior) src/drivers/ncmpio/Legacy (1.9.0 and later):
```
    varnx.m4
    varnx_api_header.m4
    varn_api_header.m4
    swap.c
```
src/libf (1.8.1 and prior) src/binding/f77 (1.9.0 and later):
```
    varn_external.m4
    varnx_external.m4
    createffiles
```
src/libf90 (1.8.1 and prior) src/binding/f90 (1.9.0 and later):
```
    varn_interface.m4
    varnx_interface.m4
```

---
### Note on adding a new configure command-line option
 * Files need updated
   * `configire.ac` -- The AC_OUTPUT section, add "enabled" on screen if the
     new feature is enabled. No output when disabled. For example,
     ```
        if test "x${enable_netcdf4}" = xyes; then
           echo "\
                      NetCDF-4 support                            - enabled"
        fi
     ```
   * `src/utils/pnetcdf-config.in` -- add similar outputs as in `configure.ac`.
     * Note unlike `configure.ac`, in this file the feature must show either
       "enabled" or "disabled".
   * `src/include/pnetcdf.h.in` -- essential configure-time options are now
     explicitly set in the header file
   * `src/binding/f77/pnetcdf.inc.in` -- similar to pnetcdf.h.in
   * `src/binding/f90/nfmpi_constants.fh.in` -- similar to pnetcdf.h.in
     * These parameters must be public in Fortran 90.

---
### Note on creating new PnetCDF APIs
If a new PnetCDF API is created, please check and do the followings.
1. In src/lib (1.8.1 and prior) src/include (1.9.0 and later)
   * add the declaration of new APIs in "pnetcdf.h.in"
     For example,
     ```
     int ncmpi_inq_num_rec_vars(int ncid, int *nump);
     ```
   * If the new APIs have Fortran counterparts, their C declarations should
     be placed outside of the comment block marked with:
     ```
     /* Begin Skip Prototypes for Fortran binding */
     and
     /* End Skip Prototypes for Fortran binding */
     ```
     Otherwise, their declarations should be inside of the block.

2. In src/libf (1.8.1 and prior) src/binding/f77 (1.9.0 and later)
   * The new APIs should be declared in "pnetcdf.inc.in"
   * add their C-to-Fortran file name in "Makefile.in" ((1.8.1 and prior)
     For example, API nfmpi_put_att() corresponds to file put_attf.c
   * Some APIs have arguments that need special treatment in file "defs"
     for example, put_vard API's second argument varid, its Fortran value
     is C value + 1
     ```
        'put_vard' => '2',
                'put_vard-2'            => 'in:OffsetIndexIn',
     ```
3. In src/libf90 (1.8.1 and prior) src/binding/f90 (1.9.0 and later)
   * The new APIs should be declared in "visibility.f90" and/or "api.f90.in"
   * their definition should be added in the right files. For example,
     * nfmpi_inq_striping() is declared in api.f90.in
     * nf90mpi_inq_striping() is declared in visibility.f90
     * The definition of nf90mpi_inq_striping() is defined in file.f90 by
     calling nfmpi_inq_striping().

4. In src/libcxx (1.8.1 and prior) src/binding/cxx (1.9.0 and later)
   * The new APIs should be declared and added to the right files. For example,
     getRecVarCount() is declared in ncmpiGroup.h and defined in ncmpiGroup.cpp.

---
### Note on PnetCDF internal function and variable name convention

* internal vs. external data type
  * Internal data types refer to the data types of user's I/O buffers in memory.
  * External data types refer to the NC data type, i.e. nc_type, netCDF
    variable's data type stored in the netCDF files. External types are fixed,
    independent from the system platforms.

* Internal data types used in PnetCDF are: char, signed char, unsigned char,
  short, unsigned short, int, unsigned int, long, float, double, long long, and
  unsigned long long. Their corresponding function name substrings are "text",
  "schar", "uchar", "short", "ushort", "int", "uint", "long", "float",
  "double", "longlong", and "ulonglong".

* External data types are of nc_type. For CDF-1 and 2, they are NC_CHAR,
  NC_BYTE, NC_SHORT, NC_INT, NC_LONG (is aliased to NC_INT), NC_FLOAT, and
  NC_DOUBLE. For CDF-5, additional types are NC_UBYTE, NC_USHORT, NC_UINT,
  NC_INT64, and NC_UINT64.

* When creating an internal functions, after 1.7.0, we start to use names of
  nc_type to represent the external data types and the above convention for
  internal ones. For example, ncmpix_getn_NC_SHORT_longlong(). In this case,
  we can clearly see that this function is to read n elements of a netCDF
  variable of NC_SHORT external data type from a file to a user buffer of
  internal long long type.

* Function argument name convention: we use "xtype" for the argument of
  external data type and "itype" for internal data types. Thus, xtype must be
  of nc_type defined in pnetcdf.h and itype be either of an internal data type,
  or an MPI derived data type.

---
### Note on debugging
* Enable debugging option (--enable-debug) at the configure time can trace the
  usage of malloc and whether there is a malloc residue. All PnetCDF internal
  subroutines should call NCI_Malloc, NCI_Calloc, NCI_Realloc, and NCI_Free,
  instead of malloc, calloc, realloc, and free. When adding a new test or
  example program, please add a check for any malloc residue at the end. This
  is to make sure PnetCDF properly free up all malloc used internally. The
  code fragment is something like below.
```
   /* check if there is any PnetCDF internal malloc residue */
   MPI_Offset malloc_size, sum_size;
   int err = ncmpi_inq_malloc_size(&malloc_size);
   if (err == NC_NOERR) {
       MPI_Reduce(&malloc_size, &sum_size, 1, MPI_OFFSET, MPI_SUM, 0, MPI_COMM_WORLD);
       if (rank == 0 && sum_size > 0)
           printf("heap memory allocated by PnetCDF internally has %lld bytes yet to be freed\n",
                  sum_size);
   }
```

---
### Note on adding a new error code
PnetCDF error codes start at -200 and the error codes for header/argument
inconsistency of any kind start at -250.
1. If the new error code is not related to data inconsistency, add the code to
   the end before -250
2. If the new error code is related to data inconsistency, add it to the end
   of -250 section and make sure the number is between NC_EMULTIDEFINE_FIRST
   and NC_EMULTIDEFINE_LAST in pnetcdf.h.in. Update NC_EMULTIDEFINE_LAST if
   necessary.
3. Again, check Fortran error codes defined as parameters in
   ```
     src/libf/pnetcdf.inc.in            or src/binding/f77/pnetcdf.inc.in
     src/libf90/nf90_constants.f90      or src/binding/f90/nf90_constants.fh
     src/libf90/nfmpi_constants.f90.in  or src/binding/f90/nfmpi_constants.fh.in
   ```
4. The error message should be kept less than 80 characters, because this is
   the string length used in Fortran API nfmpi_strerror().

---
### Note on setting the last modify time of files to avoid rebuild
* Under directory src/utils/ncmpigen, the following files are pre-built and
  provided in the releases as is. They are no longer required to be built by
  PnetCDF users since 1.5.0.
  ```
    ncmpigenyy.c
    ncmpigentab.c
    ncmpigentab.h
  ```
  Their dependency are
  ```
    ncmpigenyy.c: ncmpigen.l
    ncmpigentab.c ncmpigentab.h: ncmpigen.y ncmpigenyy.c ncmpigen.h
  ```
* When their source files are modified, these files must be regenerated.
  However, when using command "svn commit" to commit the changes to svn repo,
  the modify time will be reset by svn, which may cause make to regenerate them
  again. To make the last modify time of these 3 files newer than their source
  files, one solution is to run two separate commit commands.
  * First, run "svn commit" for the source files.
  * Second, run "svn del" to delete these 3 files.
  * Third, run "svn add" to add these 3 files.
  * Fourth, run "svn commit" for these 3 files.

---
### Some tricks for wiki trac format
* Escape character is !
  * for example `ncmpi__enddef` will show underscore "enddef", so add ! before the
  double underscores to disable the formatting. eg. `ncmpi!__enddef`
* Example of using it on the command line:
  ```
  svn commit -m "add \!__func\!__ to error message" error.c error.h
  ```

---
### Trace MPI communication calls and I/O calls
* Add the following C define macros to the end of DEFS variable in macros.make to enable tracing.
  ```
    -DPNETCDF_TRACE_MPI_COMM
    -DPNETCDF_TRACE_MPI_IO
  ```
* For 1.9.0 and later, run the following commands to enable this feature.
  ```
    make clean
    make CFLAGS="-DPNETCDF_TRACE_MPI_COMM -DPNETCDF_TRACE_MPI_IO"
  ```
* Example outputs from stdout:
  ```
  TRACE-MPI-COMM: FILE mpincio.c FUNC ncmpiio_create() LINE 244 calling MPI_Bcast()
  TRACE-MPI-IO:   FILE mpincio.c FUNC ncmpiio_create() LINE 261 calling MPI_File_open()
  TRACE-MPI-COMM: FILE nc.c FUNC NC_begins() LINE 429 calling MPI_Allreduce()
  TRACE-MPI-COMM: FILE nc.c FUNC NC_check_header() LINE 111 calling MPI_Bcast()
  TRACE-MPI-COMM: FILE nc.c FUNC NC_check_header() LINE 119 calling MPI_Bcast()
  TRACE-MPI-COMM: FILE nc.c FUNC NC_check_header() LINE 130 calling MPI_Allreduce()
  TRACE-MPI-IO:   FILE nc.c FUNC write_NC() LINE 834 calling MPI_File_write_at()
  ```
* The program test/testcases/profile.c can be used to print the sequence of MPI
  calls for various PnetCDF calls. It can be helpful to find unnecessary MPI
  calls under safe or not-safe mode. For example, when safe mode is disable and
  running profile on one MPI process with option "-v", the stdout is shown below.
  ```
  0: ---- after ncmpi_create
  0: ---- before ncmpi_enddef() ----
  TRACE-MPI-COMM: FILE nc.c FUNC NC_begins() LINE 414 calling MPI_Bcast()
  TRACE-MPI-COMM: FILE nc.c FUNC NC_check_header() LINE 111 calling MPI_Bcast()
  TRACE-MPI-IO:   FILE nc.c FUNC write_NC() LINE 770 calling MPI_File_write_at()
  0: ---- before ncmpi_redef() ----
  0: ---- before ncmpi_enddef() ----
  TRACE-MPI-COMM: FILE nc.c FUNC NC_begins() LINE 414 calling MPI_Bcast()
  TRACE-MPI-COMM: FILE nc.c FUNC NC_check_header() LINE 111 calling MPI_Bcast()
  TRACE-MPI-IO:   FILE nc.c FUNC write_NC() LINE 770 calling MPI_File_write_at()
  0: ---- before ncmpi_iput_varn_int() ----
  0: ---- before ncmpi_iput_varn_int() ----
  0: ---- before ncmpi_iput_varn_int() ----
  0: ---- before ncmpi_wait_all() ----
  TRACE-MPI-COMM: FILE nonblocking.c FUNC ncmpii_wait() LINE 680 calling MPI_Allreduce()
  TRACE-MPI-IO:   FILE vard.c FUNC ncmpii_file_set_view() LINE 88 calling MPI_File_set_view()
  TRACE-MPI-IO:   FILE nonblocking.c FUNC ncmpii_mgetput() LINE 1899 calling MPI_File_write_at_all()
  TRACE-MPI-COMM: FILE nc.c FUNC ncmpii_sync_numrecs() LINE 619 calling MPI_Allreduce()
  0: ---- before ncmpi_iput_varn_int() ----
  0: ---- before ncmpi_iget_varn_int() ----
  0: ---- before ncmpi_iget_varn_int() ----
  0: ---- before ncmpi_iget_varn_int() ----
  0: ---- before ncmpi_wait_all() ----
  TRACE-MPI-COMM: FILE nonblocking.c FUNC ncmpii_wait() LINE 680 calling MPI_Allreduce()
  TRACE-MPI-IO:   FILE vard.c FUNC ncmpii_file_set_view() LINE 88 calling MPI_File_set_view()
  TRACE-MPI-IO:   FILE nonblocking.c FUNC ncmpii_mgetput() LINE 1899 calling MPI_File_write_at_all()
  TRACE-MPI-COMM: FILE nc.c FUNC ncmpii_sync_numrecs() LINE 619 calling MPI_Allreduce()
  TRACE-MPI-IO:   FILE vard.c FUNC ncmpii_file_set_view() LINE 88 calling MPI_File_set_view()
  TRACE-MPI-IO:   FILE nonblocking.c FUNC ncmpii_mgetput() LINE 1875 calling MPI_File_read_at_all()
  0: ---- before ncmpi_close() ----
  TRACE-MPI-IO:   FILE mpincio.c FUNC ncmpiio_close() LINE 410 calling MPI_File_close()
  ```

---
### Mirror SVN repo to Github (obsolete)
* PnetCDF svn repo is currently mirrored to GitHub daily in
  https://github.com/wkliao/parallel-netcdf
  (This repo is no longer maintained after the office repo migrated to github)

* Below are Commands for creating a mirror on GitHub:
  ```
  cd /path/to/your/git/clone/of/parallel-netcdf
  git svn init -s https://svn.mcs.anl.gov/repos/parallel-netcdf parallel-netcdf
  cd parallel-netcdf
  git svn fetch >& fetch.log &     # this will take long to get all commits
  ```
* Log into your github account and create a new repo, named "parallel-netcdf"
  Set the remote repo URL @ Github
  ```
  git remote add origin https://github.com/wkliao/parallel-netcdf.git
  git push -u origin master
  ```
* Create a cron job to sync svn repo to Github using a script file named, say
  cron_sync.sh. Its contents are:
  ```
  #!/bin/bash
  cd /path/to/your/git/clone/of/parallel-netcdf
  git svn rebase
  git push origin master
  ```
* Push a new PnetCDF tag branch to github:
  ```
  git checkout -b 1-8-0pre1 remotes/origin/tags/v1-8-0pre1
  git push -u origin 1-8-0pre1
  ```

---
### Setting up Travis CI for PnetCDF @ Github
* Instructions are based on https://docs.travis-ci.com/user/getting-started/
  1. Sign in to Travis CI with your GitHub account
  2. Click (Your Name) -> Accounts -> Sync account (This should show a new repo just created.)
  3. Click the repo to enable Travis CI and configure "settings" tab

* Travis only runs a build on the commits you push after adding the repository
  to Travis. Note: If your project already has a .travis.yml file, you need to
  push another commit to trigger a build.

---
### Coverity Scan + Travis CI Integration
* Following the instructions in https://scan.coverity.com/travis_ci
* The modeling file ./coverity_model.c is for Coverity Scan used in the Analysis Setting.
* Change the branch for scan: Under "coverity_scan:". change "branch_pattern:" to the branch name

---
### Testing with valgrind
* Valgrind checks memory leak (e.g. command: valgrind --leak-check=full -q),
  including uninitialized I/O buffer used in MPI_File_write. This warning message
  can appear when PnetCDF detects out-of-range data type conversion, error code
  NC_ERANGE. Note that PnetCDF continues writing the data out even if only
  partial of the user buffer causes NC_ERANGE. In the internal buffer allocated
  for type conversion, the buffer locations corresponding to those out-of-range
  data elements will not be initialized. Therefore, you should expect to see many
  warning messages of this kind when running "make check", as our test programs
  intentionally test whether NC_ERANGE can be returned properly.

* Another memory leak reported by valgrind is when file header expands causing
  data section to be moved to later file offsets. PnetCDF calls MPI read and
  write to move a variable at a time. The ideal case is to write the actual read
  size reported by MPI_Get_count in the MPI write call. This is in case some
  variables are defined but never been written. However, in some MPICH versions
  MPI_Get_count fails to report the correct value due to an internal error that
  fails to initialize the MPI_Status object. Therefore, the solution can be
  either to explicitly initialize the status object to zeros, or to just use the
  same read request amount for write. Note that the latter will write the
  variables that have not been written before. For now we adopt the former
  option. See comments about MPI_Get_count() in function move_file_block() of
  src/drivers/ncmpio/ncmpio_enddef.c.c

* When data sieving is enabled in MPI-IO (default in ROMIO), its
  read-modify-write operation can also cause valgrind to complain uninitialized
  I/O buffer. In particular, when a new file is created and PnetCDF fixed-sized
  variable alignment is enabled, the read in read-modify-write may read no data
  at all, resulting data sieving buffer containing uninitialized data. To
  eliminate this possibility, set the hint environment variable to disable
  alignment, e.g. PNETCDF_HINTS="nc_var_align_size=1;nc_header_align_size=1".
  Another scenario is when writing more than one record of a record variable in
  a single put call. Uninitialized bytes will be the records of other variables
  sitting in between. Disable data sieving can silence the valgrind message.
  i,e. PNETCDF_HINTS="romio_lustre_ds_in_coll=disable" or
  PNETCDF_HINTS="romio_ds_write=disable".

* When using MPICH 3.2 with the bug of #2332 fixed, running "make check" and
  "make ptests" through valgrind should run without any complains. See MPICH
  ticket #2332 in https://trac.mpich.org/projects/mpich/ticket/2332 or
  https://github.com/pmodels/mpich/issues/2332

---
### Note on using clang and gprof together
```
% clang --version
clang version 3.4.2 (tags/RELEASE_34/dot2-final)
Target: x86_64-redhat-linux-gnu
Thread model: posix

% gprof --version
GNU gprof version 2.20.51.0.2-5.43.el6 20100205
Based on BSD gprof, copyright 1983 Regents of the University of California.
This program is free software.  This program has absolutely no warranty.

The problem is reported in https://llvm.org/bugs/show_bug.cgi?id=14713
```
* When compiling PnetCDF using clang based MPI compiler with compile option
  "-g -O2 -pg", programs will receive signal SIGSEGV, Segmentation fault.
  However, when compiled with -g -O0 -pg, programs receive no error.

* Workaround: use "-pg -mno-omit-leaf-frame-pointer -fno-omit-frame-pointer"

---
### Note on including config.h
* When doing VPATH build, remember to pass the C compiler a -I. option. Even if
  you use #include "config.h", the preprocessor searches only the source
  directory, not the build directory. Thus, we should use #include <config.h>
  instead of #include "config.h". In addition, use -I. -I$(srcdir) in
  Makefile.in.

* Autoconf manual suggests it is a good habit to use angle brackets, because in
  the rare case when the source directory contains another config.h, the build
  directory should be searched first.

---
### Note on using NAG Fortran compiler
* Type conversion that causes NC_ERANGE error will also cause a coredump with
  an error  message of "Arithmetic exception". See discussion threads in
  http://www.unidata.ucar.edu/support/help/MailArchives/netcdf/msg13327.html
  The solution is to enable "erange-fill" option at the configure time (default
  in PnetCDF).  The implementation of this option is also provided to netCDF,
  but need manually activate. See
  https://github.com/Unidata/netcdf-c/pull/319#issuecomment-256982471

---
### Note on M4 flags
* M4 has a nice feature called synclines that adds line numbers into m4 files
  so compilers can report the error locations in the m4 files, instead of the
  derived C/Fortran files. To enable this feature, developers add M4FLAGS=-s to
  the configure command line. Note that synclines currently only take effect for
  C files. There is still some issues needed to be resolved for Fortran files.
  Developers are also warned that when m4 macro functions are used, the line
  numbers reported are the locations the functions are invoked, not the lines
  inside the functions.

---
### NC error code precedence
* The precedence of reporting error codes depends on the seriousness of the
  error. The guide line is the following. The most serious ones are related to
  ncid, such as NC_EBADID, NC_EPERM, and NC_EINDEFINE. The next is related to
  varid, such as NC_ENOTVAR. These two types of errors will make PnetCDF programs
  difficult to continue, even if we modified the request to NULL for processes
  that produce an error.
* For put att APIs:
  ```
    NC_EBADID, NC_EPERM, NC_ENOTVAR, NC_EBADNAME, NC_EBADTYPE, NC_ECHAR,
    NC_EINVAL, NC_ENOTINDEFINE, NC_ERANGE
  ```
* For get att APIs:
  ```
    NC_EBADID, NC_ENOTVAR, NC_EBADNAME, NC_ENOTATT, NC_ECHAR, NC_EINVAL,
    NC_ERANGE
  ```
* For put/get variable APIs:
  ```
    NC_EBADID, NC_EPERM, NC_EINDEFINE, NC_ENOTVAR, NC_ECHAR, NC_EINVALCOORDS,
    NC_EEDGE, NC_ESTRIDE, NC_EINVAL, NC_ERANGE
  ```

---
### Setting PnetCDF software release date
* Prior to version 1.8.1, the release date was obtained from the SVN keyword
  LastChangedDate set in file configure.in. It is used to produce two
  variables: PNETCDF_RELEASE_DATE and PNETCDF_RELEASE_DATE_FULL. These two
  variables are used by all man pages, pnetcdf.h, pnetcdf_version, and
  pnetcdf-config.  We used the keyword value set by SVN as the release date.
  The assumption is that updating PACKAGE_VERSION in configure.in is the last
  step before making a release, i.e.  the last modification date of file
  configure.in is the latest among all files.  This approach can cause a
  problem when using git-svn to clone the source codes, as one must set git
  smudge/clean filter to produce the effect of SVN keywords.  In addition, it
  makes more sense to use the date when running command "make dist" to create
  the tar ball.

* Staring from 1.9.0, the release date will be set to the date when running
  command "make dist". See the makefile target "dist-hook" in
  ./src/include/Makefile.am for an approach using command sed to do string
  substitution for DIST_DATE.

---
### Hacking autoconf and automake
* Autoconf 2.69 has a problem of checking the version of the Fujitsu Fortran
  compiler, which causes errors for building PnetCDF at the configure time.

* The fix and patch have been discussed and provided in the link below. PnetCDF
  users/developers are encouraged to apply to patch to their autoconf utility.

* Fujitsu compilers
  ```
  Subject: Support Fujitsu in _AC_PROG_FC_V
  https://lists.gnu.org/archive/html/autoconf-patches/2013-11/msg00001.html

  diff -u Origin/autoconf-2.69/lib/autoconf/fortran.m4 autoconf-2.69/lib/autoconf/fortran.m4
  --- Origin/autoconf-2.69/lib/autoconf/fortran.m4        2012-03-07 11:35:25.000000000 -0600
  +++ autoconf-2.69/lib/autoconf/fortran.m4        2017-05-08 12:23:15.110473352 -0500
  @@ -578,6 +578,8 @@
   # information of library and object files (normally -v)
   # Needed for _AC_FC_LIBRARY_FLAGS
   # Some compilers don't accept -v (Lahey: (-)-verbose, xlf: -V, Fujitsu: -###)
  +# Fujitsu accepts --verbose and passes it to the linker, which doesn't yield
  +# the desired result. Therefore test for -### before testing for --verbose.
   AC_DEFUN([_AC_PROG_FC_V],
   [_AC_FORTRAN_ASSERT()dnl
   AC_CACHE_CHECK([how to get verbose linking output from $[]_AC_FC[]],
  @@ -585,7 +587,7 @@
   [AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
   [ac_cv_prog_[]_AC_LANG_ABBREV[]_v=
   # Try some options frequently used verbose output
  -for ac_verb in -v -verbose --verbose -V -\#\#\#; do
  +for ac_verb in -v -verbose -V -\#\#\# --verbose; do
     _AC_PROG_FC_V_OUTPUT($ac_verb)
     # look for -l* and *.a constructs in the output
     for ac_arg in $ac_[]_AC_LANG_ABBREV[]_v_output; do
  ```

---
### Fortran preprocessor flag
* On some systems, the Fortran preprocessor flag is not "-D". For example, IBM
  XL fortran compilers use flag "-WF,-D". However, autoconf 2.69 hard-codes
  "-DHAVE_CONFIG_H" into pre-defined variable "DEFS" which is used in C, C++,
  F77, and F90 compile commands. The hack below adds a pre-define variable,
  FC_DEFS, and replaces DEFS with it for Fortran-based compile commands. Thus,
  in Makefile.am, use ${FC_DEFINE} instead of -D when setting FCFLAGS/FFLAGS.
  See an example in test/common/Makefile.am.
  ```
  diff -u Origin/autoconf-2.69/lib/autoconf/general.m4 autoconf-2.69/lib/autoconf/general.m4
  --- Origin/autoconf-2.69/lib/autoconf/general.m4        2012-04-24 21:37:26.000000000 -0500
  +++ autoconf-2.69/lib/autoconf/general.m4        2017-05-08 10:33:20.396072040 -0500
  @@ -1408,6 +1408,7 @@
   dnl
   dnl Substitute for predefined variables.
   AC_SUBST([DEFS])dnl
  +AC_SUBST([FC_DEFS])dnl
   AC_SUBST([ECHO_C])dnl
   AC_SUBST([ECHO_N])dnl
   AC_SUBST([ECHO_T])dnl
  ```
  ```
  diff -u Origin/autoconf-2.69/lib/autoconf/status.m4 autoconf-2.69/lib/autoconf/status.m4
  --- Origin/autoconf-2.69/lib/autoconf/status.m4        2012-04-24 21:37:26.000000000 -0500
  +++ autoconf-2.69/lib/autoconf/status.m4        2017-05-08 18:49:02.966611595 -0500
  @@ -1266,6 +1266,12 @@
   test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

   m4_ifdef([_AC_SEEN_CONFIG(HEADERS)], [DEFS=-DHAVE_CONFIG_H], [AC_OUTPUT_MAKE_DEFS()])
  +if test "x$FC_DEFINE" != "x-D" -a "x$FC_DEFINE" != x ; then
  +   ac_fc_define_sed_str="s/-D/${FC_DEFINE}/g"
  +   FC_DEFS=`echo $DEFS | sed $ac_fc_define_sed_str`
  +else
  +   FC_DEFS=$DEFS
  +fi

   dnl Commands to run before creating config.status.
   AC_OUTPUT_COMMANDS_PRE()dnl
  ```
  ```
  diff -u Origin/automake-1.15/bin/automake.in automake-1.15/bin/automake.in
  --- Origin/automake-1.15/bin/automake.in        2015-01-05 13:25:55.000000000 -0600
  +++ automake-1.15/bin/automake.in        2017-05-08 01:01:05.098144772 -0500
  @@ -622,6 +622,15 @@
       $(CPPFLAGS)
     };

  +my @fpplike_flags =
  +  qw{
  +    $(FC_DEFS)
  +    $(DEFAULT_INCLUDES)
  +    $(INCLUDES)
  +    $(AM_CPPFLAGS)
  +    $(CPPFLAGS)
  +  };
  +
   # C.
   register_language ('name' => 'c',
                      'Name' => 'C',
  @@ -876,7 +885,7 @@
                      'flags' => ['FCFLAGS', 'CPPFLAGS'],
                      'ccer' => 'PPFC',
                      'compiler' => 'PPFCCOMPILE',
  -                   'compile' => "\$(FC) @cpplike_flags \$(AM_FCFLAGS) \$(FCFLAGS)",
  +                   'compile' => "\$(FC) @fpplike_flags \$(AM_FCFLAGS) \$(FCFLAGS)",
                      'compile_flag' => '-c',
                      'output_flag' => '-o',
                      'libtool_tag' => 'FC',
  @@ -908,7 +917,7 @@
                      'flags' => ['FFLAGS', 'CPPFLAGS'],
                      'ccer' => 'PPF77',
                      'compiler' => 'PPF77COMPILE',
  -                   'compile' => "\$(F77) @cpplike_flags \$(AM_FFLAGS) \$(FFLAGS)",
  +                   'compile' => "\$(F77) @fpplike_flags \$(AM_FFLAGS) \$(FFLAGS)",
                      'compile_flag' => '-c',
                      'output_flag' => '-o',
                      'libtool_tag' => 'F77',
  ```

---
### Note on burst buffer driver
* Burst buffer driver relies on ncmpio driver to write/read NetCDF files, as it
  uses subroutines of ncmpio driver internally. When the structure of
  PNC_Driver is modified, not only the interface of burst buffer driver needs
  to be updated but also its internal ncmpio driver related codes.

* A utility program named ncmpilogdump has been developed to print on screen
  the contents of log files stored in the burst buffer.

* Known issues:
  1. vard API is not supported. It will be carried out by blocking vard APIs
     through the ncmpio driver directly.

---

