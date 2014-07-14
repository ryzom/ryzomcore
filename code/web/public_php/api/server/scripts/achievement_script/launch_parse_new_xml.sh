#!/bin/sh -

cd /home/api/public_html/server/scripts/achievement_script

sudo -u api nohup ./parse_new_xml.sh &
