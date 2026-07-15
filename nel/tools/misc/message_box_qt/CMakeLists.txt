FILE(GLOB MESSAGE_BOX_SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${MESSAGE_BOX_SRC})

ADD_EXECUTABLE(message_box_qt WIN32)
target_sources(message_box_qt
        PRIVATE
        main.cpp
        main.rc
)
TARGET_LINK_LIBRARIES(message_box_qt PRIVATE Qt::Core Qt::Widgets NeL::misc)
NL_DEFAULT_PROPS(message_box_qt "NeL, Tools, Misc: Qt Message Box")
NL_ADD_RUNTIME_FLAGS(message_box_qt)

INSTALL(TARGETS message_box_qt RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)

