# SPDX-License-Identifier: MIT

#[=======================================================================[.rst:
FindLibserialport
-------

Finds the sigrok serial port library (``libserialport``)

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following imported targets, if found:

``Libserialport::Libserialport``
  The serialport library

Result Variables
^^^^^^^^^^^^^^^^

This module will define the following variables:

``Libserialport_FOUND``
  True if the system has the serialport library.
``Libserialport_VERSION``
  The version of the serialport library which was found.
``Libserialport_INCLUDE_DIRS``
  Include directories needed to use ``libserialport``.
``Libserialport_LIBRARIES``
  Libraries needed to link to ``libserialport``.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Libserialport_INCLUDE_DIR``
  The directory containing ``libserialport.h``.
``Libserialport_LIBRARY``
  The path to the ``libserialport`` library.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_Libserialport QUIET libserialport)

find_path(Libserialport_INCLUDE_DIR
    NAMES libserialport.h
    PATHS "${PC_Libserialport_INCLUDE_DIRS}"
)
find_library(Libserialport_LIBRARY
    NAMES serialport
    HINTS "${PC_Libserialport_LIBRARY_DIRS}"
)

set(Foo_VERSION ${PC_Foo_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libserialport
    FOUND_VAR Libserialport_FOUND
    REQUIRED_VARS
        Libserialport_LIBRARY
        Libserialport_INCLUDE_DIR
    VERSION_VAR Libserialport_VERSION
)

if(Libserialport_FOUND)
    set(Libserialport_LIBRARIES ${Libserialport_LIBRARY})
    set(Libserialport_INCLUDE_DIRS ${Libserialport_INCLUDE_DIR})
    set(Libserialport_DEFINITIONS ${PC_Liberialport_CFLAGS_OTHER})
endif()

if(Libserialport_FOUND AND NOT TARGET Libserialport::Libserialport)
    add_library(Libserialport::Libserialport UNKNOWN IMPORTED)
    set_target_properties(Libserialport::Libserialport PROPERTIES
        IMPORTED_LOCATION "${Libserialport_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Libserialport_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Libserialport_INCLUDE_DIR}"
    )
endif()
