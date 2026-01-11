r:
cd \code_private\nel\tools\leveldesign
cvs -d :pserver:besson@server:/home/cvsroot update
cd \code_private\nel\tools\3d\ligo
cvs -d :pserver:besson@server:/home/cvsroot update
cd \code\ryzom\src
cvs -d :pserver:besson@server:/home/cvsroot update

xcopy "\\server\code\distrib\gamedev\lib\logic_rd.lib" "R:\distrib\gamedev\lib" /Y
xcopy "\\server\code\distrib\nel\lib\nlnet_rd.lib" "R:\distrib\nel\lib" /Y

pause
cd \code\nel\tools\leveldesign
