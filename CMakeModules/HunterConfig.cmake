# This file is parsed by HunterGate command

hunter_config(luabind
  VERSION 0.9.1
  URL  "https://github.com/nimetu/luabind/tarball/2fa4606"
  SHA1 "3b4646bab9f0b2362d7b8d71d78e40deaf3cc747"
  )

hunter_config(Boost
  VERSION 1.79.0
  URL "https://archives.boost.io/release/1.79.0/source/boost_1_79_0.tar.bz2"
  SHA1 "31209dcff292bd6a64e5e08ceb3ce44a33615dc0"
)

hunter_config(breakpad
    VERSION 0.0.0-12ecff3-p4
    CMAKE_ARGS
        "DIASDK_INCLUDE_DIR=C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/DIA SDK/include"
)

# needs C++20, and does not configure without the embedded HRTF data
hunter_config(OpenAL
  VERSION 1.25.2
  URL "https://github.com/kcat/openal-soft/archive/1.25.2.tar.gz"
  SHA1 "22fcefb486d598f748c021296a5d7b683e78069c"
  CMAKE_ARGS
    ALSOFT_ENABLE_MODULES=OFF
    ALSOFT_EMBED_HRTF_DATA=ON
    ALSOFT_INSTALL_CONFIG=OFF
    ALSOFT_INSTALL_HRTF_DATA=OFF
    ALSOFT_INSTALL_AMBDEC_PRESETS=OFF
)