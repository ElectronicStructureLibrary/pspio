#
# Copyright (C) 2012-2015 M. Oliveira, Y. Pouillon
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or 
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#

#
# Makefile for the sources of Libpspio-Fortran
#

                    # ------------------------------------ #

#
# Build parameters
#

AM_CFLAGS = @CFLAGS_COVERAGE@
AM_FCFLAGS = @FCFLAGS_COVERAGE@ -I..

                    # ------------------------------------ #

#
# FRUIT unit test framework
#

noinst_LTLIBRARIES = libfruit.la
libfruit_la_SOURCES = fruit.f90

                    # ------------------------------------ #

#
# Test programs
#
fpio_basic_tests = \
  test_fortran_error \
  test_fortran_info \
  test_fortran_mesh
fpio_xfail_tests = \
  test_fortran_io
fpio_wrapped_tests = \
  test_fortran_io_multiple
fpio_gcov_pre_tests = \
  gcov_fortran_clean
fpio_gcov_post_tests = \
  gcov_fortran_report
fpio_memprof_tests = \
  test_fortran_memory

check_PROGRAMS = $(fpio_basic_tests) $(fpio_xfail_tests)
nodist_check_SCRIPTS = \
  $(fpio_gcov_pre_tests) \
  $(fpio_wrapped_tests) \
  $(fpio_gcov_post_tests) \
  $(fpio_memprof_tests)

test_fortran_error_SOURCES = m_error_test.F90 test_fortran_error.F90
test_fortran_error_LDADD = \
  libfruit.la \
  ../libfpspio.la \
  @pio_pspio_libs@ \
  $(LIBS_COVERAGE)
test_fortran_error_DEPENDENCIES = \
  ../libfpspio.la \
  @pio_pspio_libs@

test_fortran_info_SOURCES = m_info_test.F90 test_fortran_info.F90
test_fortran_info_LDADD = \
  libfruit.la \
  ../libfpspio.la \
  @pio_pspio_libs@ \
  $(LIBS_COVERAGE)
test_fortran_info_DEPENDENCIES = \
  ../libfpspio.la \
  @pio_pspio_libs@

test_fortran_mesh_SOURCES = m_mesh_test.F90 m_mesh_basket.F90 test_fortran_mesh.F90
test_fortran_mesh_LDADD = \
  libfruit.la \
  ../libfpspio.la \
  @pio_pspio_libs@ \
  $(LIBS_COVERAGE)
test_fortran_mesh_DEPENDENCIES = \
  ../libfpspio.la \
  @pio_pspio_libs@

test_fortran_io_SOURCES = test_fortran_io.F90
test_fortran_io_LDADD = \
  ../libfpspio.la \
  @pio_pspio_libs@ \
  $(LIBS_COVERAGE)
test_fortran_io_DEPENDENCIES = \
  ../libfpspio.la \
  @pio_pspio_libs@

TESTS = \
  $(fpio_gcov_pre_tests) \
  $(fpio_basic_tests) \
  $(fpio_xfail_tests) \
  $(fpio_wrapped_tests) \
  $(fpio_gcov_post_tests) \
  $(fpio_memprof_tests)
XFAIL_TESTS = \
  $(fpio_xfail_tests)

                    # ------------------------------------ #

#
# Explicit dependencies
#

# Code coverage
-include gcov-fortran-tests-deps.mf

                    # ------------------------------------ #

#
# Explicit targets
#

# Temporary test outputs
MOSTLYCLEANFILES = *.gcda *.gcno *.gcov test_*.tmp