# Runs a Ring scenario file through the editor's Lua without the client: parse, version update, save
# and read back, translation to the run-time form, and the check of the scenario service on the result.
# A console program that works on the scenario files given on the command line; run_corpus.py runs it
# over a directory of them. WITH_RYZOM_R2_SCENARIO_CHAIN and what it needs are in ryzom/CMakeLists.txt.
add_executable(ryzom_r2_scenario_chain
        r2_scenario_chain.cpp
        r2_headless_host.cpp
        r2_headless_host.h
        r2_headless_driver.lua
)

target_include_directories(ryzom_r2_scenario_chain
        PRIVATE
        ${CMAKE_SOURCE_DIR}/ryzom/common/src
)

target_compile_definitions(ryzom_r2_scenario_chain
        PRIVATE
        RYZOM_R2_LUA_DIR="${CMAKE_SOURCE_DIR}/ryzom/common/data_common/r2"
        RYZOM_R2_CHAIN_DRIVER="${CMAKE_CURRENT_SOURCE_DIR}/r2_headless_driver.lua"
)

target_link_libraries(ryzom_r2_scenario_chain
        PRIVATE
        Ryzom::gameshare
        NeL::misc
        NeL::ligo
        NeL::georges
        Lua::Lua
)

# The translations of the interface are not in this repository: they are in the leveldesign data
# (translation/translated), with the creature sheets and sheet_id.bin. Without them the editor's scripts
# get the label of a text for its translation, a creature is taken to have a skeleton and a sheet has no
# name; the output says so, and the tests below pass either way: what they expect does not depend on it.
# With the directory given here they run on the real data.
set(RYZOM_R2_SCENARIO_CHAIN_LEVELDESIGN "" CACHE PATH "Leveldesign directory the scenario chain tests read sheets and translations from (optional)")
set(R2_CHAIN_DATA_ARGUMENTS)
if (RYZOM_R2_SCENARIO_CHAIN_LEVELDESIGN)
    set(R2_CHAIN_DATA_ARGUMENTS --leveldesign "${RYZOM_R2_SCENARIO_CHAIN_LEVELDESIGN}")
endif ()

# Every scenario below is a test twice: once read from its file, as the author loads it, and once as a
# scenario the editor gets from the server in the state of the file (--from-server), as an edition
# session the server kept. The version update runs at another place for each, and both must come to
# what the expectations say.

# The example scenarios of the client go through the whole chain. They are of the versions before this
# editor and have nothing to update: fixtures/examples.expect says that they come out as they went in.
foreach (R2_CHAIN_EXAMPLE "hearty_meal:Example_01_-_A_Hearty_Meal.r2" "plight_of_trigio:Plight_Of_Trigio.r2")
    string(REGEX REPLACE ":.*$" "" R2_CHAIN_EXAMPLE_NAME "${R2_CHAIN_EXAMPLE}")
    string(REGEX REPLACE "^[^:]*:" "" R2_CHAIN_EXAMPLE_FILE "${R2_CHAIN_EXAMPLE}")
    add_test(NAME ryzom_r2_scenario_chain_${R2_CHAIN_EXAMPLE_NAME}
            COMMAND ryzom_r2_scenario_chain "${CMAKE_SOURCE_DIR}/ryzom/client/examples/${R2_CHAIN_EXAMPLE_FILE}"
            --expect-file "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/examples.expect" ${R2_CHAIN_DATA_ARGUMENTS})
    add_test(NAME ryzom_r2_scenario_chain_${R2_CHAIN_EXAMPLE_NAME}_from_server
            COMMAND ryzom_r2_scenario_chain "${CMAKE_SOURCE_DIR}/ryzom/client/examples/${R2_CHAIN_EXAMPLE_FILE}"
            --expect-file "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/examples.expect" --from-server ${R2_CHAIN_DATA_ARGUMENTS})
endforeach ()

# Scenarios written for these tests, one per step of the version update and per corrected translation:
# fixtures/<name>.r2, with what its run must come to in fixtures/<name>.expect and, for a scenario that
# needs them, more options of the program in fixtures/<name>.args (@FIXTURES@ is the fixtures directory).
# A fixtures/<name>.edit file holds a component the author adds before the test (--edit-file).
file(GLOB R2_CHAIN_FIXTURES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/*.r2")
foreach (R2_CHAIN_FIXTURE ${R2_CHAIN_FIXTURES})
    get_filename_component(R2_CHAIN_FIXTURE_NAME "${R2_CHAIN_FIXTURE}" NAME_WE)
    set(R2_CHAIN_FIXTURE_ARGUMENTS)
    set(R2_CHAIN_FIXTURE_ARGUMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/${R2_CHAIN_FIXTURE_NAME}.args")
    if (EXISTS "${R2_CHAIN_FIXTURE_ARGUMENTS_FILE}")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${R2_CHAIN_FIXTURE_ARGUMENTS_FILE}")
        file(STRINGS "${R2_CHAIN_FIXTURE_ARGUMENTS_FILE}" R2_CHAIN_FIXTURE_ARGUMENTS_TEXT LIMIT_COUNT 1)
        string(REPLACE "@FIXTURES@" "${CMAKE_CURRENT_SOURCE_DIR}/fixtures" R2_CHAIN_FIXTURE_ARGUMENTS_TEXT "${R2_CHAIN_FIXTURE_ARGUMENTS_TEXT}")
        separate_arguments(R2_CHAIN_FIXTURE_ARGUMENTS NATIVE_COMMAND "${R2_CHAIN_FIXTURE_ARGUMENTS_TEXT}")
    endif ()
    add_test(NAME ryzom_r2_scenario_chain_fixture_${R2_CHAIN_FIXTURE_NAME}
            COMMAND ryzom_r2_scenario_chain "${R2_CHAIN_FIXTURE}"
            --expect-file "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/${R2_CHAIN_FIXTURE_NAME}.expect"
            ${R2_CHAIN_FIXTURE_ARGUMENTS} ${R2_CHAIN_DATA_ARGUMENTS})
    add_test(NAME ryzom_r2_scenario_chain_fixture_${R2_CHAIN_FIXTURE_NAME}_from_server
            COMMAND ryzom_r2_scenario_chain "${R2_CHAIN_FIXTURE}"
            --expect-file "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/${R2_CHAIN_FIXTURE_NAME}.expect"
            --from-server ${R2_CHAIN_FIXTURE_ARGUMENTS} ${R2_CHAIN_DATA_ARGUMENTS})
endforeach ()

if (TARGET unit_test)
    add_dependencies(unit_test ryzom_r2_scenario_chain)
endif ()
