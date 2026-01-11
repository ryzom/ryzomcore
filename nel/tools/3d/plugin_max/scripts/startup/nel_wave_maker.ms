-- this script is used to place a wave maker
plugin Geometry nel_wave_maker
name:"Wave Maker"
category:"Nel Tools"
classID:#(0x77e24828, 0x329a1de5)
extends:Sphere
(
	parameters param rollout:main	
	(
		period			 type:#Float   	default: 1      ui: cPeriod
		intensity		 type:#Float   	default: 1      ui: cIntensity
		radius		 	 type:#Float   	default: 3      ui: cRadius
		poolID			 type:#Integer  default: 0      ui: cPoolID
		impulsionMode    type:#boolean  default: true   ui: cImpulsionMode
	)
	
	rollout main "Parameters"
	(
		Spinner 	cPeriod 	   "period         : "	   range:[0.1, 1000, 1]   type:#float scale: 0.1 align: #right
		Spinner 	cIntensity 	   "intensity      : "	   range:[-100, 100, 1]   type:#float scale: 0.1 align: #right
		Spinner 	cRadius  	   "radius         : "	   range:[0, 1000, 1]   type:#float scale: 0.1 align: #right
		Spinner 	cPoolID        "pool ID        : "	   range:[0, 3, 0] 		type:#integer scale: 1 align: #right
		CheckBox	cImpulsionMode "impulsion mode : "     align: #right
	)
)
