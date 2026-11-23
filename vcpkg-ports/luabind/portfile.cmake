#vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# a commit, never a branch: the SHA512 stops matching when the branch moves; the same revision as
# .github/scripts/build-luabind.sh, the Dockerfiles and CMakeModules/HunterConfig.cmake
vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO ryzom/luabind
        REF af52fcf7cc903437d739ee2c9bee92e6025b4908
        SHA512 2861c5b3a4b925cac58972e0a400ea352d4c64e0c5c414a603a647a8f67fd0c4cbf4a43a71c0deb4cce9042a8a046c75742fcb2012a7455ebb930bb7570c3271
        HEAD_REF master
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

#vcpkg_cmake_config_fixup(PACKAGE_NAME "Luabind")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")