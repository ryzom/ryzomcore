
r2.BasicBricks = {}

r2.BasicBricks["RtScenario"] = { acts = {} }

r2.BasicBricks["RtAct"] = 
{ 
	id="", npc_grps={}, fauna_grps={}, ai_states={}, npcs={}
}

r2.BasicBricks["RtNpcGrp"] = 
{ 
	id="", Name="", children={}, 
	autospawn=1, bot_chat_parameters="", bot_equipment="",
	bot_sheet_client="", bot_vertical_pos="auto",
	count = 0, grp_keywords =""
}

r2.BasicBricks["RtNpc"]= 
{
	id="",	Name="",
	chat_parameters="", equipment="", is_stuck=0,
	keywords="", sheet_client="",	bot_vertical_pos="auto",
	angle=0, pt = {x=0,y=0,z=0}
}

r2.BasicBricks["RtAiState"]=
{
	id="", Name="", children={},
	ai_activity="no_change", ai_movement="stand_on_start_point",
	ai_profile_params = 0, keywords="", vertical_pos="auto",
	pt = {}
}

