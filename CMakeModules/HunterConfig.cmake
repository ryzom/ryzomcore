# This file is parsed by HunterGate command

# Boost 1.86.0 exactly as Hunter pins it, with the bundled tools/build swapped
# for b2 5.4.2 — the first b2 release whose bootstrap and msvc.jam know the
# Visual Studio 2026 toolset (vc145); 1.86's own b2 dies with "Unknown
# toolset: vcunk" on VS2026 runners. Library sources and headers are
# byte-identical to the upstream archive (luabind compatibility unchanged);
# b2 5.4.2 builds older Boost trees and toolsets fine, so the override is
# uniform across all Hunter platforms.
hunter_config(Boost
  VERSION "1.86.0-p1"
  URL "https://cdn.ryzom.dev/core/boost_1_86_0-b2542.tar.gz"
  SHA1 "d62482baabbf96d70270ba3a5b1fe3d841be1f1d"
)

hunter_config(luabind
  VERSION 0.9.1
  URL  "https://github.com/ryzom/luabind/tarball/6cb0688f78b753ce94df528ac640090251043759"
  SHA1 "620c2b630be02c1061f44745b8a4b1bf03d42fbb"
)

hunter_config(OpenAL
  VERSION "1.24.3"
  URL "https://github.com/kcat/openal-soft/archive/1.24.3.tar.gz"
  SHA1 "5d311a0ed6acded10e1a5eab44de8c18ec5790c5"
)
