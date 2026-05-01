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