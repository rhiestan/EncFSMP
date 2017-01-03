# - Find the Pismo File Mount Library (http://www.pismotechnic.com/pfm/)
# This module defines the following variables:
#  PFM_INCLUDE_DIR - include directories for PFM
#  PFM_FOUND - true if PFM has been found and can be used

#=============================================================================
# Copyright 2014 Roman Hiestand
#
# Distributed under the MIT License.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_path(PFM_INCLUDE_DIR pfmapi.h
	HINTS ${PFM_ROOT} $ENV{PFM_ROOT}
	PATH_SUFFIXES include inc)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PFM
                                  REQUIRED_VARS PFM_INCLUDE_DIR)

mark_as_advanced(PFM_INCLUDE_DIR)
