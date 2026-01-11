INCLUDE(FindHelpers)

FIND_PACKAGE_HELPER(STLport iostream
  RELEASE stlport_cygwin stlport_gcc stlport_x stlport_x.5.2 stlport_statix stlport_static
  DEBUG stlport_cygwin_debug stlport_cygwin_stldebug stlport_gcc_debug stlport_gcc_stldebug stlportstld_x stlportstld_x.5.2 stlportd_statix stlportd_static)
