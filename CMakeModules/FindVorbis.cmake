INCLUDE(FindHelpers)

FIND_PACKAGE_HELPER(Vorbis vorbis/vorbisenc.h)
FIND_PACKAGE_HELPER(VorbisFile vorbis/vorbisfile.h SUFFIXES vorbis)

IF(VORBISFILE_FOUND)
  add_library(Vorbis::vorbis UNKNOWN IMPORTED)
  set_target_properties(Vorbis::vorbis PROPERTIES
          IMPORTED_LOCATION "${VORBIS_LIBRARIES}"
          INTERFACE_INCLUDE_DIRECTORIES "${VORBIS_INCLUDE_DIR}"
  )

  add_library(Vorbis::vorbisfile UNKNOWN IMPORTED)
  set_target_properties(Vorbis::vorbisfile PROPERTIES
          IMPORTED_LOCATION "${VORBISFILE_LIBRARIES}"
          INTERFACE_INCLUDE_DIRECTORIES "${VORBIS_INCLUDE_DIR}"
  )
  set(Vorbis_FOUND ON)
ENDIF()
