#vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO cpptest/cpptest
        # release 2.0.0 does not contain the README fix
        #REF "${VERSION}"
        REF 6cefc1a39367439ed49e5f3eb0d51a59b621c40f
        SHA512 37f224be0de32d02f3c028caa76a0ec7864ae7aac30f0a77d52ed1c6f6e5e2794d4e78f23cea7294a771bb36a9e846942e9ebe04f74906e7449441db3e72c9ec
        HEAD_REF master
)

vcpkg_make_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        AUTORECONF
)

vcpkg_make_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")