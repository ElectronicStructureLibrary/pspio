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
 * @file read_upf.c
 * @brief implementation to read in UPF files 
 */
#include <string.h>
#include <ctype.h>
#include "upf.h"
#include "util.h"

#if defined HAVE_CONFIG_H
#include "config.h"
#endif


int pspio_upf_init(FILE *fp, pspio_pspdata_t **pspdata){
  int np, has_nlcc;  

  HANDLE_FUNC_ERROR(upf_read_header(fp, &np, pspdata));

  HANDLE_FUNC_ERROR(upf_read_mesh(fp, np, pspdata));

  HANDLE_FUNC_ERROR( pspio_xc_has_nlcc((*pspdata)->xc, &has_nlcc));
  if (has_nlcc) {
    HANDLE_FUNC_ERROR(upf_read_nlcc(fp, np, pspdata));
  }

  HANDLE_FUNC_ERROR(upf_read_nonlocal(fp, np, pspdata));

  HANDLE_FUNC_ERROR(upf_read_local(fp, np, pspdata));

  HANDLE_FUNC_ERROR(upf_read_pswfc(fp, np, pspdata));

  HANDLE_FUNC_ERROR(upf_read_rhoatom(fp, np, pspdata));

  return PSPIO_SUCCESS;
}


int upf_read_header(FILE *fp, int *np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  int version_number, i;
  char *kind_ps;
  char nlcc_flag;
  char xc_string[MAX_STRLEN];
  int exchange, correlation;
  double total_energy, wfc_cutoff, rho_cutoff;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp,"PP_HEADER", GO_BACK));
  
  //Read the version number
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%d", &version_number) != 1, PSPIO_EIO);
 
  //Read the atomic symbol
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  (*pspdata)->symbol = (char *)malloc(2*sizeof(char));
  HANDLE_FATAL_ERROR((*pspdata)->symbol == NULL,PSPIO_ENOMEM);
  strcpy((*pspdata)->symbol, strtok(line," "));
  HANDLE_FUNC_ERROR( symbol_to_z((*pspdata)->symbol, (*pspdata)->z) );

  //Read the kind of pseudo-potentials US|NC|PAW
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  kind_ps = (char *)malloc(3*sizeof(char));
  HANDLE_FATAL_ERROR(kind_ps == NULL, PSPIO_ENOMEM);
  kind_ps = strtok(line," ");
  if (strncmp(kind_ps,"NC",2)) {
    //At the moment LIBPSP_IO can only read norm-conserving pseudo-potentials
    HANDLE_ERROR(PSPIO_ENOSUPPORT);
  }
  
  // read the nonlinear core correction
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%1s", &nlcc_flag) != 1, PSPIO_EIO);

  //Exchange-correlation functional
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  strncpy(xc_string, line, 21); //the xc string should always the first 21 chars of the line
  HANDLE_FUNC_ERROR( upf_to_libxc(xc_string, &exchange, &correlation));

  // read the Z valence
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%lf",&(*pspdata)->zvalence) != 1, PSPIO_EIO);

  // read the total energy
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%lf",&total_energy) != 1, PSPIO_EIO);
  
  //read the suggested cutoff for wfc and rho
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%lf %lf",&wfc_cutoff,&rho_cutoff) != 2, PSPIO_EIO);
  
  // read the max angular momentun component
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%d",&(*pspdata)->l_max) != 1, PSPIO_EIO);
  
  // read the number of points in mesh
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%d",np) != 1, PSPIO_EIO);
  
  // read the number of wavefunctions and projectors 
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line, "%d %d", &(*pspdata)->n_states, &(*pspdata)->n_kbproj) != 2, PSPIO_EIO);

  //Skip info on wavefunctions, as it is repeated in the PP_PSWFC block
  for (i=0; i<(*pspdata)->n_states+1; i++) {
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  }

  // Initialize xc
  if (strcmp(&nlcc_flag, "T") == 0) {
    HANDLE_FUNC_ERROR(pspio_xc_alloc(&(*pspdata)->xc, PSPIO_NLCC_UNKNOWN, *np));
  } else {
    HANDLE_FUNC_ERROR(pspio_xc_alloc(&(*pspdata)->xc, PSPIO_NLCC_NONE, *np));
  }
  HANDLE_FUNC_ERROR(pspio_xc_set(&(*pspdata)->xc, exchange, correlation));

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp, "PP_HEADER"));

  return PSPIO_SUCCESS;
}


