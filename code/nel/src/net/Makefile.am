#
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelnet.la

libnelnet_la_SOURCES = buf_client.cpp                      \
                       buf_net_base.cpp                    \
                       buf_server.cpp                      \
                       buf_sock.cpp                        \
                       callback_client.cpp                 \
                       callback_net_base.cpp               \
                       callback_server.cpp                 \
                       dummy_tcp_sock.cpp                  \
                       inet_address.cpp                    \
                       listen_sock.cpp                     \
                       login_client.cpp                    \
                       login_cookie.cpp                    \
                       login_server.cpp                    \
                       message.cpp                         \
                       message_recorder.cpp                \
                       naming_client.cpp                   \
                       net_displayer.cpp                   \
                       net_log.cpp                         \
                       service.cpp                         \
                       sock.cpp                            \
                       tcp_sock.cpp                        \
                       udp_sock.cpp                        \
                       udp_sim_sock.cpp                    \
                       unitime.cpp                         \
                       unified_network.cpp                 \
                       varpath.cpp			   \
                       transport_class.cpp                 \
                       email.cpp                           \
                       admin.cpp                           \
		       stdin_monitor_thread.cpp		   \
		       stdin_monitor_thread.h		   \
                       module.cpp                          \
                       module_common.cpp                   \
                       module_gateway.cpp                  \
                       module_manager.cpp                  \
                       module_message.cpp                  \
					   module_socket.cpp                   \
					   module_gateway_transport.cpp		   \
					   module_l5_transport.cpp			\
					   module_local_gateway.cpp \
					   stdnet.cpp

noinst_HEADERS	     = stdnet.h

AM_CXXFLAGS          = -I$(top_srcdir)/src

libnelnet_la_LIBADD  = -lc -lpthread

libnelnet_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

