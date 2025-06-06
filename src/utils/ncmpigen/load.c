/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <pnetcdf.h>
#include "generic.h"
#include "ncmpigen.h"
#include "genlib.h"

extern int netcdf_flag;
extern int c_flag;
extern int fortran_flag;

#define fpr    (void) fprintf

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

/*
 * Remove trailing zeros (after decimal point) but not trailing decimal
 * point from ss, a string representation of a floating-point number that
 * might include an exponent part.
 */
static void
tztrim(
    char *ss			/* returned string representing dd */
    )
{
    char *cp, *ep;

    cp = ss;
    if (*cp == '-')
      cp++;
    while(isdigit((int)*cp) || *cp == '.')
      cp++;
    if (*--cp == '.')
      return;
    ep = cp+1;
    while (*cp == '0')
      cp--;
    cp++;
    if (cp == ep)
      return;
    while (*ep)
      *cp++ = *ep++;
    *cp = '\0';
    return;
}


/* generate C to put netCDF record from in-memory data */
static void
gen_load_c(
    void *rec_start
    )
{
    int  idim, ival;
    char *val_string;
    char *charvalp = NULL;
    short *shortvalp = NULL;
    int *intvalp = NULL;
    float *floatvalp = NULL;
    double *doublevalp = NULL;
    unsigned char *ubytevalp = NULL;
    unsigned short *ushortvalp = NULL;
    unsigned int *uintvalp = NULL;
    long long *int64valp = NULL;
    unsigned long long *uint64valp = NULL;
    char stmnt[C_MAX_STMNT];
    size_t stmnt_len;
    char s2[C_MAX_STMNT];

    if (!vars[varnum].has_data)
	return;

    s2[0] = '\0';
    cline("");
    sprintf(stmnt, "   {\t\t\t/* store %s */", vars[varnum].name);
    cline(stmnt);

    if (vars[varnum].ndims > 0) {
	if (vars[varnum].dims[0] == rec_dim) {
	    sprintf(stmnt, "    static MPI_Offset %s_start[RANK_%s];",
		    vars[varnum].lname, vars[varnum].lname);
	    cline(stmnt);

	    sprintf(stmnt, "    static MPI_Offset %s_count[RANK_%s];",
		    vars[varnum].lname, vars[varnum].lname);
	    cline(stmnt);
	}

	/* load variable with data values using static initialization */
	sprintf(stmnt, "    static %s %s[] = {",
		ncctype(vars[varnum].type),
		vars[varnum].lname);

	stmnt_len = strlen(stmnt);
	switch (vars[varnum].type) {
	  case NC_CHAR:
	    val_string = cstrstr((char *) rec_start, var_len);
	    sprintf(s2, "%s", val_string);
	    strncat(stmnt, s2, C_MAX_STMNT - strlen(stmnt) );
	    free(val_string);
	    break;
	  default:
	    switch (vars[varnum].type) {
	      case NC_BYTE:
		charvalp = (char *) rec_start;
		break;
	      case NC_SHORT:
		shortvalp = (short *) rec_start;
		break;
	      case NC_INT:
		intvalp = (int *) rec_start;
		break;
	      case NC_FLOAT:
		floatvalp = (float *) rec_start;
		break;
	      case NC_DOUBLE:
		doublevalp = (double *) rec_start;
		break;
	      case NC_UBYTE:
		ubytevalp = (unsigned char *) rec_start;
		break;
	      case NC_USHORT:
		ushortvalp = (unsigned short *) rec_start;
		break;
	      case NC_UINT:
		uintvalp = (unsigned int *) rec_start;
		break;
	      case NC_INT64:
		int64valp = (long long *) rec_start;
		break;
	      case NC_UINT64:
		uint64valp = (unsigned long long *) rec_start;
		break;
	      default:
		derror("Unhandled type %d\n", vars[varnum].type);
		return;
	    }
            for (ival = 0; ival < var_len-1; ival++) {
		switch (vars[varnum].type) {
		  case NC_BYTE:
			sprintf(s2, "%d, ", *charvalp++);
		    break;
		  case NC_SHORT:
			sprintf(s2, "%d, ", *shortvalp++);
		    break;
		  case NC_INT:
			sprintf(s2, "%ld, ", (long)*intvalp++);
		    break;
		  case NC_FLOAT:
			sprintf(s2, "%.8g, ", *floatvalp++);
		    break;
		  case NC_DOUBLE:
			sprintf(s2, "%#.16g", *doublevalp++);
			tztrim(s2);
			strcat(s2, ", ");
		    break;
		  case NC_UBYTE:
			sprintf(s2, "%hhu, ", (unsigned char)*ubytevalp++);
		    break;
		  case NC_USHORT:
			sprintf(s2, "%hu, ", (unsigned short)*ushortvalp++);
		    break;
		  case NC_UINT:
			sprintf(s2, "%u, ", (unsigned int)*uintvalp++);
		    break;
		  case NC_INT64:
			sprintf(s2, "%lld, ", (long long)*int64valp++);
		    break;
		  case NC_UINT64:
			sprintf(s2, "%llu, ", (unsigned long long)*uint64valp++);
		    break;
		  default:
		    derror("Unhandled type %d\n", vars[varnum].type);
		    return;

		}
		stmnt_len += strlen(s2);
		if (stmnt_len < C_MAX_STMNT)
		  strcat(stmnt, s2);
		else {
		    cline(stmnt);
		    strcpy(stmnt,s2);
		    stmnt_len = strlen(stmnt);
		}
	    }
	    for (;ival < var_len; ival++) {
		switch (vars[varnum].type) {
		  case NC_BYTE:
			sprintf(s2, "%d", *charvalp);
		    break;
		  case NC_SHORT:
			sprintf(s2, "%d", *shortvalp);
		    break;
		  case NC_INT:
			sprintf(s2, "%ld", (long)*intvalp);
		    break;
		  case NC_FLOAT:
			sprintf(s2, "%.8g", *floatvalp);
		    break;
		  case NC_DOUBLE:
			sprintf(s2, "%#.16g", *doublevalp++);
			tztrim(s2);
		    break;
		  case NC_UBYTE:
			sprintf(s2, "%hhu, ", (unsigned char)*ubytevalp++);
		    break;
		  case NC_USHORT:
			sprintf(s2, "%hu, ", (unsigned short)*ushortvalp++);
		    break;
		  case NC_UINT:
			sprintf(s2, "%u, ", (unsigned int)*uintvalp++);
		    break;
		  case NC_INT64:
			sprintf(s2, "%lld, ", (long long)*int64valp++);
		    break;
		  case NC_UINT64:
			sprintf(s2, "%llu, ", (unsigned long long)*uint64valp++);
		    break;
		  default:
		    derror("Unhandled type %d\n", vars[varnum].type);
		    break;
		}
		stmnt_len += strlen(s2);
		if (stmnt_len < C_MAX_STMNT)
		  strcat(stmnt, s2);
		else {
		    cline(stmnt);
		    strcpy(stmnt,s2);
		    stmnt_len = strlen(stmnt);
		}
	    }
	    break;
	}
	strcat(stmnt,"};");
	cline(stmnt);

	if (vars[varnum].dims[0] == rec_dim) {
	    sprintf(stmnt,
		    "    %s_len = %lu;			/* number of records of %s data */",
		    dims[rec_dim].lname,
		    (unsigned long)vars[varnum].nrecs, /* number of recs for this variable */
		    vars[varnum].name);
	    cline(stmnt);

	    for (idim = 0; idim < vars[varnum].ndims; idim++) {
		sprintf(stmnt, "    %s_start[%d] = 0;",
			vars[varnum].lname,
			idim);
		cline(stmnt);
	    }

	    for (idim = 0; idim < vars[varnum].ndims; idim++) {
		sprintf(stmnt, "    %s_count[%d] = %s_len;",
			vars[varnum].lname,
			idim,
			dims[vars[varnum].dims[idim]].lname);
		cline(stmnt);
	    }
	}

	if (vars[varnum].dims[0] == rec_dim) {
	    sprintf(stmnt,
		    "    stat = ncmpi_put_vara_%s_all(ncid, %s_id, %s_start, %s_count, %s);",
		    ncstype(vars[varnum].type),
		    vars[varnum].lname,
		    vars[varnum].lname,
		    vars[varnum].lname,
		    vars[varnum].lname);
	     cline(stmnt);
	} else {		/* non-record variables */
	    cline("  ncmpi_begin_indep_data(ncid);");
	    sprintf(stmnt,
		    "    stat = ncmpi_put_var_%s(ncid, %s_id, %s);",
		    ncstype(vars[varnum].type),
		    vars[varnum].lname,
		    vars[varnum].lname);
	    cline(stmnt);
	    cline("  ncmpi_end_indep_data(ncid);");
	}
    } else {			/* scalar variables */
	/* load variable with data values using static initialization */
	sprintf(stmnt, "    static %s %s = ",
		ncctype(vars[varnum].type),
		vars[varnum].lname);

	switch (vars[varnum].type) {
	  case NC_CHAR:
	    val_string = cstrstr((char *) rec_start, var_len);
	    val_string[strlen(val_string)-1] = '\0';
	    sprintf(s2, "'%s'", &val_string[1]);
	    free(val_string);
	    break;
	  case NC_BYTE:
	    charvalp = (char *) rec_start;
	    sprintf(s2, "%d", *charvalp);
	    break;
	  case NC_SHORT:
	    shortvalp = (short *) rec_start;
	    sprintf(s2, "%d", *shortvalp);
	    break;
	  case NC_INT:
	    intvalp = (int *) rec_start;
	    sprintf(s2, "%ld", (long)*intvalp);
	    break;
	  case NC_FLOAT:
	    floatvalp = (float *) rec_start;
	    sprintf(s2, "%.8g", *floatvalp);
	    break;
	  case NC_DOUBLE:
	    doublevalp = (double *) rec_start;
	    sprintf(s2, "%#.16g", *doublevalp++);
	    tztrim(s2);
	    break;
	  case NC_UBYTE:
	    ubytevalp = (unsigned char *) rec_start;
	    sprintf(s2, "%hhu", (unsigned char)*ubytevalp);
	    break;
	  case NC_USHORT:
	    ushortvalp = (unsigned short *) rec_start;
	    sprintf(s2, "%hu", (unsigned short)*ushortvalp);
	    break;
	  case NC_UINT:
	    uintvalp = (unsigned int *) rec_start;
	    sprintf(s2, "%u", (unsigned int)*uintvalp);
	    break;
	  case NC_INT64:
	    int64valp = (long long *) rec_start;
	    sprintf(s2, "%lld", (long long)*int64valp);
	    break;
	  case NC_UINT64:
	    uint64valp = (unsigned long long *) rec_start;
	    sprintf(s2, "%llu", (unsigned long long)*uint64valp);
	    break;
	  default:
	    derror("Unhandled type %d\n", vars[varnum].type);
	    break;
	}
	strncat(stmnt, s2, C_MAX_STMNT - strlen(stmnt) );
	strcat(stmnt,";");
	cline(stmnt);
	cline("  ncmpi_begin_indep_data(ncid);");
	sprintf(stmnt,
		"    stat = ncmpi_put_var_%s(ncid, %s_id, &%s);",
		ncstype(vars[varnum].type),
		vars[varnum].lname,
		vars[varnum].lname);
	cline(stmnt);
	cline("  ncmpi_end_indep_data(ncid);");
    }
    cline("    check_err(stat,__LINE__,__FILE__);");
    cline("   }");
}