int upf_read_mesh(FILE *fp, const int np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  int i;
  double *r, *drdi;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp, "PP_MESH", GO_BACK));

  //Allocate memory
  r = (double *)malloc(np*sizeof(double));
  HANDLE_FATAL_ERROR(r == NULL, PSPIO_ENOMEM);

  drdi = (double *)malloc(np*sizeof(double));
  HANDLE_FATAL_ERROR(drdi == NULL, PSPIO_ENOMEM);

  //Read mesh points
  HANDLE_FUNC_ERROR(init_tag(fp, "PP_R", NO_GO_BACK));
  for (i=0; i<np; i+=4) {
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%lf %lf %lf %lf",&r[i],&r[i+1],&r[i+2],&r[i+3]) != 4, PSPIO_EIO);
  }
  HANDLE_FUNC_ERROR(check_end_tag(fp, "PP_R"));
  
  //Read Rab
  HANDLE_FUNC_ERROR(init_tag(fp, "PP_RAB", NO_GO_BACK));
  for (i=0; i<np; i+=4) {
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%lf %lf %lf %lf",&drdi[i],&drdi[i+1],&drdi[i+2],&drdi[i+3]) != 4, PSPIO_EIO);
  }
  HANDLE_FUNC_ERROR(check_end_tag(fp, "PP_RAB"));

  //Store the mesh in the pspdata structure
  HANDLE_FUNC_ERROR(pspio_mesh_alloc(&(*pspdata)->mesh, np));
  HANDLE_FUNC_ERROR(pspio_mesh_init_from_points(&(*pspdata)->mesh, r, drdi));

  //Free memory
  free(r);
  free(drdi);

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp, "PP_MESH")); 

  return PSPIO_SUCCESS;
}


int upf_read_nlcc(FILE *fp, const int np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  int i;
  double *density;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp, "PP_NLCC", GO_BACK));

  //Allocate memory
  density = (double *)malloc(np*sizeof(double));
  HANDLE_FATAL_ERROR(density == NULL,PSPIO_ENOMEM);

  //Read core density
  for (i=0; i<np; i+=4) {
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%lf %lf %lf %lf",&density[i],&density[i+1],&density[i+2],&density[i+3]) != 4, PSPIO_EIO);
  }

  //Store the non-linear core corrections in the pspdata structure
  HANDLE_FUNC_ERROR(pspio_xc_nlcc_set(&(*pspdata)->xc, (*pspdata)->mesh, density));

  //Free memory
  free(density);

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp,"PP_NLCC"));

  return PSPIO_SUCCESS;
}

