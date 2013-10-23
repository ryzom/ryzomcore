# TODO add install for the .txt and .cfg
ADD_SUBDIRECTORY(plugin_max)

INSTALL(FILES ligoscape.cfg DESTINATION maxplugin/plugins)
INSTALL(FILES ligoscape.txt DESTINATION maxplugin/docs)