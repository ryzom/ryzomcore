rem Only the Max 2010 plugins are built (Toolchain/PluginMax/Versions), the plugins for the later versions do not work at the moment
rem cmd /C "call nel_plugin_max_9_x86"
cmd /C "call nel_plugin_max_2010_x86"
rem cmd /C "call nel_plugin_max_2012_x86"
rem cmd /C "call nel_plugin_max_2017_x64"
rem cmd /C "call nel_plugin_max_2018_x64"
rem cmd /C "call nel_plugin_max_2019_x64"
rem cmd /C "call nel_plugin_max_2020_x64"
rem cmd /C "call nel_plugin_max_2022_x64"
rem cmd /C "call nel_plugin_max_2023_x64"
call all_nomax.bat