int upf_read_nonlocal(FILE *fp, const int np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  int i, j, l, proj_np, n_dij, ii, jj;
  double *proj_j;
  double energy;
  double *ekb, *projector_read;
  pspio_qn_t *qn = NULL;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp,"PP_NONLOCAL",GO_BACK));

  //Allocate memory
  HANDLE_FUNC_ERROR(pspio_qn_alloc(&qn));

  proj_j = (double *)malloc((*pspdata)->n_kbproj * sizeof(double));
  HANDLE_FATAL_ERROR(proj_j == NULL, PSPIO_ENOMEM);

  projector_read = (double *)malloc(np * sizeof(double));
  HANDLE_FATAL_ERROR(projector_read == NULL, PSPIO_ENOMEM);

  (*pspdata)->kb_projectors = (pspio_projector_t **)malloc( (*pspdata)->n_kbproj*sizeof(pspio_projector_t *));
  HANDLE_FATAL_ERROR( (*pspdata)->kb_projectors == NULL, PSPIO_ENOMEM);
  for (i=0; i<(*pspdata)->n_kbproj; i++) (*pspdata)->kb_projectors[i] = NULL;

  ekb = (double *)malloc((*pspdata)->n_kbproj*sizeof(double));
  HANDLE_FATAL_ERROR(ekb == NULL, PSPIO_ENOMEM);
  memset(ekb, 0, (*pspdata)->n_kbproj*sizeof(double));

  // We start by reading the KB energies, as it is more convinient this way
  HANDLE_FUNC_ERROR(init_tag(fp,"PP_DIJ",NO_GO_BACK));

  //Read the number of n_dij
  HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
  HANDLE_FATAL_ERROR( sscanf(line,"%d",&n_dij) != 1, PSPIO_EIO);

  for (i=0; i<n_dij; i++){
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%d %d %lf", &ii, &jj, &energy) != 3, PSPIO_EIO);
    ASSERT(ii == jj, PSPIO_EVALUE);
    ekb[ii-1] = energy;
  }

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp,"PP_DIJ"));
  HANDLE_FUNC_ERROR(check_end_tag(fp,"PP_NONLOCAL"));

  //There might be extra information in the ADDINFO tag
  if (tag_isdef(fp,"PP_ADDINFO")){
    HANDLE_FUNC_ERROR(init_tag(fp,"PP_ADDINFO",GO_BACK));
    //Skip the lines with the wavefunctions info
    for (i=0; i<(*pspdata)->n_states; i++) {
      HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    }
    //Read j quantum numbers
    for (i=0; i<(*pspdata)->n_kbproj; i++) {
      HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
      HANDLE_FATAL_ERROR( sscanf(line,"%d %lf",&j, &proj_j[i]) != 2, PSPIO_EIO);
    }
  } else {
    for (i=0; i<(*pspdata)->n_kbproj; i++) proj_j[i] = 0.0;
  }

  // Now we go back and read the projector functions
  HANDLE_FUNC_ERROR(init_tag(fp,"PP_NONLOCAL",GO_BACK));

  for (i=0; i<(*pspdata)->n_kbproj; i++){
    HANDLE_FUNC_ERROR(init_tag(fp,"PP_BETA",NO_GO_BACK));

    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%d %d", &ii, &l) != 2, PSPIO_EIO);

    //Read the number of points of projections
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%d",&proj_np) != 1, PSPIO_EIO);

    //Read the projector function
    for (j=0; j<proj_np; j+=4){
      HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
      HANDLE_FATAL_ERROR( sscanf(line,"%lf %lf %lf %lf",&projector_read[j],&projector_read[j+1],&projector_read[j+2],&projector_read[j+3]) != 4, PSPIO_EIO);
    }
    //Fill with zeros, if any left
    for (j=proj_np; j<np; j++) projector_read[i] = 0.0;

    //Store the projectors in the pspdata structure
    HANDLE_FUNC_ERROR(pspio_qn_set(&qn, 0, l, proj_j[i]));
    HANDLE_FUNC_ERROR(pspio_projector_alloc( &((*pspdata)->kb_projectors[i]), np));
    HANDLE_FUNC_ERROR(pspio_projector_set( &(*pspdata)->kb_projectors[i], qn, ekb[i], (*pspdata)->mesh, projector_read));

  }

  //Free memory
  free(projector_read);
  free(ekb);
  free(proj_j);
  HANDLE_FUNC_ERROR (pspio_qn_free(&(qn)));

  return PSPIO_SUCCESS;
}


