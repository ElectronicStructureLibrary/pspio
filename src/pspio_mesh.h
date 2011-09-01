/*
 Copyright (C) 2011 J. Alberdi, M. Oliveira, Y. Pouillon, and M. Verstraete

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
 * @file pspio_mesh.h
 * @brief header file for the handling of the mesh 
 */

#ifndef PSPIO_MESH_H
#define PSPIO_MESH_H

#include "pspio_error.h"


/**********************************************************************
 * Defines                                                            *
 **********************************************************************/

#define MESH_NONE    0 /**< mesh not set yet */
#define MESH_LOG1    1 /**< r_i = b*exp(a*i) */
#define MESH_LOG2    2 /**< r_i = b*(exp(a*i) - 1) */
#define MESH_LINEAR  3 /**< r_i = a*i + b */
#define MESH_UNKNOWN 4 /**< unkown type of mesh */


/**********************************************************************
 * Data structures                                                    *
 **********************************************************************/

/**
* Mesh structure
*/
typedef struct{
  int type;    /**< Type of mesh */
  double a, b; /**< Mesh parameters */
  int np;      /**< Number of points in mesh */
  double *r;   /**< Mesh points */
} pspio_mesh_t;


/**********************************************************************
 * Global routines                                                    *
 **********************************************************************/

/**
 * Allocates memory and preset mesh structure
 * 
 * @param[in,out] mesh: mesh structure
 * @param[in] np: number of points in the mesh
 * @return error code
 * @note np should be larger than 1.
 */
int pspio_mesh_alloc(pspio_mesh_t *mesh, const int np);


/**
 * Sets the mesh data.
 * @param[in,out] mesh: mesh structure to set
 * @param[in] type: type of mesh. Can be LOG1, LOG2, or LINEAR.
 * @param[in] a: parameter a. The meaning depends on the type of mesh.
 * @param[in] b: parameter b. The meaning depends on the type of mesh.
 * @param[in] r: pointer to mesh radial points. Should be of size mesh->np.
 * @return error code
 * @note The mesh pointer has to be allocated first with the pspio_mesh_alloc
 *       method.
 */
int pspio_mesh_set(pspio_mesh_t *mesh, const int type, const double a, 
		   const double b, const double *r);


/**
 * Duplicates a mesh structure
 * 
 * @param[out] dst: destination mesh structure pointer
 * @param[in] src: source mesh structure pointer
 * @return error code
 * @note The src pointer has to be allocated first with the pspio_mesh_alloc 
 *       method.
 * @note The dst pointer might or might not be allocated. If it is not, then it
 *       is allocate here.
 */
int pspio_mesh_copy(pspio_mesh_t *dst, const pspio_mesh_t *src);


/**
 * Sets the mesh data from a list of points. The function will try to determine
 * the type of mesh. If it is not able to do it it will set it to MESH_UNKNOWN.
 * @param[in,out] mesh: mesh structure to set
 * @param[in] r: pointer to mesh radial points
 * @return error code
 * @note The mesh pointer has to be allocated first with the pspio_mesh_alloc
 *       method.
 * @note r should be of size mesh->np.
 */
int pspio_mesh_init_from_points(pspio_mesh_t *mesh, const double *r);


/**
 * Generates the mesh from the mesh type and parameters.
 * @param[in,out] mesh: mesh structure to set
 * @param[in] type: type of mesh. Can be LOG1, LOG2, or LINEAR.
 * @param[in] a: parameter a. The meaning depends on the type of mesh.
 * @param[in] b: parameter b. The meaning depends on the type of mesh.
 * @return error code
 * @note The mesh pointer has to be allocated first with the pspio_mesh_alloc
 *       method.
 */
int pspio_mesh_init_from_parameters(pspio_mesh_t *mesh, const int type, 
				    const double a, const double b);


/**
 * Frees all memory associated with mesh structure
 * 
 * @param[in,out] mesh: mesh structure
 * @return error code
 * @note This function can be safelly called even if some or all of the mesh 
 *       compoments have not been allocated.
 */
int pspio_mesh_free(pspio_mesh_t *mesh);


#endif
