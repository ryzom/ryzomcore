# The rules of the shard unifier that need no database: what the permission rows of an account give it on a
# shard, and the choice of the mainland shard for the character selection. (The other sources of this folder
# are tests of an earlier test framework and are not built.)
add_test_executable(ryzom_shard_unifier_shard_access
        shard_access_test.cpp
        ${RZ_SERVER_SRC_DIR}/shard_unifier_service/shard_access.cpp
)

target_include_directories(ryzom_shard_unifier_shard_access
        PRIVATE
        ${RZ_SERVER_SRC_DIR}
        ${RZ_SERVER_SRC_DIR}/shard_unifier_service
)

target_link_libraries(ryzom_shard_unifier_shard_access
        PRIVATE
        NeL::misc
)