int upf_read_local(FILE *fp, const int np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  double *vlocal;
  int i, j, n, l;
  pspio_qn_t *qn = NULL;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp, "PP_LOCAL",GO_BACK));

  //Allocate memory
  HANDLE_FUNC_ERROR(pspio_qn_alloc(&qn));

  vlocal = (double *)malloc(np*sizeof(double));
  HANDLE_FATAL_ERROR(vlocal == NULL, PSPIO_ENOMEM);

  //Deduce l local (this is done in a very crude way and it should probably be made more robust)
  (*pspdata)->l_local = -1;
  for (i=0; i<(*pspdata)->l_max+1; i++) {
    n = 0;
    for (j=0; j<(*pspdata)->n_kbproj; j++) {
      HANDLE_FUNC_ERROR(pspio_projector_l( (*pspdata)->kb_projectors[j], &l));
      if (l == i) n++;
    }
    if (n == 0) (*pspdata)->l_local = i;
  }

  //Read local potential
  for (i=0; i<np; i+=4) {
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL,  PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line,"%lf %lf %lf %lf",&vlocal[i],&vlocal[i+1],&vlocal[i+2],&vlocal[i+3]) != 4, PSPIO_EIO);
  }

  //Store the local potential in the pspdata structure
  HANDLE_FUNC_ERROR(pspio_qn_set(&qn, 0, (*pspdata)->l_local, 0.0));
  HANDLE_FUNC_ERROR(pspio_potential_alloc(&(*pspdata)->vlocal, np));
  HANDLE_FUNC_ERROR(pspio_potential_set(&(*pspdata)->vlocal, qn, (*pspdata)->mesh, vlocal));

  //Free memory
  free(vlocal);
  HANDLE_FUNC_ERROR (pspio_qn_free(&(qn)));

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp,"PP_LOCAL"));

  return PSPIO_SUCCESS;
}


int upf_read_pswfc(FILE *fp, const int np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  char label[2];
  int i, j, l;
  double occ;
  double *wf;
  pspio_qn_t *qn = NULL;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp,"PP_PSWFC",GO_BACK));

  //Allocate memory
  HANDLE_FUNC_ERROR(pspio_qn_alloc(&qn));

  wf = (double *)malloc(np * sizeof(double));
  HANDLE_FATAL_ERROR(wf == NULL, PSPIO_ENOMEM);

  (*pspdata)->states = (pspio_state_t **)malloc( (*pspdata)->n_states*sizeof(pspio_state_t *));
  HANDLE_FATAL_ERROR( (*pspdata)->states == NULL, PSPIO_ENOMEM);
  for (j=0; j<(*pspdata)->n_states; j++) (*pspdata)->states[j] = NULL;

  //Read states
  for (j=0; j<(*pspdata)->n_states; j++) {
    //Read the quantum numbers and occupations
    HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
    HANDLE_FATAL_ERROR( sscanf(line, "%2c %d %lf", &label[0], &l, &occ) != 3, PSPIO_EIO);
    HANDLE_FUNC_ERROR( pspio_qn_set(&qn, (int)label[0], l, 0.0));

    //Read wavefunction
    for (i=0; i<np; i+=4) {
      HANDLE_FATAL_ERROR( fgets(line, MAX_STRLEN, fp) == NULL, PSPIO_EIO);
      HANDLE_FATAL_ERROR( sscanf(line,"%lf %lf %lf %lf", &wf[i], &wf[i+1], &wf[i+2], &wf[i+3]) != 4, PSPIO_EIO);
    }

    //Store the state in the pspdata structure
    HANDLE_FUNC_ERROR (pspio_state_alloc( &(*pspdata)->states[j], np));
    HANDLE_FUNC_ERROR (pspio_state_set( &(*pspdata)->states[j], 0.0, label, qn, occ, 0.0, (*pspdata)->mesh, wf));
  }

  //Free memory
  free(wf);
  HANDLE_FUNC_ERROR (pspio_qn_free(&(qn)));

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp,"PP_PSWFC"));

  return PSPIO_SUCCESS;
}


int upf_read_rhoatom(FILE *fp, const int np, pspio_pspdata_t **pspdata){
  char line[MAX_STRLEN];
  int i;
  double *rho_read;

  //Find init tag
  HANDLE_FUNC_ERROR(init_tag(fp,"PP_RHOATOM",GO_BACK));

  //Allocate memory
  rho_read = (double *)malloc(np * sizeof(double));
  HANDLE_FATAL_ERROR(rho_read == NULL, PSPIO_ENOMEM);

  //Read valence density
  for (i=0; i<np; i+=4){
    HANDLE_FATAL_ERROR( (fgets(line, MAX_STRLEN, fp) == NULL), PSPIO_EIO);
    if (sscanf(line,"%lf %lf %lf %lf",&rho_read[i],&rho_read[i+1],&rho_read[i+2],&rho_read[i+3]) != 4) {
      HANDLE_ERROR(PSPIO_EIO);
    }
  }

  //Store the density in the pspdata structure
  HANDLE_FUNC_ERROR(pspio_meshfunc_alloc(&(*pspdata)->rho_valence, np));
  HANDLE_FUNC_ERROR(pspio_meshfunc_set(&(*pspdata)->rho_valence, (*pspdata)->mesh, rho_read));

  //Free memory
  free(rho_read);

  //Check end tag
  HANDLE_FUNC_ERROR(check_end_tag(fp,"PP_RHOATOM"));

  return PSPIO_SUCCESS;
}


