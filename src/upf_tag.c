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
 * @file upf_tag.c
 * @brief functions to deal with UPF tags
 */
#include <string.h>
#include <ctype.h>
#include "upf.h"
#include "util.h"

#if defined HAVE_CONFIG_H
#include "config.h"
#endif


int init_tag(FILE * fp, const char * tag, const int go_back){
  char line[MAX_STRLEN];
  char * init_tag = NULL;
  char * read_string = NULL;
  int i;

  if (go_back) rewind(fp);
  
  //Prepare base string
  init_tag = (char *)malloc((strlen(tag)+2) * sizeof(char));
  ASSERT(init_tag != NULL, PSPIO_ENOMEM);
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
  ASSERT( ending_tag != NULL, PSPIO_ENOMEM);
  strcpy(ending_tag, "");
  strcat(ending_tag,"</");
  strncat(ending_tag,tag,strlen(tag));
  strcat(ending_tag,">");
  for (i=0;ending_tag[i]; i++)
    ending_tag[i] = tolower(ending_tag[i]);
  
  ASSERT( fgets(line, sizeof line, fp) != NULL, PSPIO_EIO);
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
  ASSERT( init_tag != NULL, PSPIO_ENOMEM);
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
