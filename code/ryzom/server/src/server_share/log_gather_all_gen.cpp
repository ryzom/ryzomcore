
#include "stdpch.h"
#include "game_share/utils.h"



extern void forceLink_Player();

extern void forceLink_Character();

extern void forceLink_Item();

extern void forceLink_Command();

extern void forceLink_Outpost();

extern void forceLink_Ring();

extern void forceLink_Chat();


void forceLinkOfAllLogs()
{
	
	forceLink_Player();
	
	forceLink_Character();
	
	forceLink_Item();
	
	forceLink_Command();
	
	forceLink_Outpost();
	
	forceLink_Ring();
	
	forceLink_Chat();
	
};
	


