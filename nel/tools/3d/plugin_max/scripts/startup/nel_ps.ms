-- plugin to put a nel particle system in a scene
plugin Geometry nel_ps
name:"Particle Sys"
category:"Nel Tools"
classID:#(0x58ce2893, 0xbd2b74ef)
extends:Box
(	
	parameters pblock rollout:param
	(
		ps_file_name type:#String default:"*.ps"
		PSParam0 type:#float animatable:true ui:userParam1_s
		PSParam1 type:#float animatable:true ui:userParam2_s
		PSParam2 type:#float animatable:true ui:userParam3_s
		PSParam3 type:#float animatable:true ui:userParam4_s
		PSTrigger type:#boolean animatable:true ui:trigger_s
	)

	rollout param "Parameters"
	(
		edittext ps_file_name_et "file:"
		button browse "Browse..."
		spinner  userParam1_s "user param 1" range:[0,1,0] type:#float scale:0.01
		spinner  userParam2_s "user param 2" range:[0,1,0] type:#float scale:0.01
		spinner  userParam3_s "user param 3" range:[0,1,0] type:#float scale:0.01
		spinner  userParam4_s "user param 4" range:[0,1,0] type:#float scale:0.01
		checkbox trigger_s     "trigger system" type:#boolean scale:0.01

		on ps_file_name_et changed val do
		(
			ps_file_name = val
			setAppData this 1970 ps_file_name
		)		
		
		on param open do
		(
			ps_file_name_et.text = copy ps_file_name
		)
		
		on browse pressed do
		(
			fname = getOpenFileName caption: "particle system file" types:"particle system file(*.ps)|" ps_file_name:"*.ps"
			if not fname == undefined then
			(
			  -- ps_file_name = filenameFromPath fname 
			  ps_file_name = fname 
	  		  setAppData this 1970 ps_file_name
			  ps_file_name_et.text = ps_file_name
			)			
		)
	)

	
)