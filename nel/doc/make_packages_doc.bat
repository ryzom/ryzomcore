hhc nel.hhp
hhc nelns.hhp
hhc neltools.hhp

del html\download\*.chm
move *.chm html\download\

del html\download\neldox.zip
zip -9 -r neldox.zip html\nel html\nelns html\tool html\index.html
move neldox.zip html\download\

pause