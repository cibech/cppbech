#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
SET(CMAKE_IMPORT_FILE_VERSION 1)

# Compute the installation prefix relative to this file.
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "png14" for configuration "Debug"
SET_PROPERTY(TARGET png14 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
SET_TARGET_PROPERTIES(png14 PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/libpng14d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "zlibd.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/libpng14d.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS png14 )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_png14 "${_IMPORT_PREFIX}/lib/libpng14d.lib" "${_IMPORT_PREFIX}/bin/libpng14d.dll" )

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "png14_static" for configuration "Debug"
SET_PROPERTY(TARGET png14_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
SET_TARGET_PROPERTIES(png14_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "zlibd.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libpng14_staticd.lib"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS png14_static )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_png14_static "${_IMPORT_PREFIX}/lib/libpng14_staticd.lib" )

# Loop over all imported files and verify that they actually exist
FOREACH(target ${_IMPORT_CHECK_TARGETS} )
  FOREACH(file ${_IMPORT_CHECK_FILES_FOR_${target}} )
    IF(NOT EXISTS "${file}" )
      MESSAGE(FATAL_ERROR "The imported target \"${target}\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    ENDIF()
  ENDFOREACH()
  UNSET(_IMPORT_CHECK_FILES_FOR_${target})
ENDFOREACH()
UNSET(_IMPORT_CHECK_TARGETS)

# Cleanup temporary variables.
SET(_IMPORT_PREFIX)

# Commands beyond this point should not need to know the version.
SET(CMAKE_IMPORT_FILE_VERSION)
