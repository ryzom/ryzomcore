# Dummy project to pre-build dependencies

cmake_minimum_required(VERSION 3.0)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

INCLUDE("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.321.tar.gz"
    SHA1 "5e53cbb0429037ea8e2592bfd92704b8ff3ab492"
    FILEPATH "${CMAKE_CURRENT_LIST_DIR}/cmake/HunterConfig.cmake"
)

PROJECT(HunterDependencies)

SET(ON 1)

HUNTER_ADD_PACKAGE(ZLIB)
HUNTER_ADD_PACKAGE(libxml2)
HUNTER_ADD_PACKAGE(OpenSSL)
HUNTER_ADD_PACKAGE(luabind)
HUNTER_ADD_PACKAGE(CURL)
HUNTER_ADD_PACKAGE(freetype)
HUNTER_ADD_PACKAGE(Jpeg)
HUNTER_ADD_PACKAGE(giflib)
HUNTER_ADD_PACKAGE(PNG)
HUNTER_ADD_PACKAGE(libogg)
HUNTER_ADD_PACKAGE(vorbis)
HUNTER_ADD_PACKAGE(ffmpeg)
HUNTER_ADD_PACKAGE(OpenAL)

# end of file
