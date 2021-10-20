rollout nel_matrix_properties "NeL Matrix Properties"
( 

spinner _LandScapeWidth "Zones X " range:[1,128,8] type:#integer align:#center width:140
spinner _LandScapeLength "Zones Y " range:[1,128,8] type:#integer align:#center width:140

)

rollout nel_displace_properties "Apply Height Map"
( 

spinner _DisplaceStrength "Strength  " range:[1,512,128] type:#integer align:#center width:140
button smap "Select Height Map" align:#center width:178

on smap pressed do
( 
  global _DisplaceMapFile = selectbitmap()
)

)

rollout nel_matrix_create "NeL Matrix"
( 

button go "Create Matrix" align:#center width:178
label lab1 "©Neverborn Entertainment 2004"

on go pressed do
( 

units.USType = #Dec_In
units.SystemType = #Inches
units.DisplayType = #US

_PatchWidth = 160
_PatchLength = 160
_SegmentsX = 5
_SegmentsY = 7
_CurrentX = 400
_CurrentY = -560
_CurrentZ = 0
_DefaultX = 400
_DefaultY = -560
_DefaultZ = 0
_NameX = 3
_NameY = 65
_NameZ = 67
_Color = 1

for i = 1 to nel_matrix_properties._LandScapeLength.value do
(
for i = 1 to nel_matrix_properties._LandScapeWidth.value do
(
  _Patch = Quadpatch pos:[_CurrentX, _CurrentY, _CurrentZ]
  _Patch.name = _NameX as string + "_" + bit.intAsChar _NameY + bit.intAsChar _NameZ
  _Patch.lengthsegs = _SegmentsY
  _Patch.widthsegs = _SegmentsX
  _Patch.length = _PatchWidth
  _Patch.width = _PatchLength

  if _Color == 1 then (_Patch.wireColor = [200,200,200]; _Color = 2) else (_Patch.wireColor = [100,100,100]; _Color = 1)

  convertTo _Patch Editable_Patch
 
  _NameX += 1
  _CurrentY -= _PatchLength
) 
  _Odd = mod nel_matrix_properties._LandScapeWidth.value 2
  if _Odd == 0.0 do (if _Color == 2 then _Color = 1 else _Color = 2)
  _CurrentY = _DefaultY
  _CurrentX += _PatchWidth
  _NameX = 3
  _NameZ += 1
  if _NameZ == 91 do (_NameY = _NameY + 1; _NameZ = 65)
)

max modify mode
max select all
dis = displace()
modpanel.addModToSelection dis
_DisplaceBitMap = _DisplaceMapFile
dis.bitmap = _DisplaceBitMap
dis.strength = nel_displace_properties._DisplaceStrength.value
convertTo $ Editable_Patch
clearSelection() 

)

)

NeL = newRolloutFloater "NeL Matrix v2.01" 206 236
addRollout nel_matrix_properties NeL 
addRollout nel_displace_properties NeL
addRollout nel_matrix_create NeL

-- ©Neverborn Entertainment 2004
-- Debug
-- debug@neverborn.net
-- www.neverborn.net