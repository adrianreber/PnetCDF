/*********************************************************************
 *
 *  Copyright (C) 2014, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 *
 *********************************************************************/
/* $Id$ */

/*    This example is similar to collective_write.c but using nonblocking APIs.
 *    It creates a netcdf file in CD-5 format and writes a number of
 *    3D integer non-record variables. The measured write bandwidth is reported
 *    at the end. Usage: (for example)
 *    To compile:
 *        mpicxx -O2 nonblocking_write.cpp -o nonblocking_write -lpnetcdf
 *    To run:
 *        mpiexec -n num_processes ./nonblocking_write [filename] [len]
 *    where len decides the size of each local array, which is len x len x len.
 *    So, each non-record variable is of size len*len*len * nprocs * sizeof(int)
 *    All variables are partitioned among all processes in a 3D
 *    block-block-block fashion. Below is an example standard output from
 *    command:
 *        mpiexec -n 32 ./nonblocking_write /pvfs2/wkliao/testfile.nc 100
 *
 *    MPI hint: cb_nodes        = 2
 *    MPI hint: cb_buffer_size  = 16777216
 *    MPI hint: striping_factor = 32
 *    MPI hint: striping_unit   = 1048576
 *    Local array size 100 x 100 x 100 integers, size = 3.81 MB
 *    Global array size 400 x 400 x 200 integers, write size = 0.30 GB
 *     procs    Global array size  exec(sec)  write(MB/s)
 *     -------  ------------------  ---------  -----------
 *        32     400 x  400 x  200     6.67       45.72
 */

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream> // for ostringstream

using namespace std;

#include <string.h> /* strcpy(), strncpy() */
#include <unistd.h> /* getopt() */
#include <pnetcdf>

using namespace PnetCDF;
using namespace PnetCDF::exceptions;

#define NDIMS    3
#define NUM_VARS 10

static void
usage(char *argv0)
{
    cerr <<
    "Usage: %s [-h] | [-q] [-l len] [file_name]\n"
    "       [-h] Print help\n"
    "       [-q] Quiet mode (reports when fail)\n"
    "       [-l len] size of each dimension of the local array\n"
    "       [filename] output netCDF file name\n"
    << argv0;
}

/*----< print_info() >------------------------------------------------------*/
static
void print_info(MPI_Info *info_used)
{
    int     flag;
    char    info_cb_nodes[64], info_cb_buffer_size[64];
    char    info_striping_factor[64], info_striping_unit[64];

    strcpy(info_cb_nodes,        "undefined");
    strcpy(info_cb_buffer_size,  "undefined");
    strcpy(info_striping_factor, "undefined");
    strcpy(info_striping_unit,   "undefined");

    MPI_Info_get(*info_used, (char*)"cb_nodes", 64, info_cb_nodes, &flag);
    MPI_Info_get(*info_used, (char*)"cb_buffer_size", 64, info_cb_buffer_size, &flag);
    MPI_Info_get(*info_used, (char*)"striping_factor", 64, info_striping_factor, &flag);
    MPI_Info_get(*info_used, (char*)"striping_unit", 64, info_striping_unit, &flag);

    printf("MPI hint: cb_nodes        = %s\n", info_cb_nodes);
    printf("MPI hint: cb_buffer_size  = %s\n", info_cb_buffer_size);
    printf("MPI hint: striping_factor = %s\n", info_striping_factor);
    printf("MPI hint: striping_unit   = %s\n", info_striping_unit);
}

