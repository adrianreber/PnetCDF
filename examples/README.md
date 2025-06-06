This directory contains several example programs. Detailed descriptions of
each program, compile and run instructions, and sample outputs are provided
at the beginning of each file. Most of the example programs are developed
with C, C++, F77, and F90 versions.

* ./tutorial
  + A directory contains example programs that use different parallel I/O
    strategies, including:
    *   Parallel I/O from the master process
    *   Concurrent I/O on separate files
    *   Real parallel I/O on shared files
    *   Using "flexible" APIs
    *   Using non-blocking APIs
    *   Using non-blocking buffered write APIs
  + Detailed descriptions for these programs can be found in
    https://parallel-netcdf.github.io/wiki/QuickTutorial.html

* block_cyclic
  + ./C/block_cyclic.c
  + ./CXX/block_cyclic.cpp
  + ./F77/block_cyclic.f
  + ./F90/block_cyclic.f90
  + This example makes a number of nonblocking API calls, each to write a
    block of columns into a 2D integer variable in a file. In other words,
    data partitioning pattern is a block-cyclic along X dimension.
    The pattern is described by the rank IDs if run with 4 processes.
    ```c
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3,
        0,  0,  1,  1,  2,  2,  3,  3,  0,  0,  1,  1,  2,  2,  3,  3 ;
    ```

* collective_write
  + ./C/collective_write.c
  + ./CXX/collective_write.cpp
  + This example defined `NUM_VARS` 3D integer non-record variables in a file.
    All variables are partitioned among processes in a 3D `block-block-block`
    fashion. The I/O is carried out by making `NUM_VARS` calls to
    `ncmpi_put_vara_int_all()`, one for each variable. Performance measurements
    are reported in the standard output.


* column_wise
  + ./C/column_wise.c
  + ./CXX/column_wise.cpp
  + ./F77/column_wise.f
  + ./F90/column_wise.f90
  + This example makes a number of nonblocking API calls, each writes a single
    column of a 2D integer variable defined in a file. The data partitioning
    pattern among processes is a cyclic along dimension X, illustrated below
    by the process rank IDs if run with 4 processes
  ```c
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
       0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 ;
  ```

* flexible_api
  + ./C/flexible_api.c ./C/flexible_bottom.c ./C/vard_bottom.c
  + ./CXX/flexible_api.cpp
  + ./F77/flexible_api.f
  + ./F90/flexible_api.f90
  + This example shows how to use PnetCDF flexible APIs, `ncmpi_put_vara_all()`
    and `ncmpi_iput_vara()` to write two 2D array variables (one is of 4-byte
    integer byte and the other float type) in parallel.  Local buffers have a
    ghost cell of length 3 surrounded along each dimension.

* get_info
  + ./C/get_info.c
  + ./CXX/get_info.cpp
  + ./F77/get_info.f
  + ./F90/get_info.f90
  + These two example programs print the PnetCDF and MPI-IO hints to the
    standard output.

* hints
  + ./C/hints.c
  + ./CXX/hints.cpp
  + ./F77/hints.f
  + ./F90/hints.f90
  + This example sets PnetCDF hint: `nc_var_align_size`
    and prints the hint values, the header size, header
    extent, and variables' starting file offsets.

* ./C/mput.c
  + This example shows how to use a single call of `ncmpi_mput_vara_all()` to
    write a sequence of requests with arbitrary array indices and lengths.  It
    is intended to run on 4 processes. If more processes were allocated, the
    extra processes write zero-length requests. The offsets and lengths are
    just some random numbers to make the output look like:
    ```c
        3, 3, 3, 1, 1, 0, 0, 2, 1, 1,
        0, 2, 2, 2, 3, 1, 1, 2, 2, 2,
        1, 1, 2, 3, 3, 3, 0, 0, 1, 1,
        0, 0, 0, 2, 1, 1, 1, 3, 3, 3 ;
    ```

* nonblocking_write
  + ./C/nonblocking_write.c
  + ./CXX/nonblocking_write.cpp
  + ./F77/nonblocking_write.f
  + ./F90/nonblocking_write.f90
  + This example is almost the same as to `collective_write.c` but using
    nonblocking APIs instead.


* put_vara
  + ./C/put_vara.c
  + ./CXX/put_vara.cpp
  + ./F77/put_vara.f
  + ./F90/put_var.f90
  + This example shows how to use `nfmpi_put_vara_int_all()` to write a 2D
    4-byte integer array in parallel. The data partitioning pattern among
    processes is a `*-block` in Fortran order. It is described by the process
    rank IDs as below if run on 4 processes.
    ```c
         0, 0, 0, 0, 0,
         0, 0, 0, 0, 0,
         0, 0, 0, 0, 0,
         0, 0, 0, 0, 0,
         1, 1, 1, 1, 1,
         1, 1, 1, 1, 1,
         1, 1, 1, 1, 1,
         1, 1, 1, 1, 1,
         2, 2, 2, 2, 2,
         2, 2, 2, 2, 2,
         2, 2, 2, 2, 2,
         2, 2, 2, 2, 2,
         3, 3, 3, 3, 3,
         3, 3, 3, 3, 3,
         3, 3, 3, 3, 3,
         3, 3, 3, 3, 3 ;
    ```

