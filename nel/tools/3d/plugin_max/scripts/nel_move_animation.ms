-- This script move biped animation keeping the IK.

-- Include some NeL stuff

include "nel_utility.ms"

struct backupKey ( IkBlend, IkSpace, IkJoinedPivot )

rollout modify_animation_rollout "Properties"
(
	-- My parameters
	group "Modify animation"
	(
		Spinner	TransX	"Translation X" type:#float align:#left
		Spinner	TransY	"Translation Y" type:#float align:#left
		Spinner	TransZ	"Translation Z" type:#float align:#left
	)

	-- A function that move biped controller keys
	fn translateNode node2 controller trans =
	(
		for keyNum = 1 to controller.keys.count do
		(
			-- Get the key
			key = biped.getKey controller keyNum
			
			-- Animate
			with animate on
			(
				sliderTime = key.time
				at time key.time move node2 trans
			)
		)
	)

	-- Disactive / activate ik
	fn setIk node2 activate arrayIkBlend =
	(
		for keyNum = 1 to node2.transform.controller.keys.count do
		(
			-- Get the key
			key = biped.getKey node2.controller keyNum
			
			-- Activate ik ?
			if activate == true then
			(
				-- Restaure the ik blend
				sliderTime = key.time
				key.IkJoinedPivot = arrayIkBlend[keyNum].IkJoinedPivot
				key.IkSpace = arrayIkBlend[keyNum].IkSpace
				key.IkBlend = arrayIkBlend[keyNum].IkBlend
			)
			-- Disactivate ik
			else
			(
				-- Backup the ik blend
				append arrayIkBlend (backupKey key.IkBlend key.IkSpace key.IkJoinedPivot)
				
				-- Disactivate
				sliderTime = key.time
				key.IkBlend = 0
				key.IkSpace = 0
				key.IkJoinedPivot = false
			)
		)
	)

	-- My go function
	fn do_it =
	(
		-- Local count
		local countModification
		countModification = 0

		-- Motion panel
		setCommandPanelTaskMode mode:#motion
		
		-- For each biped
		local node
		for node in objects do
		(
			-- Is the root ?
			if (classof node.controller) == Vertical_Horizontal_Turn then
			(
				-- An array of key
				vector = [TransX.value,TransY.value,TransZ.value]
				
				-- Select the node or it doesn't work..
				
				-- Backup iks
				arrayIkLArm = #()
				arrayIkRArm = #()
				arrayIkLLeg = #()
				arrayIkRLeg = #()
				setIk (biped.getNode node #larm) false arrayIkLArm
				setIk (biped.getNode node #rarm) false arrayIkRArm
				setIk (biped.getNode node #lleg) false arrayIkLLeg
				setIk (biped.getNode node #rleg) false arrayIkRLeg

				-- Select the node				
				select node
				sliderTime = 1
				sliderTime = 0
				
				-- Horizontal controller
				node.controller.trackSelection = 1
				
				-- Transform it
				translateNode node node.controller.horizontal.controller [vector.x,vector.y,0]

				-- Select the node				
				select node
				
				-- Vertical controller
				node.controller.trackSelection = 2

				-- Transform it
				translateNode node node.controller.vertical.controller [0,0,vector.z]

				-- Restaure Iks
				sliderTime = 1
				sliderTime = 0
				select (biped.getNode node #larm)
				setIk (biped.getNode node #larm) true arrayIkLArm
				select (biped.getNode node #rarm)
				setIk (biped.getNode node #rarm) true arrayIkRArm
				select (biped.getNode node #lleg)
				setIk (biped.getNode node #lleg) true arrayIkLLeg
				select (biped.getNode node #rleg)
				setIk (biped.getNode node #rleg) true arrayIkRLeg

				-- One more biped
				countModification = countModification + 1
			)
		)

		return countModification
	)

	include "nel_batched_script.ms"	
)

-- Exec the script

gc ()

if modify_animation_floater != undefined do
(
	closerolloutfloater modify_animation_floater
)

modify_animation_floater = newRolloutFloater "NeL move animation" 550 874
addrollout modify_animation_rollout modify_animation_floater rolledUp:false
