#
#

MAINTAINERCLEANFILES  = Makefile.in

lib_LTLIBRARIES       = libnelpacs.la

libnelpacs_la_SOURCES = chain.cpp                          \
                        chain.h                            \
                        chain_quad.cpp                     \
                        chain_quad.h                       \
                        collision_callback.cpp             \
                        collision_callback.h               \
                        collision_desc.cpp                 \
                        collision_desc.h                   \
                        collision_mesh_build.h             \
                        collision_ot.cpp                   \
                        collision_ot.h                     \
                        collision_surface_temp.cpp         \
                        collision_surface_temp.h           \
                        edge_collide.cpp                   \
                        edge_collide.h                     \
                        edge_quad.cpp                      \
                        edge_quad.h                        \
                        exterior_mesh.cpp                  \
                        exterior_mesh.h                    \
                        global_retriever.cpp               \
                        global_retriever.h                 \
                        local_retriever.cpp                \
                        local_retriever.h                  \
                        move_cell.cpp                      \
                        move_cell.h                        \
                        move_container.cpp                 \
                        move_container.h                   \
                        move_container_inline.h            \
                        move_element.cpp                   \
                        move_element.h                     \
                        move_element_inline.h              \
                        move_primitive.cpp                 \
                        move_primitive.h                   \
                        primitive_world_image.cpp          \
                        primitive_world_image.h            \
                        primitive_block_pacs.cpp           \
                        primitive_block.h                  \
                        retrievable_surface.cpp            \
                        retrievable_surface.h              \
                        retriever_bank.cpp                 \
                        retriever_bank.h                   \
                        retriever_instance.cpp             \
                        retriever_instance.h               \
			stdpacs.cpp			   \
                        surface_quad.cpp                   \
                        surface_quad.h                     \
                        vector_2s.cpp                      \
                        vector_2s.h                        \
                        build_indoor.cpp                   \
                        build_indoor.h

AM_CXXFLAGS           = -I$(top_srcdir)/src

noinst_HEADERS	      = stdpacs.h face_grid.h quad_grid.h

libnelpacs_la_LIBADD  = -lc

libnelpacs_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