int init_tag(FILE * fp, const char * tag, const int go_back){
  char line[MAX_STRLEN];
  char * init_tag = NULL;
  char * read_string = NULL;
  int i;

  if (go_back) rewind(fp);
  
  //Prepare base string
  init_tag = (char *)malloc((strlen(tag)+2) * sizeof(char));
  HANDLE_FATAL_ERROR(init_tag == NULL,PSPIO_ENOMEM);
  strcpy(init_tag, "");
  strcat(init_tag,"<");
  strncat(init_tag,tag,strlen(tag));
  strcat(init_tag,">");
  for (i=0;init_tag[i]; i++)
    init_tag[i] = tolower(init_tag[i]);

  while (fgets(line, sizeof line, fp) != NULL){
    //Skip white spaces
    if (line[0] == ' ')
      read_string = strtok(line," ");
    else 
      read_string = line;
    //Lowercase line
    for (i=0;read_string[i]; i++)
      read_string[i] = tolower(read_string[i]);
    
    if (strncmp(read_string,init_tag,strlen(init_tag))==0) return PSPIO_SUCCESS;
  }
  return PSPIO_EFILE_FORMAT;
}

int check_end_tag(FILE * fp, const char * tag){
  char line[MAX_STRLEN];
  char * ending_tag = NULL;
  char * read_string = NULL;
  int i;
  
  //Prepare base string
  ending_tag = (char *)malloc((strlen(tag)+3) * sizeof(char));
  HANDLE_FATAL_ERROR(ending_tag == NULL,PSPIO_ENOMEM);
  strcpy(ending_tag, "");
  strcat(ending_tag,"</");
  strncat(ending_tag,tag,strlen(tag));
  strcat(ending_tag,">");
  for (i=0;ending_tag[i]; i++)
    ending_tag[i] = tolower(ending_tag[i]);
  
  if(fgets(line, sizeof line, fp) == NULL){
    HANDLE_ERROR(PSPIO_EIO);
  }

  //Skip white spaces
  if (line[0] == ' ')
    read_string = strtok(line," ");
  else 
    read_string = line;
  //Lowercase line
  for (i=0;line[i]; i++)
    read_string[i] = tolower(read_string[i]);

  //Compare with the ending tag
  if (strncmp(read_string,ending_tag,strlen(ending_tag)) == 0) return PSPIO_SUCCESS;
  else {
    printf("PSPIO library is not able to find %s ending tag",ending_tag);
    return PSPIO_EFILE_FORMAT;
  }
}

int tag_isdef(FILE * fp, const char * tag){
  char line[MAX_STRLEN];
  char * init_tag = NULL; 
  char * read_string = NULL;
  int i;
  
  //Go to the beginning of the buffer
  rewind(fp);
  
  //Prepare base string
  init_tag = (char *)malloc((strlen(tag)+2) * sizeof(char));
  HANDLE_FATAL_ERROR(init_tag == NULL,PSPIO_ENOMEM);
  strcpy(init_tag, "");
  strcat(init_tag,"<");
  strncat(init_tag,tag,strlen(tag));
  strcat(init_tag,">");
  for (i=0;init_tag[i]; i++)
    init_tag[i] = tolower(init_tag[i]);
  
  while (fgets(line, sizeof line, fp) != NULL){
    //Skip white spaces
    if (line[0] == ' ')
      read_string = strtok(line," ");
    else 
      read_string = line;
    //Lowercase line
    for (i=0;line[i]; i++)
      line[i] = tolower(line[i]);

    if (strncmp(read_string,init_tag,strlen(init_tag))==0) return 1;
  }
  //End of the buffer reached; so return false
  return 0;
}
