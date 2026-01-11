#!/bin/bash

# we might want to add some security here, in order to check the parameters

cd /home/atrium-admin/public_html/scripts
nohup /usr/bin/php4 $* >/dev/null &
