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

#include "pspio_meshfunc.h"

#include <stdlib.h>

int pspio_meshfunc_alloc(pspio_meshfunc_t *func, const int np){
  int i, ierr;

  ASSERT (np > 1, PSPIO_ERROR);

  func = (pspio_meshfunc_t *) malloc (sizeof(pspio_meshfunc_t));  
  if (func == NULL) {
    HANDLE_FATAL_ERROR (PSPIO_ENOMEM);
  }

  func->f = (double *) malloc (np * sizeof(double));
  if (func->f == NULL) {
    HANDLE_FATAL_ERROR (PSPIO_ENOMEM);
  }

  ierr = pspio_mesh_alloc(func->mesh, np))
  if (ierr) {
    pspio_meshfunc_free(func);
    HANDLE_ERROR (ierr);
  }
  func->spl = gsl_spline_alloc(gsl_interp_cspline, np);
  func->acc = gsl_interp_accel_alloc();

  for (i = 0; i < np; i++)
    {
      func->f[i] = 0;
    }
  
  return PSPIO_SUCCESS;
}


int pspio_meshfunc_set(pspio_meshfunc_t *func, pspio_mesh_t *mesh, double *f){
  int i;
  
  HANDLE_FUNC_ERROR (pspio_mesh_copy(func->mesh, mesh));

  for (i = 0; i < func->mesh->np; i++)
    {
      func->f[i] = f[i];
    }

  gsl_spline_init(func->spl, func->mesh->r, func->f, func->mesh->np);

  return PSPIO_SUCCESS;
}


int pspio_meshfunc_eval(pspio_meshfunc_t *func, const double r, double f){
  ASSERT (func != NULL, PSPIO_ERROR);
  
  f = gsl_spline_eval(func->spl, r, func->acc);

  return PSPIO_SUCCESS;
}


int pspio_meshfunc_eval_deriv(pspio_meshfunc_t *func, const double r, double fp){
  ASSERT (func != NULL, PSPIO_ERROR);  

  fp = gsl_spline_eval_deriv(func->spl, r, func->acc);

  return PSPIO_SUCCESS;
}


int pspio_meshfunc_eval_deriv2(pspio_meshfunc_t *func, const double r, double fpp){
  ASSERT (func != NULL, PSPIO_ERROR);  

  fpp = gsl_spline_eval_deriv2(func->spl, r, func->acc);

  return PSPIO_SUCCESS;
}


int pspio_meshfunc_free(pspio_meshfunc_t *func){

  if (func != NULL) {
    if (func->f != NULL) free(func->f);
    pspio_mesh_free(func->mesh);
    gsl_spline_free(func->spl);
    gsl_interp_accel_free(func->acc);
    free(func);
  }

  return PSPIO_SUCCESS;
}

