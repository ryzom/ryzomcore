// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdafx.h"

// ----------------------------------------------------------------------------------------------------------------------------------

typedef struct SYS_CPU
{
	unsigned long	ID;
	__int64			Start,Stop;
	unsigned long	NbCycles;
	struct SYS_CPU	*Previous;
	struct SYS_CPU	*Next;
} SYS_CPU;

// ----------------------------------------------------------------------------------------------------------------------------------

static __int64			first_cycle; 
static __int64			last_cycle; 
static unsigned long	NbCpus=0;
static struct SYS_CPU	*Sys_Cpu=NULL;

// ----------------------------------------------------------------------------------------------------------------------------------



/*
typedef struct ROUT
{
	unsigned char	Name[32];
	unsigned long	id;
	unsigned long	time;
} ROUT;
*/

ROUT Routines[20];

//#define PROFILE_BEGIN(name,i)	{strcpy(Routine[i].Name,name); Routine[i].id=Sys_StartClock();}
//#define PROFILE_END(i)			{Routine[i].time=Sys_StopClock(Routine[i].id);}


void FlushProfile(void)
{
	FILE*	file;
	int		i;

	file=fopen("\\profile.txt","wt");
	if (file)
	{
		for(i=0 ; i<20 ; i++)
		{
			if (Routines[i].id)
			{
				fprintf(file,"-------------------------------\n");
				fprintf(file,"Routine : %s",Routines[i].Name);
				fprintf(file," Cpu : %2.2f\n",Routines[i].time/(float)Routines[i].nbtimes);
			}
		}
	}
	fclose(file);
}

unsigned long Sys_StartClock(void)
{
	SYS_CPU			*cpu,*temp,*cpu2;
	unsigned long	i,taken;

	cpu=(SYS_CPU*)malloc(sizeof(SYS_CPU));
	if (!cpu)
	{
		return(0);
	}
	if (Sys_Cpu==NULL)
	{
		Sys_Cpu=cpu;
		cpu->Previous=NULL;
		cpu->Next=NULL;
	}
	else
	{
/*		temp=Sys_Cpu->Next;
		Sys_Cpu->Next=cpu;
		cpu->Previous=Sys_Cpu;
		cpu->Next=temp;
		if (temp) 
		{
			temp->Previous=cpu;
		}*/
		temp = Sys_Cpu;
		Sys_Cpu = cpu;
		Sys_Cpu->Previous = NULL;
		Sys_Cpu->Next = temp;
		temp->Previous = Sys_Cpu;
	}
	NbCpus++;
	for(i=1 ; i<NbCpus+1 ; i++)
	{
		taken=0;
		for(cpu2=Sys_Cpu ; cpu2 ; cpu2=cpu2->Next)
		{
			if (cpu2->ID==i) 
			{
				taken=1; 
				break; 
			}
		}
		if (!taken) 
		{ 
			cpu->ID=i; 
			break; 
		}
	}
	__asm
	{
		push eax 
		push edx 
		_emit 0x0F
		_emit 0x31
		mov  ebx, OFFSET first_cycle
		mov  [ebx+0],eax
		mov  [ebx+4],edx
		pop edx
		pop eax
	}
	cpu->Start=first_cycle;
	return(cpu->ID);
}

// ----------------------------------------------------------------------------------------------------------------------------------

float Sys_StopClock(unsigned long id)
{
	SYS_CPU			*cpu;
	__int64			diftime;

	for(cpu=Sys_Cpu ; cpu ; cpu=cpu->Next)
	{
		if (cpu->ID==id) 
		{
			break;
		}
	}
	__asm
	{
		push eax 
		push edx 
		_emit 0x0F
		_emit 0x31
		mov  ebx,OFFSET last_cycle
		mov  [ebx+0],eax
		mov  [ebx+4],edx
		pop edx
		pop eax
	}
	cpu->Stop=last_cycle;
	diftime=cpu->Stop-cpu->Start;
	if (diftime>0xFFFFFFFF) 
	{
		diftime=0xFFFFFFFF;
	}
	cpu->NbCycles=(unsigned long)diftime;
	free(cpu);
	return( diftime );
}

// ----------------------------------------------------------------------------------------------------------------------------------

