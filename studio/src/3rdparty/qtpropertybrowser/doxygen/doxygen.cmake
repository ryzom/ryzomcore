# Generates doxygen.
find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation. Please install it correctly")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/html/index.html 
                   COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                   MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in
                   COMMENT "Generating API documentation with Doxygen")

add_custom_target(doc ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/html/index.html )

install( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Doxygen DESTINATION ${CMAKE_INSTALL_PREFIX}/doc )
