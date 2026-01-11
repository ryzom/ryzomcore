--*************************************************************

-- compute looped index
fn exVertIndex index numPts = 
(
	if (index == 0) then numPts
	else if (index == numPts + 1) then 1
	else index
)

--*************************************************************

-- compute the tangent of the given vertex
fn getExtrudeTgt mesh index numPoints =
(
	
	
	i1 = getVert mesh (exVertIndex (index - 1) numPoints)
	--i2 = exVertIndex index numPoints
	i3 = getVert mesh (exVertIndex (index + 1) numPoints)
	dir = [0, 0, 0]
	dir.x = i3.y - i1.y
	dir.y = - (i3.x - i1.x)
	dir.z = i3.z - i1.z
	dir = normalize dir
	dir
)


--*****************************************************
-- compute the perimeter of the mesh to extrude
fn computeWaterBorderPerimeter mesh =
(
	lenght = 0
	for i = 1 to (mesh.numverts - 1) do
	(
		lenght = lenght + (distance (getVert mesh i) (getVert mesh i + 1))
	)
	lenght = lenght + (distance (getVert mesh (mesh.numverts - 1)) (getVert mesh 1))
	return lenght
)



--*************************************************************

fn setWEFace mesh indexFace i1 i2 i3 =
(
	setFace mesh indexFace [i2, i1, i3]
)

--*************************************************************

plugin simpleMod waterStretch
name: "WaterStretch"
classID:#(0x1f7535ea, 0x36423d01)
version: 1
(	
	local numSlices
	parameters params rollout:main
	(	   
		tgts	   type:#point3tab tabSizeVariable: true
		vertsNb    type:#integer
		amount1    type:#float ui:cAmount1 default:1			
		amount2    type:#float ui:cAmount2 default:1			
		amount3    type:#float ui:cAmount3 default:1			
		amount4    type:#float ui:cAmount4 default:1					
	)
	rollout main "Parameters"
	(
		spinner cAmount1 "amount 1" type:#float range:[-1000, 1000, 0.1]
		spinner cAmount2 "amount 2" type:#float range:[-1000, 1000, 0.1]
		spinner cAmount3 "amount 3" type:#float range:[-1000, 1000, 0.1]
		spinner cAmount4 "amount 4" type:#float range:[-1000, 1000, 0.1]
		
		on main open do
		(
			print "rollout opened"
			if (numSlice < 2) then cAmount2.enabled = false
			if (numSlice < 3) then cAmount3.enabled = false
			if (numSlice < 4) then cAmount4.enabled = false		
		)
	)
	on map i p do
	(				
	    if (i == 0) then return p
		if (vertsNb != 0) then
		(
			if (i > vertsNb) then
			(	
				numSlice = (floor ((i - 1) / vertsNb)) as integer				
				v0 =  p 
				v0n = tgts[(i - (numSlice * vertsNb))]			
				
				if (numSlice == 1) then amount = amount1
				if (numSlice == 2) then amount = amount1 + amount2
				if (numSlice == 3) then amount = amount1 + amount2 + amount3
				if (numSlice == 4) then amount = amount1 + amount2 + amount3 + amount4

				return (v0 + (amount * v0n))
			)
			else
			(
				return p
			)
		)
	)
		
)