/*----< main() >------------------------------------------------------------*/
int main(int argc, char **argv)
{
    extern int optind;
    extern char *optarg;
    int i, j, verbose=1;
    int nprocs, len=0, *buf[NUM_VARS], bufsize, rank;
    int gsizes[NDIMS], psizes[NDIMS];
    double write_timing, max_write_timing, write_bw;
    char filename[256];
    int req[NUM_VARS], st[NUM_VARS];
    MPI_Offset write_size, sum_write_size;
    vector<MPI_Offset> starts(NDIMS), counts(NDIMS);
    MPI_Offset bbufsize, put_size;
    MPI_Info info, info_used;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    /* get command-line arguments */
    while ((i = getopt(argc, argv, "hql:")) != EOF)
        switch(i) {
            case 'q': verbose = 0;
                      break;
            case 'l': len = atoi(optarg);
                      break;
            case 'h':
            default:  if (rank==0) usage(argv[0]);
                      MPI_Finalize();
                      return 1;
        }
    if (argv[optind] == NULL) strcpy(filename, "testfile.nc");
    else                      snprintf(filename, 256, "%s", argv[optind]);

    len = (len <= 0) ? 10 : len;

    for (i=0; i<NDIMS; i++) psizes[i] = 0;

    MPI_Dims_create(nprocs, NDIMS, psizes);
    starts[0] = rank % psizes[0];
    starts[1] = (rank / psizes[1]) % psizes[1];
    starts[2] = (rank / (psizes[0] * psizes[1])) % psizes[2];

    bufsize = 1;
    for (i=0; i<NDIMS; i++) {
        gsizes[i] = len * psizes[i];
        starts[i] *= len;
        counts[i]  = len;
        bufsize   *= len;
    }

    /* allocate buffer and initialize with some non-zero numbers */
    for (i=0; i<NUM_VARS; i++) {
        buf[i] = (int *) malloc(sizeof(int) * bufsize);
        for (j=0; j<bufsize; j++) buf[i][j] = rank * i + 123 + j;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    write_timing = MPI_Wtime();

    try {
        MPI_Info_create(&info);
        MPI_Info_set(info, (char*)"nc_var_align_size", (char*)"1");

        /* create the file */
        NcmpiFile nc(MPI_COMM_WORLD, filename, NcmpiFile::replace,
                     NcmpiFile::classic5, info);
        MPI_Info_free(&info);

        /* define dimensions */
        vector<NcmpiDim> dimids(NDIMS);
        for (i=0; i<NDIMS; i++) {
            char name[2];
            name[0] = 'x'+i;
            name[1] = '\0';
            dimids[i] = nc.addDim(name, gsizes[i]);
        }

        /* define variables */
        vector<NcmpiVar> vars(NUM_VARS);
        for (i=0; i<NUM_VARS; i++) {
            std::ostringstream name;
            name << "var" << i;
            vars[i] = nc.addVar(name.str(), ncmpiInt, dimids);
        }

        /* get all the hints used */
        nc.Inq_file_info(&info_used);

        /* write one variable at a time using iput */
        for (i=0; i<NUM_VARS; i++)
            vars[i].iputVar(starts, counts, &buf[i][0], &req[i]);

        /* wait for the nonblocking I/O to complete */
        nc.Wait_all(NUM_VARS, req, st);

        for (i=0; i<NUM_VARS; i++) {
            if (st[i] != NC_NOERR)
                printf("Error: nonblocking write fails on request %d (%s)\n",
                       i, ncmpi_strerror(st[i]));
        }

        /* write one variable at a time using bput */

        /* bbufsize must be max of data type converted before and after */
        bbufsize = bufsize * NUM_VARS * sizeof(int);
        nc.Buffer_attach(bbufsize);

        for (i=0; i<NUM_VARS; i++) {
            vars[i].bputVar(starts, counts, buf[i], &req[i]);
            /* can safely change contents or free up the buf[i] here */
        }

        /* wait for the nonblocking I/O to complete */
        nc.Wait_all(NUM_VARS, req, st);
        for (i=0; i<NUM_VARS; i++) {
            if (st[i] != NC_NOERR)
                printf("Error: nonblocking write fails on request %d (%s)\n",
                       i, ncmpi_strerror(st[i]));
        }

        /* detach the temporary buffer */
        nc.Buffer_detach();

        nc.Inq_put_size(&put_size);
        MPI_Allreduce(MPI_IN_PLACE, &put_size, 1, MPI_OFFSET, MPI_SUM, MPI_COMM_WORLD);
    }
    catch(NcmpiException& e) {
       cout << e.what() << " error code=" << e.errorCode() << " Error!\n";
       return 1;
    }
    write_timing = MPI_Wtime() - write_timing;

    write_size = bufsize * NUM_VARS * sizeof(int);
    for (i=0; i<NUM_VARS; i++) free(buf[i]);

    MPI_Reduce(&write_size,   &sum_write_size,   1, MPI_OFFSET, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&write_timing, &max_write_timing, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0 && verbose) {
        double write_amount = sum_write_size;
        float subarray_size = (float)bufsize*sizeof(int)/1048576.0;
        printf("\n");
        printf("Total amount writes to variables only   (exclude header) = %lld bytes\n", sum_write_size);
        printf("Total amount writes reported by pnetcdf (include header) = %lld bytes\n", put_size);
        printf("\n");
        print_info(&info_used);
        printf("Local array size %d x %d x %d integers, size = %.2f MB\n",len,len,len,subarray_size);
        write_amount /= 1048576.0;
        printf("Global array size %d x %d x %d integers, write size = %.2f GB\n",
               gsizes[0], gsizes[1], gsizes[2], write_amount/1024.0);

        write_bw = write_amount/max_write_timing;
        printf(" procs    Global array size  exec(sec)  write(MB/s)\n");
        printf("-------  ------------------  ---------  -----------\n");
        printf(" %4d    %4d x %4d x %4d %8.2f  %10.2f\n", nprocs,
               gsizes[0], gsizes[1], gsizes[2], max_write_timing, write_bw);
    }
    MPI_Info_free(&info_used);

    /* check if there is any PnetCDF internal malloc residue */
    MPI_Offset malloc_size, sum_size;
    int err = ncmpi_inq_malloc_size(&malloc_size);
    if (err == NC_NOERR) {
        MPI_Reduce(&malloc_size, &sum_size, 1, MPI_OFFSET, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0 && sum_size > 0)
            printf("heap memory allocated by PnetCDF internally has %lld bytes yet to be freed\n",
                   sum_size);
    }

    MPI_Finalize();
    return 0;
}

