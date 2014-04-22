// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "std_afx.h"

#include "nel_export.h"

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::ExtractFileName(char* Path, char* Name)
{
	long	i,j;
	char	temp[MAX_PATH];

	for(j=0,i=strlen(Path)-1 ; i>=0 && Path[i]!='\\' && Path[i]!='//' ; i--,j++)
		temp[j]=Path[i];
	temp[j]=0;

	for(i=strlen(temp)-1,j=0 ; i>=0 ; i--,j++)
		Name[j]=temp[i];
	Name[j]=0;

	return(1);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::ExtractPath(char* FullPath, char* Path)
{
	long	i;

	for(i=strlen(FullPath)-1 ; i>=0 && FullPath[i]!='\\' && FullPath[i]!='//' ; i--);
	strncpy(Path,FullPath,i+1);
	Path[i+1]=0;

	return(1);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::SelectFileForLoad(HWND Parent, char* Title, const char* Mask, char* FileName)
{
	OPENFILENAME	ofn;
	char			r;
	char			curdir[MAX_PATH];
	char			fname[MAX_PATH];

	fname[0]=0;
	if (!FileName[0])
	{
		GetCurrentDirectory(MAX_PATH,curdir);
	}
	else
	{
		ExtractPath(FileName,curdir);
		if (!curdir[0])
		{
			GetCurrentDirectory(MAX_PATH,curdir);
		}
		ExtractFileName(FileName,fname);
	}

	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	 		=	sizeof ( OPENFILENAME );
	ofn.hwndOwner			=	Parent;
	ofn.hInstance			=	GetModuleHandle(NULL);
	ofn.lpstrFilter	  		=	Mask;
	ofn.lpstrCustomFilter	=	NULL;
	ofn.nFilterIndex	  	=	0;
	ofn.lpstrFile		    =	fname;
	ofn.nMaxFile		    =	500;
	ofn.lpstrTitle	    	=	Title;
	ofn.Flags			    =	OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt		  	=	"*";
	ofn.lpstrInitialDir		=	curdir;
	r=GetOpenFileName ( &ofn );
	strcpy(FileName,fname);
	return(r);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::SelectFileForSave(HWND Parent, char* Title, const char* Mask, char* FileName)
{
	OPENFILENAME	ofn;
	char			r;
	char			curdir[MAX_PATH];
	char			fname[MAX_PATH];

	fname[0]=0;
	if (!FileName[0])
	{
		GetCurrentDirectory(MAX_PATH,curdir);
	}
	else
	{
		ExtractPath(FileName,curdir);
		if (!curdir[0])
		{
			GetCurrentDirectory(MAX_PATH,curdir);
		}
		ExtractFileName(FileName,fname);
	}

	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	 		=	sizeof ( OPENFILENAME );
	ofn.hwndOwner			=	Parent;
	ofn.hInstance			=	GetModuleHandle(NULL);;
	ofn.lpstrFilter	  		=	Mask;
	ofn.lpstrCustomFilter	=	NULL;
	ofn.nFilterIndex	  	=	0;
	ofn.lpstrFile		    =	fname;
	ofn.nMaxFile		    =	500;
	ofn.lpstrTitle	    	=	Title;
	ofn.Flags			    =	OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt		  	=	"*";
	ofn.lpstrInitialDir		=	curdir;
	r=GetSaveFileName ( &ofn );
	strcpy(FileName,fname);
	return(r);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::SelectDir(HWND Parent, char* Title, char* Path)
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;

	bi.hwndOwner=Parent;
	bi.pidlRoot=NULL;
	bi.pszDisplayName=Path;
	bi.lpszTitle=Title;
	bi.ulFlags=0;
	bi.lpfn=0;
	bi.lParam=0;
	bi.iImage=0;
	pidl=SHBrowseForFolder(&bi);
	if (!SHGetPathFromIDList(pidl,str) ) 
	{
		return(0);
	}
 	strcpy(Path,str);
	return(1);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::FileExists(const char* FileName)
{
	FILE	*file;
	if ( !strcmp(FileName,"") ) return(0);
	file=fopen(FileName,"rb");
	if (!file) return(0);
	fclose(file);
	return(1);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::GetFileSize(char* FileName)
{
	FILE			*file;
	unsigned long	fsize;

	file=fopen(FileName,"rb");
	if (!file) return(0);
	fseek(file,0,SEEK_END);
	fsize=ftell(file);
	fclose(file);
	return(fsize);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::ProcessDir(char* Dir, const char* Mask, unsigned long flag, ULONG Fnct(char* FileName) )
{
	char			ToFound[MAX_PATH];
	char			FullDir[MAX_PATH];
	char			str[MAX_PATH];
	HANDLE			h;
	WIN32_FIND_DATA	fi;
	BOOL			r;

	GetFullPathName(Dir,MAX_PATH,FullDir,NULL);
	// --- Directory
	if (flag)
	{
		strcpy(ToFound,Dir);
		if ( ToFound[strlen(ToFound)-1]!='\\')
			strcat(ToFound,"\\");
		strcat(ToFound,"*.*");
		h=FindFirstFile(ToFound,&fi);
		if (h!=INVALID_HANDLE_VALUE)
		{
			r=1;
			while(r)
			{
				if ( strcmp(fi.cFileName,".") && strcmp(fi.cFileName,"..") )
				{
					if (	
						fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
						!(fi.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
						)
					{
						strcpy(str,Dir);
						if ( str[strlen(str)-1]!='\\' ) strcat(str,"\\");
						strcat(str,fi.cFileName);
						if ( !ProcessDir(str,Mask,flag,Fnct) ) return(0);
					}
				}					
				r=FindNextFile(h,&fi);
			}
			FindClose(h);
		}
	}
	// --- Files
	strcpy(ToFound,Dir);
	if ( ToFound[strlen(ToFound)-1]!='\\') strcat(ToFound,"\\");
	strcat(ToFound,Mask);
	h=FindFirstFile(ToFound,&fi);
	if (h!=INVALID_HANDLE_VALUE)
	{
		r=1;
		while(r)
		{
			if ( strcmp(fi.cFileName,".") && strcmp(fi.cFileName,"..") )
			{
				if ( !(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					strcpy(str,FullDir);
					if ( str[strlen(str)-1]!='\\' ) strcat(str,"\\");
					strcat(str,fi.cFileName);
					::strupr(str);
					if ( !Fnct(str) ) return(0);
				}
			}					
			r=FindNextFile(h,&fi);
		}
		FindClose(h);
	}
	return(1);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::CleanFileName(char* FileName)
{
	char	str[MAX_PATH];
	unsigned long	i,j,found;
	
	j=0;
	for(i=0 ; i<strlen(FileName) ; i++)
	{
		found=0;
		if (FileName[i]=='\\' && i!=0 && FileName[i-1]!='\\') { str[j]='\\'; j++; found=1; }
		if (FileName[i]=='/' && i!=0 && FileName[i-1]!='/') { str[j]='/'; j++; found=1; }
		if (!found) { str[j]=FileName[i]; j++; }
	}
	return(1);
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::CreateBAKFile(char* FileName)
{
	FILE			*fin,*fout;
	unsigned long	fsize;
	char	*block;
	char	str[MAX_PATH];

	if ( !FileExists(FileName) )
	{
		return(1);
	}
	fin=fopen(FileName,"rb");
	if (!fin)
	{
		//SetError("CreateBAKFile, unable to open %s",FileName);
		return(0);
	}
	fsize=GetFileSize(FileName);
	if (!fsize)
	{
		//SetError("CreateBAKFile, unable to get file size for %s",FileName);
		return(0);
	}
	block=(char*)calloc(1,fsize);
	if (!block)
	{
		//SetError("CreateBAKFile, not enough memory");
		return(0);	
	}
	fread(block,1,fsize,fin);
	fclose(fin);

	strcpy(str,FileName);
	str[strlen(str)-1]='K';
	str[strlen(str)-2]='A';
	str[strlen(str)-3]='B';

	fout=fopen(str,"w+b");
	if (!fout)
	{
		//SetError("CreateBAKFile, unable to open %s",str);
		return(0);
	}
	fwrite(block,1,fsize,fout);
	fclose(fout);
	return(1);
}

