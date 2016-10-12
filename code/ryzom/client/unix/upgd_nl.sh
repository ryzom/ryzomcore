#!/bin/sh

if [ -z "$ROOTPATH" ]
then
  echo "upgd_nl.sh can only be launched from updt_nl.sh"
  exit 1
fi

# executable flag for all executables
chmod +x "$ROOTPATH/ryzom_client"
chmod +x "$ROOTPATH/crash_report"
chmod +x "$ROOTPATH/ryzom_client_patcher"
chmod +x "$ROOTPATH/ryzom_configuration_qt"
chmod +x "$ROOTPATH/ryzom_installer_qt"

exit 0
