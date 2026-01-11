-- config file for the r2 environment


verboseDebugInfo = false       -- when set to true, debug infos will be preceded by the number of the line that generated them
traceFunctions   = false      -- when set to true, each function enter / function leave are dumped to the stack


-- TODO nico : a table with predefined colors
r2.Config = 
{	
	PrimDisplayEnabled		= true,	     -- should route and zones be displayed ?
	FloatingShapeRefScale	= 1.0,       -- size of the floating mesh displayed at the mouse position when view-ray didn't hit a valid pos (in creation mode)
	RegionFadeTimeInMs		= 300,       -- time in ms needed for a region to get higlighted
	TestPaletteSelection    = false,     -- complete the palette for a test for selection by ecosystem & level	
	VerboseGetUI            = false,     -- some getUI can be made silent when they fail by turning this option to 'false'
	ResetWindowPos			= true,		 -- TMP : should windows pos be reseted at startup ?
	ActMaxQuota				= 99,        -- 'hardcoded" : max content that can be added into an act (checked by server too, but value is not taken from this file of course ..)
	-- decals look	
	HightlightDecalLook = 
	{
		DurationInMs = 1000,
		Texture = "r2_highlight_decal.tga",
		EndScaleFactor = 1,
		EndAngleInDegrees = 0,
		StartDiffuse = CRGBA(255, 255, 255),
		EndDiffuse = CRGBA(127, 0, 255),
		StartEmissive = CRGBA(0, 0, 0),
		EndEmissive = CRGBA(0, 0, 0),
	},
	SelectDecalLook = 
	{
		DurationInMs = 1000,
		Texture = "r2_select_decal.tga",
		EndScaleFactor = 1.2,
		EndAngleInDegrees = 0,
		StartDiffuse = CRGBA(255, 0, 0),
		EndDiffuse = CRGBA(255, 0, 0, 127),
		StartEmissive = CRGBA(0, 0, 0),
		EndEmissive = CRGBA(127, 127, 127),
	},
	SelectingDecalLook = 
	{
		DurationInMs = 200,
		Texture = "r2_select_decal.tga",
		EndScaleFactor = 1.2,
		EndAngleInDegrees = 90,
		StartDiffuse = CRGBA(255, 255, 255),
		EndDiffuse = CRGBA(255, 255, 255, 0),
		StartEmissive = CRGBA(0, 0, 0),
		EndEmissive = CRGBA(0, 0, 0)
	},
	PionneerDecalLook = 
	{
		DurationInMs = 3000,
		Texture = "r2_pionneer_edit.tga",
		EndScaleFactor = 1.05,
		EndAngleInDegrees = 0,
		StartDiffuse = CRGBA(255, 0, 0, 255),
		EndDiffuse = CRGBA(255, 0, 0, 192),
		StartEmissive = CRGBA(0, 64, 0),
		EndEmissive = CRGBA(0, 0, 0)
	},
	--
	MapEntityInvalidTexture = "r2_icon_map_invalid.tga",		   -- over displayed on minimap when object in on invalid pos
	MapEntityInvalidTextureSmall = "r2_icon_map_invalid_small.tga", -- over displayed on minimap when object in on invalid pos, small version
	MapEntityFrozenColor = CRGBA(0,  255,  255,  80),
	MapEntityLockedColor = CRGBA(127,  0,  200,  80),
	ArrayInstanceColor = CRGBA(255, 255, 255, 127),
	MapEntityDefaultTexture  = "brick_default.tga";        -- Bitmap to be used when one of the following bitmaps wasn't found
	MapEntitySelectTexture   = "r2_icon_select.tga",       -- The 'circle' bitmap draw on the map over selected / highlighted entities
	MapEntityHighlightColor  = CRGBA(255, 255, 255, 255),      -- Color of selection bitmap over an entity on the map
	MapEntitySelectColor     = CRGBA(255, 0, 0, 255),  -- Color of highlight bitmap over an entity on the map
	MapEntityFarTexture      = "r2_icon_far.tga",          -- Texture to display on the border of the map when the current selection is out 
	                                                       -- ... of the current visible world part in the map
	MapEntityFarArrowSize	 = 10,                         -- Size of the "MapEntityFarTexture" texture in pixels (scale applied ..)
	MapEntitySmallTexture    = "r2_icon_map_entity_small.tga",    -- Texture for entities when map is zoomed out
	MapEntitySmallHighlightTexture = "r2_icon_map_entity_small_highlight.tga",
	MapEntityOrientTexture   = "r2_icon_map_entity_orient.tga", -- arrow to signal entity orientation in close view
	MapEntityOrientOriginDist = 9,							-- distance  from entity to its orient texture
	MapEntityOrientOriginDistSmall = 5,					   -- distance  from entity to its orient texture when the entity is small and wihle it is being rotated
	MapEntityOrientBlendTimeInMs = 300,					   -- time that the orientation arrow need to get back to alpha = 0 when rotation is stopped		
	MapEntityCloseDist		 = 0.24,					   -- number of meter per pixel for the close view
	MapGlowStarTexture		 = "r2_glow_star.tga",         -- Texture displayed in the border of the map to signal that an object is too far to be seen
	MapGlowStarSpeed1		 = 0.50,                       -- rotation speed of first  map glow star
	MapGlowStarSpeed2		 = -0.60,                      -- rotation speed of second map glow star
	MapGlowStarSize          = 7,                           -- size of glow star 	
	-- color of regions in the 3D scene
	FocusedRegionColor = CRGBA(63,  127,  255,  100),
	SelectedRegionColor = CRGBA(192,  127,  64,  100),
	UnselectedRegionColor = CRGBA(0,  0,  255,  80),
	FrozenRegionColor = CRGBA(0,  255,  255,  80),
	LockedRegionColor = CRGBA(127,  0,  200,  80),
	-- colors for selected / focused entities / instance in scene (such as creature & npcs)
	FocusedInstanceColor = CRGBA(200,  32,  64, 127),
	SelectedInstanceColor = CRGBA(127,  127,  127),
	UnselectedInstanceColor = CRGBA(0,  0,  0),
	FrozenInstanceColor = CRGBA(0,  255,  255,  80),
	LockedInstanceColor = CRGBA(127,  0,  200,  80),
	-- world map auto pan
	MapAutoPanBorder = 20,  -- number of pixels in the border of the map for which the auto pan is tested
	MapAutoPanDeltaInMs = 300,		-- delta between each map pan when the mouse is on an auto-pan region
	MapAutoPanFastDeltaInMs = 100,  -- delta between each map pan when fast pan is active
	MapAutoPanSpeedInPixels = 15,   -- number of pixel to pan the map
	MapAutoFastPanNumTicks = 4,     -- number of 'slow' pan to do before entering fast pan mode
	-- foot steps / wander look
	FootStepMapTexture = "r2_map_foot_steps.tga",	
	FootStepDecalTexture = "r2_foot_steps.tga",
	FootStepMapWidth = 3,
	FootStepDecalUScale = 1.5,
	FootStepDecalWidth = 0.15,	
	FootStepDecalSelectedColor = CRGBA(255, 255, 255, 255),
	FootStepDecalHiddenColor = CRGBA(0, 0, 255, 120),
	FootStepDecalFocusedColor = CRGBA(255, 255, 255, 127),
	FootStepMapSelectedColor = CRGBA(180, 0, 192, 255),
	FootStepMapHiddenColor = CRGBA(150, 0, 192, 100),
	FootStepMapFocusedColor = CRGBA(150, 0, 192, 160),
	WanderDecalTexture = "r2_wander.tga",
	WanderDecalSize = 0.55,
	AutoGroupMaxDist = 4,
	-- display of inaccessible pos on map
	InaccessiblePosColor0			= CRGBA(255, 0, 0, 255),
	InaccessiblePosColor1			= CRGBA(200, 217, 0, 255),
	InaccessiblePosAnimDurationInMS = 500,
	DecalTopBlendStartDist = 3,		-- Distance in meters at which the color starts to fade for player & selection decals (at top)
	DecalBottomBlendStartDist = 1,  -- Distance in meters at which the color starts to fade for player & selection decals (at bottom)
	DecalBlendLength = 1.5			-- Length for decal color fading (player & selection decals)
}