-- This script rename texture files in material

-- Include some NeL stuff

NEL3D_APPDATA_IGNAME = 1423062564			-- string : name of the Instance Group

fn getIg node =
(
	return (getappdata node NEL3D_APPDATA_IGNAME)
)

rollout select_rollout "Properties"
(
	Label			IGNameLabel				"Name of IG:"					align:#left
	ListBox			IGNameList				height:29
	Button			RefreshBt				"Refresh list"					align:#left

	on IGNameList doubleClicked toto do
	(
		max select none
		for node in objects do
		(
			if ((getIg(node)) == IGNameList.items[toto]) then
			(
				selectmore node
			)
		)
	)

	fn refresh =
	(
		array=#()
		for node in objects do
		(
			igname = getIg node
			if (igname != undefined) then
			(
				found = false
				for i=1 to array.count do
				(
					if (array[i] == igname) then
					(
						found = true
						exit
					)
				)
				if (found == false) then
				(
					append array igname
				)
			)
		)
		IGNameList.items = array
	)

	on RefreshBt pressed do
	(
		refresh()
	)

	on select_rollout open do
	(
		refresh()
	)
)

--

gc ()

if select_floater != undefined do
(
	closerolloutfloater select_floater
)

select_floater = newRolloutFloater "NeL select IG" 300 500
addrollout select_rollout select_floater rolledUp:false
