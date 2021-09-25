-- Use to take the snapshots of a large manually created zone

from_x = 160
size_x = 7680-320
from_y = -25600+160
size_y = 5120-320

targetdir = "R:/graphics/landscape/ligo/asteroids/max"
snapshotdir = "R:/graphics/landscape/ligo/asteroids/zonebitmaps"
zonename = "anne"
resumeonly = true

cell_size = 160.0

alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" 

fn lowercase instring =                -- beginning of function definition 
(
	local upper, lower, outstring       -- declare variables as local 
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"  -- set variables to literals 
	lower="abcdefghijklmnopqrstuvwxyz" 
	outstring=copy instring
	for i=1 to outstring.count do 
	(  
		j=findString upper outstring[i] 
		if (j != undefined) do outstring[i]=lower[j] 
	) 
	return outstring     -- value of outstring will be returned as function result 
)

fn existFile fname = (getfiles fname).count != 0
	
-- Convert a coordinate in a name
-- name = coordToName #(x, y)
fn coordToName coord =
(
	up = floor(coord[1] / 26) + 1
	down = floor(coord[1] - ((up-1) * 26)) + 1
	return (((-coord[2] + 1) as integer) as string) + "_" + alphabet[up] + alphabet[down]
)

fn roundedCoord coord =
(
	return #(ceil(coord[1] / cell_size) * cell_size - (cell_size / 2), ceil(coord[2] / cell_size) * cell_size - (cell_size / 2))
)

fn coordId coord =
(
	coordr = (roundedCoord coord)
	return #(((coordr[1]) / cell_size) + 0.5, ((coordr[2]) / cell_size) + 0.5)
)

from_coord = (coordId #(from_x, from_y))
to_coord = (coordId #(from_x + size_x, from_y + size_y))

print from_coord
print to_coord

print (coordToName from_coord)

undo off
(
	for x=from_coord[1] to to_coord[1] do
	(
		for y=from_coord[2] to to_coord[2] do
		(
			ny=y+1
			sy=y-1
			ex=x+1
			wx=x-1
			zc = coordToName #(x, y)
			zn = coordToName #(x, ny)
			zne = coordToName #(ex, ny)
			ze = coordToName #(ex, y)
			zse = coordToName #(ex, sy)
			zs = coordToName #(x, sy)
			zsw = coordToName #(wx, sy)
			zw = coordToName #(wx, y)
			znw = coordToName #(wx, ny)
			maxc = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zc) + ".max"
			csnapfile = snapshotdir + "/" + zonename + "-" + (lowercase zc) + ".tga"
			if existFile maxc and (not resumeonly or not (existFile csnapfile)) then
			(
				print zc
				resetMAXFile #noprompt
				gc()
				mergeMAXFile maxc #(zc)
				ccenter = getnodebyname zc
				maxn = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zn) + ".max"
				if existFile maxn then
				(
					mergeMAXFile maxn #(zn)
					cnode = getnodebyname zn
					cnode.position.y = cnode.position.y + 160
					NeLAttachPatchMesh cnode ccenter
				)
				maxne = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zne) + ".max"
				if existFile maxne then
				(
					mergeMAXFile maxne #(zne)
					cnode = getnodebyname zne
					cnode.position.x = cnode.position.x + 160
					cnode.position.y = cnode.position.y + 160
					NeLAttachPatchMesh cnode ccenter
				)
				maxe = targetdir + "/zonematerial-" + zonename + "-" + (lowercase ze) + ".max"
				if existFile maxe then
				(
					mergeMAXFile maxe #(ze)
					cnode = getnodebyname ze
					cnode.position.x = cnode.position.x + 160
					NeLAttachPatchMesh cnode ccenter
				)
				maxse = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zse) + ".max"
				if existFile maxse then
				(
					mergeMAXFile maxse #(zse)
					cnode = getnodebyname zse
					cnode.position.x = cnode.position.x + 160
					cnode.position.y = cnode.position.y - 160
					NeLAttachPatchMesh cnode ccenter
				)
				maxs = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zs) + ".max"
				if existFile maxs then
				(
					mergeMAXFile maxs #(zs)
					cnode = getnodebyname zs
					cnode.position.y = cnode.position.y - 160
					NeLAttachPatchMesh cnode ccenter
				)
				maxsw = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zsw) + ".max"
				if existFile maxsw then
				(
					mergeMAXFile maxsw #(zsw)
					cnode = getnodebyname zsw
					cnode.position.x = cnode.position.x - 160
					cnode.position.y = cnode.position.y - 160
					--NeLAttachPatchMesh cnode ccenter
				)
				maxw = targetdir + "/zonematerial-" + zonename + "-" + (lowercase zw) + ".max"
				if existFile maxw then
				(
					mergeMAXFile maxw #(zw)
					cnode = getnodebyname zw
					cnode.position.x = cnode.position.x - 160
					NeLAttachPatchMesh cnode ccenter
				)
				maxnw = targetdir + "/zonematerial-" + zonename + "-" + (lowercase znw) + ".max"
				if existFile maxnw then
				(
					mergeMAXFile maxnw #(znw)
					cnode = getnodebyname znw
					cnode.position.x = cnode.position.x - 160
					cnode.position.y = cnode.position.y + 160
					NeLAttachPatchMesh cnode ccenter
				)
				NeLWeldPatchMesh ccenter 1.0
				NeLLigoMakeSnapShot ccenter csnapfile 0 1 0 1 false
			)
		)
	)
)