fn buildSlice sel startIndex numVerts slice nbSlice perimeter =
(
	-- setup vertices
	dist = 0
	for i = 1 to  numVerts do
	(				
		setVert sel (i  +  startIndex + numVerts) (getVert sel i)
		setTVert sel (i  + startIndex) [dist / perimeter, (slice - 1) / (nbSlice as float), 0]
		setTVert sel (i  + startIndex + numVerts) [dist / perimeter, slice / (nbSlice as float), 0]
		if (i != numVerts) then
		(
			dist = dist + (distance (getVert sel i) (getVert sel (i + 1)))
		)				
	)
		
	-- setup faces index
	for i = 0 to (numVerts - 2) do
	(		
		setWEFace sel (2 * startIndex + 2 * i + 1) (startIndex + i + 1) 		   (startIndex + i + numVerts+ 2) (startIndex + 1 + i + numVerts)
		setWEFace sel (2 * startIndex + 2 * i + 2) (startIndex + i + 1) 		   (startIndex + i + 2) 		  (startIndex + i + numVerts + 2)
		setTVFace sel (2 * startIndex + 2 * i + 1) (startIndex + i + numVerts + 2) (startIndex + i + 1) 		  (startIndex + 1 + i + numVerts)
		setTVFace sel (2 * startIndex + 2 * i + 2) (startIndex + i + 2) 		   (startIndex + i + 1) 		  (startIndex + i + numVerts + 2)
		setVCFace sel (2 * startIndex + 2 * i + 1) [slice + 1, slice, slice + 1]
		setVCFace sel (2 * startIndex + 2 * i + 2) [slice, slice, slice + 1]
	)
		
	
	--setup last face
	i = numVerts - 1
	setWEFace sel (2 * startIndex + 2 * i + 1) (startIndex + i + 1) 				 (startIndex + numVerts + 1) (startIndex + 1 + i + numVerts)
	setWEFace sel (2 * startIndex + 2 * i + 2) (startIndex +  i + 1) 				 (startIndex + 1) 			 (startIndex + numVerts+ 1)
	setTVFace sel (2 * startIndex + 2 * i + 1) (startIndex + numVerts + 1)    		 (startIndex + i + 1) 		 (startIndex + 1 + i + numVerts)
	setTVFace sel (2 * startIndex + 2 * i + 2) (startIndex + 1) 					 (startIndex + i + 1) 		 (startIndex + numVerts + 1)
	setVCFace sel (2 * startIndex + 2 * i + 1) [slice + 1, slice, slice + 1]
	setVCFace sel (2 * startIndex + 2 * i + 2) [slice, slice, slice + 1]
)

utility ExtrudeWater "Extrude water"
(
	button extrude "extrude water"
	spinner cNumSlice "numSlices" type:#integer range:[1, 4, 1]
	
	
	on extrude pressed do
	(
		numSlices = cNumSlice.value
	    for currSel = 1 to selection.count do
		(		
			sel = selection[currSel]
			-- make a copy of the selection
			myCopy = copy sel
			myCopy.name = myCopy.name + "_backup"
			ConvertToMesh sel
			--collapseStack sel
			perimeter = computeWaterBorderPerimeter sel		
			initialNumVerts = sel.numverts + 1
			-- create additionnal vertices		
			setNumVerts sel (initialNumVerts * (1 + numSlices)) true
			-- duplicate last vert (to avoid bad mapping)
			setVert sel initialNumVerts (getVert sel 1)
			setNumFaces sel (2 * initialNumVerts * numSlices) true
				
			-- vertex colors
			setNumCPVVerts sel (numSlices + 1)
			buildVCFaces sel
			for k = 1 to (numSlices + 1) do
			(
				i = 255 - (255 * (k - 1) / numSlices)
				setVertColor sel k (color i i i i)
			)
		
			-- vertex texture coordinates
			setNumTVerts sel (initialNumVerts * (1 + numSlices))
			buildTVFaces sel

			-- build slices
			for currSlice = 1 to numSlices do
			(
				buildSlice sel ((currSlice - 1) * initialNumVerts)  initialNumVerts currSlice numSlices perimeter
			)
					
			modif = waterStretch()
						
			
			-- compute tangents
			for i = 1 to initialNumVerts do
			(				
				tg = getExtrudeTgt sel i initialNumVerts
				modif.tgts[i] = tg
			)						
		
			modif.vertsNb 	= initialNumVerts
			modif.numSlices = numSlices
			addModifier sel (vertexPaint())						
			addModifier sel modif						
			update sel							
		)
	)
)

-- select vertices from a water extruded object
fn selectVertices obj slice =
(
	local vertsNb = obj.numVerts 
	tab = #()
	for i = 1 to vertsNb do
	(
		tab[i] = slice * vertsNb + i
	)	
	setVertSelection obj tab
)

-- a macro to select one slice from a water extruded shape
macroScript wsChooseSlice1 category:"NEVRAX Tools" tooltip:"Water / choose slice 1"
(
	if ($ != undefined) then
	(
		selectVertices $ 1
		update $
	)	
)

macroScript wsChooseSlice2 category:"NEVRAX Tools" tooltip:"Water / choose slice 2"
(
	if ($ != undefined) then
	(
		selectVertices $ 2
		update $
	)	
)

macroScript wsChooseSlice3 category:"NEVRAX Tools" tooltip:"Water / choose slice 3"
(
	if ($ != undefined) then
	(
		selectVertices $ 3
		update $
	)	
)

macroScript wsChooseSlice4 category:"NEVRAX Tools" tooltip:"Water / choose slice 4"
(
	if ($ != undefined) then
	(
		selectVertices $ 4
		update $
	)	
)