* put_varn_float
  + ./C/put_varn_float.c
  + ./CXX/put_varn_float.cpp
  + ./F77/put_varn_real.f
  + ./F90/put_varn_real.f90
  + This example makes a single call of `ncmpi_put_varn_float_all()` to write a
    sequence of one-element requests with arbitrary array indices. All
    subrequest indices, `starts[]`, are within the boundaries of a single
    variable. See comments at the beginning of the source file for compile,
    run instructions, and example output.

* put_varn_int
  + ./C/put_varn_int.c
  + ./CXX/put_varn_int.cpp
  + ./F77/put_varn_int.f
  + ./F90/put_varn_int.f90
  + This example makes a single call of `ncmpi_put_varn_int_all()` to write a
    sequence of requests with arbitrary array indices and lengths. All
    subrequests (`starts[]` and `counts[]`) are within the boundaries of a
    single variable. See comments at the beginning of the source file for
    compile, run instructions, and example output.

* ./C/create_open.c
  + This example shows how to use to create a new file, close it, and
    open the file for read only.

* ./C/global_attributes.c
  + This example creates a new file and add 2 global attributes, one is of text
    type and the other 10 short integers. The file is closed and re-opened to
    read back the attributes.

* get_vara
  + ./C/get_vara.c
  + ./CXX/get_vara.cpp
  + This example is the read counterpart of example `put_vara.c`. It shows how
    to use `ncmpi_get_vara_int_all()` to read a 2D 4-byte integer array in
    parallel.  It also reads a global attribute and two attribute of variable
    named "var".  The data partitioning pattern is a column-wise partitioning
    across all processes.

* transpose
  + ./C/transpose.c
  + ./CXX/transpose.cpp
  + ./F77/transpose.f
  + ./F90/transpose.f90
  + This example writes dimensional-transposed 3D arrays using `vara`m APIs.
    For example, when Z=2, Y=12, and X=8, an array partitioned among 4
    processes (P0,P1,P2,P3) and organized in dimension ZYX are illustrated
    below:
  ```c
  P0: var[Z=0][*][*]=  0,   1,   2,   3,  P1: var[Z=0][*][*]=  4,   5,   6,   7,
                       8,   9,  10,  11,                      12,  13,  14,  15,
                      16,  17,  18,  19,                      20,  21,  22,  23,
                      24,  25,  26,  27,                      28,  29,  30,  31,
                      32,  33,  34,  35,                      36,  37,  38,  39,
                      40,  41,  42,  43,                      44,  45,  46,  47,

  P2: var[Z=0][*][*]= 48,  49,  50,  51,  P3: var[Z=0][*][*]= 52,  53,  54,  55,
                      56,  57,  58,  59,                      60,  61,  62,  63,
                      64,  65,  66,  67,                      68,  69,  70,  71,
                      72,  73,  74,  75,                      76,  77,  78,  79,
                      80,  81,  82,  83,                      84,  85,  86,  87,
                      88,  89,  90,  91,                      92,  93,  94,  95,

  P0: var[Z=1][*][*]= 96,  97,  98,  99,  P1: var[Z=1][*][*]=100, 101, 102, 103,
                     104, 105, 106, 107,                     108, 109, 110, 111,
                     112, 113, 114, 115,                     116, 117, 118, 119,
                     120, 121, 122, 123,                     124, 125, 126, 127,
                     128, 129, 130, 131,                     132, 133, 134, 135,
                     136, 137, 138, 139,                     140, 141, 142, 143,

  P2: var[Z=1][*][*]=144, 145, 146, 147,  P3: var[Z=1][*][*]=148, 149, 150, 151,
                     152, 153, 154, 155,                     156, 157, 158, 159,
                     160, 161, 162, 163,                     164, 165, 166, 167,
                     168, 169, 170, 171,                     172, 173, 174, 175,
                     176, 177, 178, 179,                     180, 181, 182, 183,
                     184, 185, 186, 187,                     188, 189, 190, 191 ;

  ```
  When writing the subarray in parallel to a file, the array contents in file are:
  ```c
  var[Z=0][*][*]:  0,   1,   2,   3,   4,   5,   6,   7,
                   8,   9,  10,  11,  12,  13,  14,  15,
                  16,  17,  18,  19,  20,  21,  22,  23,
                  24,  25,  26,  27,  28,  29,  30,  31,
                  32,  33,  34,  35,  36,  37,  38,  39,
                  40,  41,  42,  43,  44,  45,  46,  47,
                  48,  49,  50,  51,  52,  53,  54,  55,
                  56,  57,  58,  59,  60,  61,  62,  63,
                  64,  65,  66,  67,  68,  69,  70,  71,
                  72,  73,  74,  75,  76,  77,  78,  79,
                  80,  81,  82,  83,  84,  85,  86,  87,
                  88,  89,  90,  91,  92,  93,  94,  95,

  var[Z=1][*][*]: 96,  97,  98,  99, 100, 101, 102, 103,
                 104, 105, 106, 107, 108, 109, 110, 111,
                 112, 113, 114, 115, 116, 117, 118, 119,
                 120, 121, 122, 123, 124, 125, 126, 127,
                 128, 129, 130, 131, 132, 133, 134, 135,
                 136, 137, 138, 139, 140, 141, 142, 143,
                 144, 145, 146, 147, 148, 149, 150, 151,
                 152, 153, 154, 155, 156, 157, 158, 159,
                 160, 161, 162, 163, 164, 165, 166, 167,
                 168, 169, 170, 171, 172, 173, 174, 175,
                 176, 177, 178, 179, 180, 181, 182, 183,
                 184, 185, 186, 187, 188, 189, 190, 191 ;
  ```

  When writing the transposed subarray (XYZ) in parallel to a file, the file contents are:
  ```c
  var[X=0][*][*]= 0, 48,  96, 144,    var[X=1][*][*]= 1, 49,  97, 145,
                  8, 56, 104, 152,                    9, 57, 105, 153,
                 16, 64, 112, 160,                   17, 65, 113, 161,
                 24, 72, 120, 168,                   25, 73, 121, 169,
                 32, 80, 128, 176,                   33, 81, 129, 177,
                 40, 88, 136, 184,                   41, 89, 137, 185,

  var[X=2][*][*]= 2, 50,  98, 146,    var[X=3][*][*]= 3, 51,  99, 147,
                 10, 58, 106, 154,                   11, 59, 107, 155,
                 18, 66, 114, 162,                   19, 67, 115, 163,
                 26, 74, 122, 170,                   27, 75, 123, 171,
                 34, 82, 130, 178,                   35, 83, 131, 179,
                 42, 90, 138, 186,                   43, 91, 139, 187,

  var[X=4][*][*]= 4, 52, 100, 148,    var[X=5][*][*]= 5, 53, 101, 149,
                 12, 60, 108, 156,                   13, 61, 109, 157,
                 20, 68, 116, 164,                   21, 69, 117, 165,
                 28, 76, 124, 172,                   29, 77, 125, 173,
                 36, 84, 132, 180,                   37, 85, 133, 181,
                 44, 92, 140, 188,                   45, 93, 141, 189,

  var[X=6][*][*]= 6, 54, 102, 150,    var[X=7][*][*]= 7, 55, 103, 151,
                 14, 62, 110, 158,                   15, 63, 111, 159,
                 22, 70, 118, 166,                   23, 71, 119, 167,
                 30, 78, 126, 174,                   31, 79, 127, 175,
                 38, 86, 134, 182,                   39, 87, 135, 183,
                 46, 94, 142, 190,                   47, 95, 143, 191 ;
  ```

