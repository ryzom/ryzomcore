ADD_EXECUTABLE(csv_transform csv_transform.cpp)

TARGET_INCLUDE_DIRECTORIES(csv_transform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(csv_transform
        PRIVATE
        NeL::misc
)
NL_DEFAULT_PROPS(csv_transform "Ryzom, Tools, Misc: CSV Transform")
NL_ADD_RUNTIME_FLAGS(csv_transform)

INSTALL(TARGETS csv_transform RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
