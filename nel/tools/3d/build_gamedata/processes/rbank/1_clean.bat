@echo off

del retrievers\*.* /Q
del tesselation\*.* /Q
del smooth\preproc\*.* /Q
del raw\preproc\*.* /Q
del bbox\*.* /Q
del tag\*.* /Q
del output\*.* /Q
del cmb\*.* /Q

bash sh/clean.sh