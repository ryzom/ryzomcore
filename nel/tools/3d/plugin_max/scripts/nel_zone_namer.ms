-- This script sets proper centered zone positions and generates their names
-- Use after cutting the zone into 160m by 160m pieces

cell_size = 160.0
offset_x = 7680 / 2
offset_y = -(20480 + (5120 / 2))

alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" 

-- http://proofofprogress.blogspot.be/2011/03/solution-align-pivot-to-world-without.html
Function alignPivotToWorld &theObject = (
	--VLM = Visible Local Matrix.
	--The matrix/pivot you see when selecting object and "Local" axis is selected as viewable.
	VLM = theObject.Transform;
	IP_LocalRot = theObject.objectOffsetRot; --Rotation to be used later.
	IP_LOCAL = theObject.objectOffsetPos;  --Invisible Pivot Local coordinates
	--In relation to VLM matrix.
	IP_WORLD = IP_LOCAL * VLM; --World Coordinates of Invisible Pivot. [Local To World Transform]
	VLM_0 = matrix3 1; --Reset Visible Local matrix coordinates.

	NEW_IP_LOCAL = IP_WORLD * inverse(VLM_0); --[World To local Transform]

	theObject.Transform = VLM_0;
	theObject.objectOffsetPos = NEW_IP_LOCAL;

	--Now Handle Rotation:
	--Since rotation of visible local matrix has been zeroed out,
	--You must add that loss to the invisible pivot rotation.
	GeomWorldRot = VLM.RotationPart + IP_LocalRot;
	theObject.objectOffsetRot = GeomWorldRot;
)
	
-- Convert a coordinate in a name
-- name = coordToName #(x, y)
fn coordToName coord =
(
	up = floor(coord[1] / 26) + 1
	down = floor(coord[1] - ((up-1) * 26)) + 1
	return (((-coord[2] + 1) as integer) as string) + "_" + alphabet[up] + alphabet[down]
)

fn realCoordToName coord =
(
	return coordToName(#(((coord[1] + offset_x) / cell_size) + 0.5, ((coord[2] + offset_y) / cell_size) + 0.5))
)

fn roundedCoord coord =
(
	return #(ceil(coord[1] / cell_size) * cell_size - (cell_size / 2), ceil(coord[2] / cell_size) * cell_size - (cell_size / 2))
)

max select none
clearselection()

for node in geometry do
(
	if (classof node) == RklPatch or (classof node) == Editable_Patch then 
	(
		newcoords = roundedCoord(#(node.center.x, node.center.y))
		newname = realCoordToName(newcoords)
		node.name = newname
		alignPivotToWorld &node
		node.pivot.x = newcoords[1]
		node.pivot.y = newcoords[2]
		resetxform node
		maxOps.CollapseNode node off
	)
)
