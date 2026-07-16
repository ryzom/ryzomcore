# This file is parsed by HunterGate command

hunter_config(luabind
  VERSION 0.9.1
  URL  "https://github.com/ryzom/luabind/tarball/0ae9bd6e40fe6c70e9d032ff096370929f58c143"
  SHA1 "1dfabfa89ee72066118e4e28e797830e118d2a9b"
)

hunter_config(OpenAL
  VERSION "1.24.3"
  URL "https://github.com/kcat/openal-soft/archive/1.24.3.tar.gz"
  SHA1 "5d311a0ed6acded10e1a5eab44de8c18ec5790c5"
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
