/*
 Copyright (C) 2012-2013 Y. Pouillon

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
 * @file abinit_util.c
 * @brief Utility routines to read/write Abinit files
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "pspio_abinit.h"
#include "pspio_error.h"
#include "abinit.h"
#include "util.h"

#if defined HAVE_CONFIG_H
#include "config.h"
#endif

/* proptotypes for the replacement functions */
static char *
my_strndup (char const *s, size_t n);

static size_t
my_strnlen (const char *string, size_t maxlen);


int abinit_read_header(FILE *fp, int *format, int *np, int *have_nlcc,
  pspio_pspdata_t **pspdata) {
  char line[PSPIO_STRLEN_LINE];
  char *line4;
  int pspcod, pspxc, lmax, lloc, mmax;
  int exchange, correlation, s;
  double zatom, zval, r2well, rchrg, fchrg, qchrg;

  // Init
  *format = PSPIO_FMT_UNKNOWN;
  *np = 0;
  *have_nlcc = 0;

  // Line 1: read title
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);
  s = strlen(line);
  (*pspdata)->info = (char *) malloc (s + 1);
  strncpy((*pspdata)->info, line, s);
  (*pspdata)->info[s] = '\0';

  // Line 2: read atomic number, Z valence
  // Note: ignoring psp date
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);
  CHECK_ERROR( sscanf(line, "%lf %lf", &zatom, &zval) == 2, PSPIO_EFILE_CORRUPT);
  (*pspdata)->z = zatom;
  (*pspdata)->zvalence = zval;

  // Line 3: read pspcod, pspxc, lmax, lloc, mmax, r2well
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);
  CHECK_ERROR( sscanf(line, "%d %d %d %d %d %lf",
    &pspcod, &pspxc, &lmax, &lloc, &mmax, &r2well) == 6, PSPIO_EFILE_CORRUPT);
  *np = mmax;
  (*pspdata)->l_max = lmax;
  (*pspdata)->l_local = lloc;

  // Line 4: read rchrg, fchrg, qchrg if NLCC
  // Note: tolerance copied from Abinit
  // FIXME: store rchrg, fchrg, qchrg
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);
  line4 = my_strndup(line, 3);
  if ( strcmp("4--", line4) != 0 ) {
    CHECK_ERROR( sscanf(line, "%lf %lf %lf",
      &rchrg, &fchrg, &qchrg) == 3, PSPIO_EFILE_CORRUPT );
    if ( abs(fchrg) >= 1.0e-14 ) {
      *have_nlcc = 1;
    }
  }

  // Ignore lines 5-7
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);
  CHECK_ERROR( fgets(line, PSPIO_STRLEN_LINE, fp) != NULL, PSPIO_EIO);

  // FIXME: assuming pspxc = -(exchange * 1000 + correlation)
  if ( pspxc < 0 ) {
    exchange = -pspxc / 1000;
    correlation = -pspxc % 1000;
  } else {
    HANDLE_FUNC_ERROR(abinit_to_libxc(pspxc, &exchange, &correlation));
  }
  if ( have_nlcc ) {
    HANDLE_FUNC_ERROR(pspio_xc_alloc(&(*pspdata)->xc, PSPIO_NLCC_FHI, *np));
  } else {
    HANDLE_FUNC_ERROR(pspio_xc_alloc(&(*pspdata)->xc, PSPIO_NLCC_NONE, *np));
  }
  (*pspdata)->xc->exchange = exchange;
  (*pspdata)->xc->correlation = correlation;

  switch (pspcod) {
    case 1:
      *format = PSPIO_FMT_ABINIT_1;
      break;
    case 2:
      *format = PSPIO_FMT_ABINIT_2;
      break;
    case 3:
      *format = PSPIO_FMT_ABINIT_3;
      break;
    case 4:
      *format = PSPIO_FMT_ABINIT_4;
      break;
    case 5:
      *format = PSPIO_FMT_ABINIT_5;
      break;
    case 6:
      *format = PSPIO_FMT_ABINIT_6;
      break;
    case 7:
      *format = PSPIO_FMT_ABINIT_7;
      break;
    case 8:
      *format = PSPIO_FMT_ABINIT_8;
      break;
    case 9:
      *format = PSPIO_FMT_ABINIT_9;
      break;
    case 10:
      *format = PSPIO_FMT_ABINIT_10;
      break;
    case 11:
      *format = PSPIO_FMT_ABINIT_11;
      break;
    case 17:
      *format = PSPIO_FMT_ABINIT_17;
      break;
    default:
      *format = PSPIO_FMT_UNKNOWN;
  }

  return PSPIO_SUCCESS;
}


int abinit_write_header(FILE *fp, const int format, const pspio_pspdata_t *pspdata) {
  char pspdate[7];
  int pspxc, have_nlcc;
  double rchrg, fchrg, qchrg;
  time_t int_now;
  struct tm *now;

  assert(pspdata != NULL);

  // Init auxiliary data
  pspio_xc_has_nlcc(pspdata->xc, &have_nlcc);
  HANDLE_FUNC_ERROR(libxc_to_abinit(pspdata->xc->exchange, \
    pspdata->xc->correlation, &pspxc));

  // Line 1: write title
  // FIXME: truncate info to its first line
  fprintf(fp, "%s\n", pspdata->info);

  // Line 2: write atomic number, Z valence, psp date
  int_now = time(NULL);
  now = localtime(&int_now);
  sprintf(pspdate, "%2.2d%2.2d%2.2d", 
    (*now).tm_year % 100, (*now).tm_mon + 1, (*now).tm_mday);
  pspdate[6] = '\0';
  fprintf(fp, "%8.3lf %8.3lf   %6s   zatom, zion, pspdat\n",
    pspdata->z, pspdata->zvalence, pspdate);

  // Line 3: write pspcod, pspxc, lmax, lloc, mmax, r2well
  // Line 4: write rchrg, fchrg, qchrg if NLCC
  // Lines 5-7: free comments, unused
  switch ( format ) {
    case 4:
    case 5:
    case 6:
      fprintf(fp, "   %d   %d   %d   %d   %d   %8.3lf   pspcod, pspxc, lmax, lloc, mmax, r2well\n",
        format, pspxc, pspdata->l_max, pspdata->l_local,
        pspdata->mesh->np, 0.0);

      // FIXME: write something relevant
      if ( have_nlcc ) {
        rchrg = 0.0;
        fchrg = 0.0;
        qchrg = 0.0;
      } else {
        rchrg = 0.0;
        fchrg = 0.0;
        qchrg = 0.0;
      }
      fprintf(fp, "%8.3lf   %8.3lf   %8.3lf   rchrg, fchrg, qchrg\n",
        rchrg, fchrg, qchrg);
      fprintf(fp, "5--- These two lines are available for giving more information, later\n6\n");
      fprintf(fp, "7-Here follows the cpi file from the fhi98pp code-\n");
      break;
    default:
      return PSPIO_EFILE_FORMAT;
  }

  return PSPIO_SUCCESS;
}

/* A replacement function, for systems that lack strndup (MacOs) */

char *
my_strndup (char const *s, size_t n)
{
  size_t len = my_strnlen (s, n);
  char *new = malloc (len + 1);

  if (new == NULL)
    return NULL;

  new[len] = '\0';
  return memcpy (new, s, len);
}


/* A replacement function, for systems that lack strnlen. (MacOs)
   Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */

size_t
my_strnlen (const char *string, size_t maxlen)
{
  const char *end = memchr (string, '\0', maxlen);
  return end ? (size_t) (end - string) : maxlen;
}
