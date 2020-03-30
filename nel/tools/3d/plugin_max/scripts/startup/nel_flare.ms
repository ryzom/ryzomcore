-- plugin to edit a nel flare
plugin Geometry nel_flare
name:"Nel flare"
category:"Nel Tools"
classID:#(0x4e913532, 0x3c2f2307)
extends:Sphere
(	
	parameters pblock rollout:param
	(
		texFileName0 	 type:#String default:".tga"
		texFileName1 	 type:#String default:".tga"
		texFileName2 	 type:#String default:".tga"
		texFileName3 	 type:#String default:".tga"
		texFileName4 	 type:#String default:".tga"
		texFileName5 	 type:#String default:".tga"
		texFileName6 	 type:#String default:".tga"
		texFileName7 	 type:#String default:".tga"
		texFileName8 	 type:#String default:".tga"
		texFileName9 	 type:#String default:".tga"
		
		flareUsed0		 type:#Boolean default: true   ui:check0
		flareUsed1		 type:#Boolean default: false  ui:check1
		flareUsed2		 type:#Boolean default: false  ui:check2
		flareUsed3		 type:#Boolean default: false  ui:check3
		flareUsed4		 type:#Boolean default: false  ui:check4
		flareUsed5		 type:#Boolean default: false  ui:check5
		flareUsed6		 type:#Boolean default: false  ui:check6
		flareUsed7		 type:#Boolean default: false  ui:check7
		flareUsed8		 type:#Boolean default: false  ui:check7
		flareUsed9		 type:#Boolean default: false  ui:check7
		
		size0			 type:#Float   default: 1      ui:UISize0
		size1			 type:#Float   default: 1      ui:UISize1
		size2			 type:#Float   default: 1      ui:UISize2
		size3			 type:#Float   default: 1      ui:UISize3
		size4			 type:#Float   default: 1      ui:UISize4
		size5			 type:#Float   default: 1      ui:UISize5
		size6			 type:#Float   default: 1      ui:UISize6
		size7			 type:#Float   default: 1      ui:UISize7
		size8			 type:#Float   default: 1      ui:UISize8
		size9			 type:#Float   default: 1      ui:UISize9

		pos0			 type:#Float   default: 0        ui:UIPos0
		pos1			 type:#Float   default: 0.1      ui:UIPos1
		pos2			 type:#Float   default: 0.2      ui:UIPos2
		pos3			 type:#Float   default: 0.3      ui:UIPos3
		pos4			 type:#Float   default: 0.4      ui:UIPos4
		pos5			 type:#Float   default: 0.5      ui:UIPos5
		pos6			 type:#Float   default: 0.6      ui:UIPos6
		pos7			 type:#Float   default: 0.7      ui:UIPos7
		pos8			 type:#Float   default: 0.8      ui:UIPos8
		pos9			 type:#Float   default: 0.9      ui:UIPos9


		
		ColorParam 				 type:#color 	animatable:false default:  white ui:UIColorParam
		PersistenceParam 		 type:#float 	animatable:false default:  1     ui:UIPersistenceParam
		Spacing          		 type:#float 	animatable:false default:  1     ui:UISpacing
		AttenuationRange  		 type:#float 	animatable:false default:  1     ui:UIAttenuationRange
		Attenuable       		 type:#boolean  animatable:false default:  false ui:UIAttenuable
		FirstFlareKeepSize 		 type:#Boolean  animatable:false default:  false ui:UIFirstFlareKeepSize 
		HasDazzle				 type:#boolean  animatable:false default:  false ui:UIHasDazzle
		DazzleColor				 type:#color	animatable:false default:  white ui:UIDazzleColor
		DazzleAttenuationRange	 type:#float	animatable:false default:  0.5   ui:UIDazzleAttenuationRange
		MaxViewDist				 type:#float	animatable:false default:  1000  ui:UIMaxViewDist
		MaxViewDistRatio		 type:#float	animatable:false default:  0.9   ui:UIMaxViewDistRatio

		occlusionTestMesh 				 type:#String default:""
		occlusionTestMeshInheritScaleRot type:#boolean  animatable:false default:false ui:UIOcclusionTestMeshInheritScaleRot
		
		sizeDisappear			 type:#Float   default: 0        ui:UISizeDisappear
		angleDisappear			 type:#Float   default: 0        ui:UIAngleDisappear
		scaleWhenDisappear 		 type:#boolean  animatable:false default:  false ui:UIScaleWhenDisappear

		lookAtMode 				type:#boolean  animatable:false default:true ui:UILookAtMode
	)

	rollout param "Parameters"
	(	
		checkbox	UILookAtMode		"LookAt mode"			   type:#boolean	
		colorpicker UIColorParam 		"color" 	               color:white 
		spinner 	UIPersistenceParam  "persistence    :"	       range:[0,20,1]  type:#float scale:0.01 align:#left
		spinner 	UISpacing           "flare spacing  :" 	       range:[0,20,1]  type:#float scale:0.01 align:#left
		checkbox    UIAttenuable        "radial attenuation" 	   type:#boolean
		spinner     UIAttenuationRange  "atten. range"        	   range:[0,20,1]  type:#float align:#left
		checkbox    UIFirstFlareKeepSize "keep 1st size  :"	       type:#boolean
		checkbox	UIHasDazzle			 "Enable dazzle  :"		   type:#boolean
		spinner     UIDazzleAttenuationRange  "dazzle atten.:" 	   range:[0,20,1]  type:#float align:#left
		colorpicker UIDazzleColor 		 "dazzle color" 	           color:white 
		edittext    UIOcclusionTestMesh  "TestMesh:"
		button      UIBrowseOcclusionMesh  "Browse occlusion mesh ..."
		checkbox	UIOcclusionTestMeshInheritScaleRot	"Test mesh inherit Scale/Rot" type:#boolean
		group "Distance"
		(
			spinner     UIMaxViewDist		"Max view dist." 	   range:[0,1000,1]  type:#float align:#left
			spinner     UIMaxViewDistRatio  "Atten. start" range:[0,0.9,0.05]  type:#float align:#left
		)

		group "Disappear behaviour"
		(
			checkbox  UIScaleWhenDisappear "Scale when disappear"	       type:#boolean
			spinner   UISizeDisappear      "size :"  range:[0,500,1] type:#float scale:0.01 align:#left
			spinner   UIAngleDisappear	   "angle  :"  range:[0,3600,0]   type:#float scale:0.01 align:#left
		)
		
		group "Flare 0"
		(
			edittext  tex_file_name_et0 "texture:"
			button    browse0           "Browse..."
			checkbox  check0            "enabled"
			spinner   UISize0           "size :"  range:[0,500,1] type:#float scale:0.01 align:#left
			spinner   UIPos0			"pos  :"  range:[0,1,0]   type:#float scale:0.01 align:#left
		)
		group "Flare 1"
		(
			edittext  tex_file_name_et1 "texture:"
			button    browse1           "Browse..."
			checkbox  check1            "enabled"
			spinner   UISize1           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos1			"pos  :"  range:[0,1,0.1]   type:#float scale:0.01 align:#left
		)
		group "Flare 2"
		(
			edittext  tex_file_name_et2 "texture:"
			button    browse2           "Browse..."
			checkbox  check2            "enabled"
			spinner   UISize2           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos2			"pos  :"  range:[0,1,0.2]   type:#float scale:0.01 align:#left
		)
		group "Flare 3"
		(
			edittext  tex_file_name_et3 "texture:"
			button    browse3           "Browse..."
			checkbox  check3            "enabled"
			spinner   UISize3           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos3			"pos  :"  range:[0,1,0.3]   type:#float scale:0.01 align:#left
		)
		group "Flare 4"
		(
			edittext  tex_file_name_et4 "texture:"
			button    browse4           "Browse..."
			checkbox  check4            "enabled"
			spinner   UISize4           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos4			"pos  :"  range:[0,1,0.4]   type:#float scale:0.01 align:#left
		)
		group "Flare 5"
		(
			edittext  tex_file_name_et5 "texture:"
			button    browse5           "Browse..."
			checkbox  check5            "enabled"
			spinner   UISize5           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos5			"pos  :"  range:[0,1,0.5]   type:#float scale:0.01 align:#left
		)
		group "Flare 6"
		(
			edittext  tex_file_name_et6 "texture:"
			button    browse6           "Browse..."
			checkbox  check6            "enabled"
			spinner   UISize6           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos6			"pos  :"  range:[0,1,0.6]   type:#float scale:0.01 align:#left
		)
		group "Flare 7"
		(
			edittext  tex_file_name_et7 "texture:"
			button    browse7           "Browse..."
			checkbox  check7            "enabled"
			spinner   UISize7           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos7			"pos  :"  range:[0,1,0.7]   type:#float scale:0.01 align:#left
		)		
		group "Flare 8"
		(
			edittext  tex_file_name_et8 "texture:"
			button    browse8           "Browse..."
			checkbox  check8            "enabled"
			spinner   UISize8           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos8			"pos  :"  range:[0,1,0.8]   type:#float scale:0.01 align:#left
		)		
		group "Flare 9"
		(
			edittext  tex_file_name_et9 "texture:"
			button    browse9           "Browse..."
			checkbox  check9            "enabled"
			spinner   UISize9           "size"  range:[0,500,1] type:#float scale:0.01
			spinner   UIPos9			"pos  :"  range:[0,1,0.9]   type:#float scale:0.01 align:#left
		)		
		
		fn updateUI =
		(
			UISizeDisappear.enabled = scaleWhenDisappear
			tex_file_name_et0.text = texFileName0
			tex_file_name_et1.text = texFileName1
	 		tex_file_name_et2.text = texFileName2
			tex_file_name_et3.text = texFileName3
			tex_file_name_et4.text = texFileName4
			tex_file_name_et5.text = texFileName5
	 		tex_file_name_et6.text = texFileName6
			tex_file_name_et7.text = texFileName7
			tex_file_name_et8.text = texFileName8
			tex_file_name_et9.text = texFileName9
			UIOcclusionTestMesh.text = occlusionTestMesh
		)

		on UIScaleWhenDisappear changed val do ( updateUI() )
		on UIOcclusionTestMesh changed val do ( occlusionTestMesh = val )
		on tex_file_name_et0 changed val do ( texFileName0 = val )
		on tex_file_name_et1 changed val do ( texFileName1 = val )
		on tex_file_name_et2 changed val do ( texFileName2 = val )
		on tex_file_name_et3 changed val do ( texFileName3 = val )
		on tex_file_name_et4 changed val do ( texFileName4 = val )
		on tex_file_name_et5 changed val do ( texFileName5 = val )
		on tex_file_name_et6 changed val do ( texFileName6 = val )
		on tex_file_name_et7 changed val do ( texFileName7 = val )
		on tex_file_name_et8 changed val do ( texFileName8 = val )
		on tex_file_name_et9 changed val do ( texFileName9 = val )
		
		on param open do
		(
			updateUI()			
		)
		
		on browse0 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName0 = fname 
			  tex_file_name_et0.text = texFileName0
			)			
		)
		on browse1 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName1 = fname 
			  tex_file_name_et1.text = texFileName1
			)			
		)
		on browse2 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName2 = fname 
			  tex_file_name_et2.text = texFileName2
			)			
		)
		on browse3 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName3 = fname 
			  tex_file_name_et3.text = texFileName3
			)			
		)
		on browse4 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName4 = fname 
			  tex_file_name_et4.text = texFileName4
			)			
		)
		on browse5 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName5 = fname 
			  tex_file_name_et5.text = texFileName5
			)			
		)
		on browse6 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName6 = fname 
			  tex_file_name_et6.text = texFileName6
			)			
		)
		on browse7 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName7 = fname 
			  tex_file_name_et7.text = texFileName7
			)			
		)
		on browse8 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName8 = fname 
			  tex_file_name_et8.text = texFileName7
			)			
		)
		on browse9 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName9 = fname 
			  tex_file_name_et9.text = texFileName7
			)			
		)

		on UIBrowseOcclusionMesh pressed do
		(
			fname = getOpenFileName caption: "shape file" types:"shape(*.shape)|" filename:"*.shape"
			if not fname == undefined then
			(
			  occlusionTestMesh = fname 
			  UIOcclusionTestMesh.text = occlusionTestMesh
			)
		)
	)
)
