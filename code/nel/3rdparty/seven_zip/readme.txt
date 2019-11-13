

This is an extract of some files from the 7zip SDK.

At time of writing (2007-01-12) there is no dll or library project in the 7zip 
distribution (lzma443), so I build a custum project and copied in the 7zip 
files needed to do 7zip/lzma extraction.

To update this library, download the latest 7zip SDK and copy all file from 
	<lzma-sdk>/C/7zip/Archive/7z_C
for the 7zip archive file format support

and the 3 files needed from 
	<lzma-sdk>/C/7zip/Compress/LZMA_C 
	(lzmaDecode.h lzmaDecode.cpp lzmaTypes.h)
for the lzma decrompression lib.


NB : If callback support must be enabled, add "#define _LZMA_IN_CB" in 
	7zTypes.h


