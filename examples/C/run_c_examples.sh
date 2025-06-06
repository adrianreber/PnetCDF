#!/bin/sh
#
# Copyright (C) 2022, Northwestern University and Argonne National Laboratory
# See COPYRIGHT notice in top-level directory.
#

# Exit immediately if a command exits with a non-zero status.
set -e

NPROCS=4
if test "x$1" != x ; then
   NPROCS=$1
fi
OUTDIR=TESTOUTDIR
MPIRUN="TESTMPIRUN"
MPIRUN=`echo ${MPIRUN} | SED_CMD -e "s/NP/${NPROCS}/g"`
run_BURST_BUFFER=ENABLE_BURST_BUFFER
run_NETCDF4=ENABLE_NETCDF4

for i in check_PROGRAMS ; do
    CMD_OPTS="-q ${OUTDIR}/$i.nc"
    if test $i = get_vara ; then
       # get_vara reads the file 'put_vara.nc' created by put_vara
       CMD_OPTS="-q ${OUTDIR}/put_vara.nc"
    elif test $i = create_from_cdl ; then
       # create_from_cdl reads a CDL header file
       CMD_OPTS="-q -o ${OUTDIR}/$i.nc ./cdl_header.txt"
    fi
    ${MPIRUN} ./$i ${CMD_OPTS}

    if test $? = 0 ; then
       echo "PASS:  C  parallel run on ${NPROCS} processes --------------- $i"
    fi

    if test "x${run_BURST_BUFFER}" = x1 ; then
       # echo "test burst buffering feature"
       export PNETCDF_HINTS="nc_burst_buf=enable;nc_burst_buf_dirname=${OUTDIR};nc_burst_buf_overwrite=enable"
       CMD_OPTS="-q ${OUTDIR}/$i.bb.nc"
       if test $i = get_vara ; then
          # get_vara reads the file 'put_vara.nc' created by put_vara
          CMD_OPTS="-q ${OUTDIR}/put_vara.bb.nc"
       elif test $i = create_from_cdl ; then
          # create_from_cdl reads a CDL header file
          CMD_OPTS="-q -o ${OUTDIR}/$i.bb.nc ./cdl_header.txt"
       fi
       ${MPIRUN} ./$i ${CMD_OPTS}
       if test $? = 0 ; then
          echo "PASS:  C  parallel run on ${NPROCS} processes --------------- $i"
       fi
       unset PNETCDF_HINTS
    fi

    if test "x${run_NETCDF4}" = x1 ; then
       # echo "test netCDF-4 feature"
       if test $i != create_from_cdl ; then
          ${MPIRUN} ./$i ${OUTDIR}/$i.nc4 4
       fi
    fi

    # delete output file
    if test $i = get_vara ; then
       rm -f ${OUTDIR}/put_vara.nc
       rm -f ${OUTDIR}/put_vara.bb.nc
    elif test $i != put_vara ; then
       rm -f ${OUTDIR}/$i.nc
       rm -f ${OUTDIR}/$i.bb.nc
    fi
done