* vard_int
  + ./C/vard_int.c
  + ./CXX/vard_int.cpp
  + ./F77/vard_int.f
  + ./F90/vard_int.f90
  + These examples show how to use `vard` APIs to write/read record and
    fixed-size variables.

* bput_varn_uint
  + ./C/bput_varn_uint.c
  + ./F77/bput_varn_int8.f
  + These examples show how to use nonblocking `bput_varn` APIs

* i_varn_int64
  + ./C/i_varn_int64.c
  + ./F77/i_varn_real.f
  + These examples show how to use nonblocking `iput_varn` and `iget_varn` APIs

* fill_mode
  + ./C/fill_mode.c
  + ./CXX/fill_mode.cpp
  + ./F77/fill_mode.f
  + ./F90/fill_mode.f90
  + These examples show how to enable file mode

* ./C/req_all.c
  + This example show how to use `NC_REQ_ALL` to flush all pending nonblocking
    requests without providing the request IDs and status array.

* ./C/nonblocking_write_in_def.c
  + This example is the same as `nonblocking_write.c` except all nonblocking
    write requests (calls to `iput` and `bput`) are posted in define mode.
    After entering to data mode, a single call to `ncmpi_wait_all()` to flush
    all pending nonblocking requests.

* ./burst_buffer/create_open.c
  + This program shows an example of creating and opening a NetCDF file with the
    burst buffering feature enabled.

* ./burst_buffer/nonblocking.c
  + This program shows an example of using non-blocking put APIs with the burst
    buffering feature enabled.

* ./C/pthread.c
  + This program shows an example of using one-file-per-thread I/O operation
    when the thread-safe capability is enabled.

* time_var
  + ./C/time_var.c
  + ./F77/time_var.f
  + These examples show how to define, write, and read record variables.

* ./adios/read_var.c
  + This example shows how to read variables from a BP file using PnetCDF
    blocking APIs

* ./adios/read_var_nb.c
  + This example shows how to read variable from a BP file using PnetCDF
    non-blocking APIs

* ./adios/read_metadata.c
  + This example shows how to enumerate all variables, dimensions, and
    attributes from a BP file

* ./C/create_from_cdl.c
  + This example shows how to call the CDL header file parser APIs. It reads
    a text file in the CDL format and creates a new netCDF file with the same
    metadata from the CDL file.

Copyright (C) 2012, Northwestern University and Argonne National Laboratory.
See COPYRIGHT notice in top-level directory.

