-- Roll out

source_node = undefined
dest_node = undefined
source_root_name = undefined
dest_root_name = undefined

-- This function look for a child with its name
fn getChildByName node name =
(
	local i

	-- Sourcename
	source_name = substring name (1+source_root_name.count) (name.count-source_root_name.count)

	-- For each children
	for i = 1 to node.children.count do
	(
		-- This child ?
		dest_name = substring node.children[i].name (1+dest_root_name.count) (node.children[i].name.count-dest_root_name.count)
		if dest_name == source_name then
		(
			return node.children[i]
		)
	)
	return undefined
)

-- This function copy the bones
fn copy_node my_source_node my_dest_node = 
(
	local i

	-- Scale using offset value
	scale my_dest_node (my_source_node.objectOffsetScale / my_dest_node.objectOffsetScale)

	-- For each children
	for i = 1 to my_source_node.children.count do
	(
		local child

		-- Get the node
		child = getChildByName my_dest_node my_source_node.children[i].name
		if (child != undefined) then
		(
			copy_node my_source_node.children[i] child
		)
		else
	)
)

rollout copy_biped_figure_mode "Properties"
(
	Pickbutton SourceNode "Pick source biped" align:#left width:250
	Pickbutton DestNode "Pick destination biped" align:#left width:250
	Button Go "Go" align:#left width:250

	on SourceNode picked obj do 
	(
		source_node = obj.controller.rootNode
		SourceNode.text = source_node.name
	)

	on DestNode picked obj do 
	(
		dest_node = obj.controller.rootNode
		DestNode.text = dest_node.name
	)

	on Go pressed do
	(
		local state_source_fm
		local state_dest_fm

		-- Root names
		source_root_name = source_node.name
		dest_root_name = dest_node.name

		-- Backup figure mode state
		state_source_fm = source_node.controller.figureMode
		state_dest_fm = dest_node.controller.figureMode

		-- Set the biped in figure mode
		source_node.controller.figureMode = true
		dest_node.controller.figureMode = true
		
		-- Copy height
		dest_node.controller.height = source_node.controller.height

		-- Copy root node
		copy_node source_node dest_node

		-- Restaure figure mode state
		source_node.controller.figureMode = state_source_fm
		dest_node.controller.figureMode = state_dest_fm
	)
)

-- Go

gc ()

if copy_biped_figure_mode_floater != undefined do
(
	closerolloutfloater copy_biped_figure_mode_floater
)

copy_biped_figure_mode_floater = newRolloutFloater "NeL Copy Biped Figure Mode" 300 145
addrollout copy_biped_figure_mode copy_biped_figure_mode_floater rolledUp:false
