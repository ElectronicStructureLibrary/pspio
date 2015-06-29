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

*/

/**
 * @file pspio_potential.h
 * @brief header file for the handling of the potentials
 */

#ifndef PSPIO_POTENTIAL_H
#define PSPIO_POTENTIAL_H

#include "pspio_error.h"
#include "pspio_mesh.h"
#include "pspio_meshfunc.h"
#include "pspio_qn.h"


/**********************************************************************
 * Data structures                                                    *
 **********************************************************************/

/**
 * Potential structure
 */
typedef struct{
  pspio_qn_t *qn;      /**< struct with quantum numbers n l j for the potential */
  pspio_meshfunc_t *v; /**< pseudopotential, on a radial mesh */
} pspio_potential_t;


/**********************************************************************
 * Global routines                                                    *
 **********************************************************************/

/**
 * Allocates memory and preset potential structure
 * 
 * @param[in,out] potential: potential structure
 * @param[in] np: number of points in mesh
 * @return error code
 * @note np should be larger than 1.
 */
int pspio_potential_alloc(pspio_potential_t **potential, const int np);


/**
 * Initializes the potential data.
 * @param[in,out] potential: potential structure to be initialized
 * @param[in] qn: pointer to quantum numbers
 * @param[in] mesh: the mesh structure
 * @param[in] vofr: values of the potential on the mesh
 * @return error code
 * @note The potential pointer has to be allocated first with the
 *       pspio_potential_alloc method.
 */
int pspio_potential_init(pspio_potential_t *potential, const pspio_qn_t *qn, 
			const pspio_mesh_t *mesh, const double *vofr);


/**
 * Duplicates a potential structure.
 * @param[out] dst: destination potential structure pointer
 * @param[in] src: source potential structure pointer 
 * @return error code
 * @note The src pointer has to be allocated first with the pspio_potential_alloc 
 *       method.
 * @note The dst pointer might or might not be allocated first. If it is not,
 *        then it is allocated here.
 */
int pspio_potential_copy(pspio_potential_t **dst, const pspio_potential_t *src);


/**
 * Frees all memory associated with potential structure
 * 
 * @param[in,out] potential: potential structure
 * @note This function can be safelly called even if some or all of the 
 *       potential components have not been allocated.
 */
void pspio_potential_free(pspio_potential_t *potential);


/**********************************************************************
 * Atomic routines                                                    *
 **********************************************************************/

/**
 * Returns the value of the potential at an arbitrary point
 * 
 * @param[in] potential: potential structure
 * @param[in] r: point were we want to evaluate the function
 * @return value of the potential at r
 * @note The potential pointer has to be fully set.
 */
double pspio_potential_eval(const pspio_potential_t *potential, double r);


/**
 * Returns the value of the derivative of the potential at an arbitrary point
 * 
 * @param[in] potential: potential structure
 * @param[in] r: point were we want to evaluate the derivative of the potential
 * @return value of the derivative at r
 * @note The potential pointer has to be fully set.
 */
double pspio_potential_eval_deriv(const pspio_potential_t *potential, double r);


/**
 * Returns the value of the second derivative of the potential at an arbitrary point
 * 
 * @param[in] potential: potential structure
 * @param[in] r: point were we want to evaluate the second derivative of the potential
 * @return value of the second derivative at r
 * @note The potential pointer has to be fully set.
 */
double pspio_potential_eval_deriv2(const pspio_potential_t *potential, double r);


/**
 * Returns the quantum numbers of the potential
 * 
 * @param[in] potential: potential structure
 * @return pointer to the potential quantum numbers
 * @note The potential pointer has to be fully set.
 */
pspio_qn_t *pspio_potential_get_qn(const pspio_potential_t *potential);


#endif