/*
 * Add to a partial Fortran statement, checking if it's too long.  If it is too
 * long, output the first part of it as a single statement with continuation
 * characters and start a new (probably invalid) statement with the remainder.
 * This will cause a Fortran compiler error, but at least all the information
 * will be available.
 */
static void
fstrcat(
    char *s,			/* source string of stement being built */
    const char *t,		/* string to be appended to source */
    size_t *slenp		/* pointer to length of source string */
    )
{
    *slenp += strlen(t);
    if (*slenp >= FORT_MAX_STMNT) {
	derror("FORTRAN statement too long: %s",s);
	fline(s);
	strcpy(s, t);
	*slenp = strlen(s);
    } else {
	strcat(s, t);
    }
}

/*
 * Create Fortran data statement to initialize numeric variable with
 * values.
 */
static void
f_var_init(
    int varnum,			/* which variable */
    void *rec_start		/* start of data */
    )
{
    char *val_string;
    char *charvalp;
    short *shortvalp;
    int *intvalp;
    float *floatvalp;
    double *doublevalp;
    unsigned char *ubytevalp;
    unsigned short *ushortvalp;
    unsigned int *uintvalp;
    long long *int64valp;
    unsigned long long *uint64valp;
    char stmnt[FORT_MAX_STMNT];
    size_t stmnt_len;
    char s2[FORT_MAX_STMNT];
    int ival;

    /* load variable with data values  */
    sprintf(stmnt, "data %s /",vars[varnum].lname);
    stmnt_len = strlen(stmnt);
    switch (vars[varnum].type) {
    case NC_BYTE:
	charvalp = (char *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    val_string = fstring(NC_BYTE,(void *)charvalp++,0);
	    sprintf(s2, "%s, ", val_string);
	    fstrcat(stmnt, s2, &stmnt_len);
	    free(val_string);
	}
	val_string = fstring(NC_BYTE,(void *)charvalp++,0);
	fstrcat(stmnt, val_string, &stmnt_len);
	free(val_string);
	break;
    case NC_SHORT:
	shortvalp = (short *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%d, ", *shortvalp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%d", *shortvalp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_INT:
	intvalp = (int *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%ld, ", (long)*intvalp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%ld", (long)*intvalp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_FLOAT:
	floatvalp = (float *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%.8g, ", *floatvalp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%.8g", *floatvalp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_DOUBLE:
	doublevalp = (double *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%#.16g", *doublevalp++);
	    tztrim(s2);
	    expe2d(s2);	/* change 'e' to 'd' in exponent */
	    fstrcat(s2, ", ", &stmnt_len);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%#.16g", *doublevalp++);
	tztrim(s2);
	expe2d(s2);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_UBYTE:
	ubytevalp = (unsigned char *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%hhu, ", (unsigned char)*ubytevalp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%hhu", (unsigned char)*ubytevalp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_USHORT:
	ushortvalp = (unsigned short *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%hu, ", (unsigned short)*ushortvalp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%hu", (unsigned short)*ushortvalp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_UINT:
	uintvalp = (unsigned int *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%u, ", (unsigned int)*uintvalp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%u", (unsigned int)*uintvalp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_INT64:
	int64valp = (long long *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%lld, ", (long long)*int64valp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%lld", (long long)*int64valp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    case NC_UINT64:
	uint64valp = (unsigned long long *) rec_start;
	for (ival = 0; ival < var_len-1; ival++) {
	    sprintf(s2, "%llu, ", (unsigned long long)*uint64valp++);
	    fstrcat(stmnt, s2, &stmnt_len);
	}
	sprintf(s2, "%llu", (unsigned long long)*uint64valp);
	fstrcat(stmnt, s2, &stmnt_len);
	break;
    default:
	derror("fstrstr: bad type");
	break;
    }
    fstrcat(stmnt, "/", &stmnt_len);

    /* For record variables, store data statement for later use;
      otherwise, just print it. */
    if (vars[varnum].ndims > 0 && vars[varnum].dims[0] == rec_dim) {
	char *dup_stmnt = (char*) emalloc(strlen(stmnt)+1);
	strcpy(dup_stmnt, stmnt); /* ULTRIX missing strdup */
	vars[varnum].data_stmnt = dup_stmnt;
    } else {
	fline(stmnt);
    }
}


/* make Fortran to put record */
static void
gen_load_fortran(
    void *rec_start
    )
{
    char stmnt[FORT_MAX_STMNT];
    struct vars *v = &vars[varnum];

    if (!v->has_data)
	return;

    if (v->ndims == 0 || v->dims[0] != rec_dim) {
	sprintf(stmnt, "* store %s", v->name);
	fline(stmnt);
    }

    /* generate code to initialize variable with values found in CDL input */
    if (v->type != NC_CHAR) {
	f_var_init(varnum, (char*)rec_start);
    } else {
	v->data_stmnt = (char*) fstrstr((char*)rec_start, valnum);
    }

    if (v->ndims >0 && v->dims[0] == rec_dim) {
	return;
    }
    if (v->type != NC_CHAR) {
	sprintf(stmnt, "iret = nf_put_var_%s(ncid, %s_id, %s)",
		nfftype(v->type), v->lname, v->lname);
    } else {
	char *char_expr = (char*) fstrstr((char*)rec_start, valnum);
	sprintf(stmnt, "iret = nf_put_var_%s(ncid, %s_id, %s)",
		nfftype(v->type), v->lname, char_expr);
	free(char_expr);
    }

    fline(stmnt);
    fline("call check_err(iret)");
}


/* invoke netcdf calls (or generate C or Fortran code) to load netcdf variable
 * from in-memory data.  Assumes following global variables set from yacc
 * parser:
 * int varnum        - number of variable to be loaded.
 * struct vars[varnum] - structure containing info on variable, specifically
 *                     name, type, ndims, dims, fill_value, has_data
 * int rec_dim       - id of record dimension, or -1 if none
 * struct dims[]     - structure containing name and size of dimensions.
 */
int
put_variable(void *rec_start)	/* points to data to be loaded  */
{
    if (netcdf_flag)
      load_netcdf(rec_start);	/* put variable values */
    if (c_flag)			/* create C code to put values */
      gen_load_c(rec_start);
    if (fortran_flag)		/* create Fortran code to put values */
      gen_load_fortran(rec_start);

    return 0;
}


/* write out variable's data from in-memory structure */
void
load_netcdf(void *rec_start)
{
    int i, idim;
    int stat = NC_NOERR;
    MPI_Offset *start, *count;
    char *charvalp = NULL;
    short *shortvalp = NULL;
    int *intvalp = NULL;
    float *floatvalp = NULL;
    double *doublevalp = NULL;
    unsigned char *ubytevalp = NULL;
    unsigned short *ushortvalp = NULL;
    unsigned int *uintvalp = NULL;
    long long *int64valp = NULL;
    unsigned long long *uint64valp = NULL;
    MPI_Offset total_size;

    /* load values into variable */

    switch (vars[varnum].type) {
      case NC_CHAR:
      case NC_BYTE:
	charvalp = (char *) rec_start;
	break;
      case NC_SHORT:
	shortvalp = (short *) rec_start;
	break;
      case NC_INT:
	intvalp = (int *) rec_start;
	break;
      case NC_FLOAT:
	floatvalp = (float *) rec_start;
	break;
      case NC_DOUBLE:
	doublevalp = (double *) rec_start;
	break;
      case NC_UBYTE:
	ubytevalp = (unsigned char *) rec_start;
	break;
      case NC_USHORT:
	ushortvalp = (unsigned short *) rec_start;
	break;
      case NC_UINT:
	uintvalp = (unsigned int *) rec_start;
	break;
      case NC_INT64:
	int64valp = (long long *) rec_start;
	break;
      case NC_UINT64:
	uint64valp = (unsigned long long *) rec_start;
	break;
      default:
	derror("Unhandled type %d\n", vars[varnum].type);
	break;
    }

    start = (MPI_Offset*) malloc(sizeof(MPI_Offset) * vars[varnum].ndims * 2);
    count = start + vars[varnum].ndims;

    if (vars[varnum].ndims > 0) {
	/* initialize start to upper left corner (0,0,0,...) */
	start[0] = 0;
	if (vars[varnum].dims[0] == rec_dim) {
	    count[0] = vars[varnum].nrecs;
	}
	else {
	    count[0] = dims[vars[varnum].dims[0]].size;
	}
    }

    for (idim = 1; idim < vars[varnum].ndims; idim++) {
	start[idim] = 0;
	count[idim] = dims[vars[varnum].dims[idim]].size;
    }

    total_size = nctypesize(vars[varnum].type);
    for (idim=0; idim<vars[varnum].ndims; idim++)
        total_size *= count[idim];

    /* If the total put size is more than 2GB, then put one subarray at a time.
     * Here the subarray is from 1, 2, ... ndims, except 0.
     * This is not a perfect solution. To be improved.
     */
    if (total_size > INT_MAX) {
        MPI_Offset nchunks=count[0];
        MPI_Offset subarray_nelems=1;
        for (idim=1; idim<vars[varnum].ndims; idim++)
            subarray_nelems *= count[idim];

        count[0] = 1;
        switch (vars[varnum].type) {
            case NC_BYTE:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_schar_all(ncid, varnum, start, count, (signed char *)charvalp);
                     check_err(stat, "ncmpi_put_vara_schar_all", __func__, __LINE__, __FILE__);
                     charvalp += subarray_nelems;
                 }
                 break;
            case NC_CHAR:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_text_all(ncid, varnum, start, count, charvalp);
                     check_err(stat, "ncmpi_put_vara_text_all", __func__, __LINE__, __FILE__);
                     charvalp += subarray_nelems;
                 }
                 break;
            case NC_SHORT:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_short_all(ncid, varnum, start, count, shortvalp);
                     check_err(stat, "ncmpi_put_vara_short_all", __func__, __LINE__, __FILE__);
                     shortvalp += subarray_nelems;
                 }
                 break;
            case NC_INT:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_int_all(ncid, varnum, start, count, intvalp);
                     check_err(stat, "ncmpi_put_vara_int_all", __func__, __LINE__, __FILE__);
                     intvalp += subarray_nelems;
                 }
                 break;
            case NC_FLOAT:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_float_all(ncid, varnum, start, count, floatvalp);
                     check_err(stat, "ncmpi_put_vara_float_all", __func__, __LINE__, __FILE__);
                     floatvalp += subarray_nelems;
                 }
                 break;
            case NC_DOUBLE:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_double_all(ncid, varnum, start, count, doublevalp);
                     check_err(stat, "ncmpi_put_vara_double_all", __func__, __LINE__, __FILE__);
                     doublevalp += subarray_nelems;
                 }
                 break;
            case NC_UBYTE:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_uchar_all(ncid, varnum, start, count, ubytevalp);
                     check_err(stat, "ncmpi_put_vara_uchar_all", __func__, __LINE__, __FILE__);
                     ubytevalp += subarray_nelems;
                 }
                 break;
            case NC_USHORT:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_ushort_all(ncid, varnum, start, count, ushortvalp);
                     check_err(stat, "ncmpi_put_vara_ushort_all", __func__, __LINE__, __FILE__);
                     ushortvalp += subarray_nelems;
                 }
                 break;
            case NC_UINT:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_uint_all(ncid, varnum, start, count, uintvalp);
                     check_err(stat, "ncmpi_put_vara_uint_all", __func__, __LINE__, __FILE__);
                     uintvalp += subarray_nelems;
                 }
                 break;
            case NC_INT64:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_longlong_all(ncid, varnum, start, count, int64valp);
                     check_err(stat, "ncmpi_put_vara_longlong_all", __func__, __LINE__, __FILE__);
                     int64valp += subarray_nelems;
                 }
                 break;
            case NC_UINT64:
                 for (i=0; i<nchunks; i++) {
                     start[0] = i;
                     stat = ncmpi_put_vara_ulonglong_all(ncid, varnum, start, count, uint64valp);
                     check_err(stat, "ncmpi_put_vara_ulonglong_all", __func__, __LINE__, __FILE__);
                     uint64valp += subarray_nelems;
                 }
                 break;
            default:
                     derror("Unhandled type %d\n", vars[varnum].type);
                     break;
        }
    }
    else {
        switch (vars[varnum].type) {
            case NC_BYTE:
                stat = ncmpi_put_vara_schar_all(ncid, varnum, start, count, (signed char *)charvalp);
                check_err(stat, "ncmpi_put_vara_schar_all", __func__, __LINE__, __FILE__);
                break;
            case NC_CHAR:
                stat = ncmpi_put_vara_text_all(ncid, varnum, start, count, charvalp);
                check_err(stat, "ncmpi_put_vara_text_all", __func__, __LINE__, __FILE__);
                break;
            case NC_SHORT:
                stat = ncmpi_put_vara_short_all(ncid, varnum, start, count, shortvalp);
                check_err(stat, "ncmpi_put_vara_short_all", __func__, __LINE__, __FILE__);
                break;
            case NC_INT:
                stat = ncmpi_put_vara_int_all(ncid, varnum, start, count, intvalp);
                check_err(stat, "ncmpi_put_vara_int_all", __func__, __LINE__, __FILE__);
                break;
            case NC_FLOAT:
                stat = ncmpi_put_vara_float_all(ncid, varnum, start, count, floatvalp);
                check_err(stat, "ncmpi_put_vara_float_all", __func__, __LINE__, __FILE__);
                break;
            case NC_DOUBLE:
                stat = ncmpi_put_vara_double_all(ncid, varnum, start, count, doublevalp);
                check_err(stat, "ncmpi_put_vara_double_all", __func__, __LINE__, __FILE__);
                break;
            case NC_UBYTE:
                stat = ncmpi_put_vara_uchar_all(ncid, varnum, start, count, ubytevalp);
                check_err(stat, "ncmpi_put_vara_uchar_all", __func__, __LINE__, __FILE__);
                break;
            case NC_USHORT:
                stat = ncmpi_put_vara_ushort_all(ncid, varnum, start, count, ushortvalp);
                check_err(stat, "ncmpi_put_vara_ushort_all", __func__, __LINE__, __FILE__);
                break;
            case NC_UINT:
                stat = ncmpi_put_vara_uint_all(ncid, varnum, start, count, uintvalp);
                check_err(stat, "ncmpi_put_vara_uint_all", __func__, __LINE__, __FILE__);
                break;
            case NC_INT64:
                stat = ncmpi_put_vara_longlong_all(ncid, varnum, start, count, int64valp);
                check_err(stat, "ncmpi_put_vara_longlong_all", __func__, __LINE__, __FILE__);
                break;
            case NC_UINT64:
                stat = ncmpi_put_vara_ulonglong_all(ncid, varnum, start, count, uint64valp);
                check_err(stat, "ncmpi_put_vara_ulonglong_all", __func__, __LINE__, __FILE__);
                break;
            default:
                derror("Unhandled type %d\n", vars[varnum].type);
                break;
        }
    }
    free(start);
}
