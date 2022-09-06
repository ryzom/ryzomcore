xsltproc -o nel_database_mapping.cpp --stringparam output cpp --stringparam filename nel_database_mapping ../../../common/src/game_share/generate_module_interface.xslt nel_database_mapping.xml
xsltproc -o nel_database_mapping.h --stringparam output header --stringparam filename nel_database_mapping ../../../common/src/game_share/generate_module_interface.xslt nel_database_mapping.xml
