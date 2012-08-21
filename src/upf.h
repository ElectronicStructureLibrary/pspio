/*
 Copyright (C) 2011-2012 J. Alberdi, M. Oliveira, Y. Pouillon, and M. Verstraete
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or 
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 $Id$
 */

/**
 * @file upf.h
 * @brief header file for UPF related routines
 */

#if !defined UPF_H
#define UPF_H

#include <stdio.h>

#include "pspio_pspdata.h"


/**********************************************************************
 * Defines                                                            *
 **********************************************************************/

#define GO_BACK    1 /**< Defines that it has to go at the beginning of the file */
#define NO_GO_BACK 0 /**< Defines that it has to stay there in the file*/


/**********************************************************************
 * upf_read routines                                                  *
 **********************************************************************/

/** 
 * Read the UPF header
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_header(FILE *fp, int *np, pspio_pspdata_t **pspdata);

/** 
 * Read the mesh
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_mesh(FILE *fp, const int np, pspio_pspdata_t **pspdata);

/** 
 * Read the non-linear core-corrections
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_nlcc(FILE *fp, const int np, pspio_pspdata_t **pspdata);

/** 
 * Read the non-local projectors
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_nonlocal(FILE *fp, const int np, pspio_pspdata_t **pspdata);

/** 
 * Read the local part of the pseudos
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_local(FILE *fp, const int np, pspio_pspdata_t **pspdata);

/** 
 * Read the pseudo-wavefunctions
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_pswfc(FILE *fp, const int np, pspio_pspdata_t **pspdata);

/** 
 * Read the valence electronic charge
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_read_rhoatom(FILE *fp, const int np, pspio_pspdata_t **pspdata);



/**********************************************************************
 * upf_write routines                                                 *
 **********************************************************************/

/** 
 * Write the UPF header
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_header(FILE *fp, const pspio_pspdata_t *pspdata);

/** 
 * Write the mesh
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_mesh(FILE *fp, const pspio_pspdata_t *pspdata);

/** 
 * Write the non-linear core-corrections
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_nlcc(FILE *fp, const pspio_pspdata_t *pspdata);

/** 
 * Write the non-local projectors
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_nonlocal(FILE *fp, const pspio_pspdata_t *pspdata);

/** 
 * Write the local part of the pseudos
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_local(FILE *fp, const pspio_pspdata_t *pspdata);

/** 
 * Write the pseudo-wavefunctions
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_pswfc(FILE *fp, const pspio_pspdata_t *pspdata);

/** 
 * Write the valence electronic charge
 * @param[in] fp a stream of the input file
 * @param[inout] pspdata the data structure
 * @return error code
 */
int upf_write_rhoatom(FILE *fp, const pspio_pspdata_t *pspdata);


/**********************************************************************
 * upf_tag routines                                                   *
 **********************************************************************/

/** 
 * Evaluates if a tag is defined
 * @param[in] fp a stream of the input file
 * @param[in] tag the tag. It is case-insensitive
 * @return 0 if defined, 1 otherwise
 */
int tag_isdef(FILE * fp, const char * tag);


/** 
 * Goes to the point just after the tag 
 * @param[in] fp a stream of the input file
 * @param[in] tag the tag. It is case-insensitive
 * @param[in] go_back decides if it has to go to the beginning of the file
 * @return error code
 */
int init_tag(FILE * fp, const char * tag, const int go_back);

/** 
 * Evaluates if a tag is correctly closed
 * @param[in] fp a stream of the input file
 * @param[in] tag the tag. It is case-insensitive
 * @return 0 if correct, 1 otherwise
 */
int check_end_tag(FILE * fp, const char * tag);


/**********************************************************************
 * upf_xc routines                                                    *
 **********************************************************************/

/**
* subroutine converts pwcf xc string to libxc codes
*@param[in]  xc_string: pwscf string
*@param[out] exchange: libxc code for exchange
*@param[out] correlation: libxc code for correlation
*/
int upf_to_libxc(const char *xc_string, int *exchange, int *correlation);


/**
* subroutine converts libxc codes to pwcf xc string
*@param[in] exchange: libxc code for exchange
*@param[in] correlation: libxc code for correlation
*@param[out]  xc_string: pwscf string
*/
int upf_from_libxc(const int exchange, const int correlation, char *xc_string);

#endif