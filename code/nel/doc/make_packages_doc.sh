#!/bin/sh

rm -f html/download/neldox.zip
zip -9 -q -r neldox.zip html/nel html/nelns html/tool html/index.html
mv neldox.zip html/download/
