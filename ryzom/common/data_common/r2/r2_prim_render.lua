-- Definition file for primitives rendering

-- enums
r2.PrimRender = {}
r2.PrimRender.Shape = { Star = 0, PolyLine = 1, ClosedPolyLine = 2 }
r2.PrimRender.WrapMode = 
{ 
	Repeat = 0, 
	Centered = 1, -- centered & clamped texture
	Scaled = 2    -- the texture corver the whole quad surface on the world map (not supported for decals, resumes to Repeat)
}


-- definition of a primitive look
-- should be passed as a parameter to the visual displayer of type R2ED::CDisplayerVisualGroup
r2.DefaultPrimLook =
{
	Shape = 	r2.PrimRender.Shape.Star,
	VertexShapeName = "",
	VertexShapeScale = 1,
	VertexLook =
	{
		DecalTexture = "",
		DecalSize = 1,
		DecalColor = CRGBA(255, 255, 255),
		DecalDistToEdgeDecal = 0,
		WorldMapTexture = "",
		WorldMapColor = CRGBA(255, 255, 255)
	},
	FirstVertexLook =
	{
		DecalTexture = "",
		DecalSize = 1,
		DecalColor = CRGBA(255, 255, 255),
		DecalDistToEdgeDecal = 0,
		WorldMapTexture = "",
		WorldMapColor = CRGBA(255, 255, 255)
	},
	EdgeLook =
	{
		ShapeName = "",
		ShapeScale = 1,
		DecalTexture = "",
		DecalUScale = 1,
		DecalWidth = 1,
		WorldMapTexture = "",
		WorldMapWidth = 0,
		WorldMapColor = CRGBA(255, 255, 255),
		WrapMode = r2.PrimRender.WrapMode.Repeat,
		VorldMapFiltered = true
	},
	LastEdgeIsValid = true
}

-- look for region
r2.PrimRender.RegionLook = 
{
	Shape =	r2.PrimRender.Shape.ClosedPolyLine,	
	VertexLook = 
	{	
	},
	EdgeLook =
	{
		DecalTexture = "r2_zone_edge.tga",
		DecalColor  = CRGBA(0, 255, 0, 255),
		DecalUScale = 2,
		DecalWidth = 0.1,
		WrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapTexture = "r2_map_zone_edge.tga",
		WorldMapWidth = 1,
		WorldMapColor = CRGBA(0, 255, 0, 255)
	}	
}

-- look for invalid region (self intersecting)
r2.PrimRender.RegionInvalidLook = 
{
	Shape =	r2.PrimRender.Shape.ClosedPolyLine,	
	VertexLook = 
	{
		DecalTexture = "r2_zone_vertex_invalid.tga",
		DecalSize = 0.2,
	},
	EdgeLook =
	{		
		DecalTexture = "r2_zone_edge.tga",
		DecalColor  = CRGBA(255, 0, 255),
		DecalUScale = 2,
		DecalWidth = 0.1,
		WrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapTexture = "r2_map_edge_stipple.tga",
		WorldMapWrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapUScale = 0.5,
		WorldMapWidth = 1,
		WorldMapColor = CRGBA(127, 127, 127)		
	}
}

-- look for region being drawn
r2.PrimRender.RegionCreateLook = clone(r2.PrimRender.RegionLook)
r2.PrimRender.RegionCreateLook.VertexShapeName = "r2_region_vertex.shape"
r2.PrimRender.RegionCreateLook.VertexShapeScale = 0.25
r2.PrimRender.RegionCreateLook.VertexShapeScale = 0.25
r2.PrimRender.RegionCreateLook.VertexLook.WorldMapTexture ="r2_icon_map_entity_small.tga"
r2.PrimRender.RegionCreateLook.Shape =	r2.PrimRender.Shape.PolyLine
--r2_icon_map_entity_small.tga
r2.PrimRender.RegionCreateInvalidLook= clone(r2.PrimRender.RegionInvalidLook)
r2.PrimRender.RegionCreateInvalidLook.VertexShapeName = ""
r2.PrimRender.RegionCreateInvalidLook.Shape = r2.PrimRender.Shape.PolyLine
r2.PrimRender.RegionCreateInvalidLook.VertexLook.WorldMapTexture ="r2_icon_map_entity_small.tga"
r2.PrimRender.RegionCreateInvalidLook.EdgeLook.WorldMapColor = CRGBA(255, 0, 0)
-- look when a region can be closed (mouse is on first vertex and there are at least 3 points)
r2.PrimRender.RegionCreateCanCloseLook = clone(r2.PrimRender.RegionCreateLook)
r2.PrimRender.RegionCreateCanCloseLook.EdgeLook.DecalColor = CRGBA(255, 255, 0)
r2.PrimRender.RegionCreateCanCloseLook.EdgeLook.WorldMapColor = CRGBA(255, 255, 0)
r2.PrimRender.RegionCreateCanCloseLook.VertexLook.WorldMapTexture ="r2_icon_map_entity_small.tga"
r2.PrimRender.RegionCreateCanCloseLook.Shape =	r2.PrimRender.Shape.ClosedPolyLine



-- look for road
r2.PrimRender.RoadLook = 
{
	Shape = r2.PrimRender.Shape.PolyLine,	
	VertexLook = 
	{	
	},
	EdgeLook = 
	{		
		DecalTexture = "r2_path.tga",
		DecalColor = CRGBA(0, 127, 255, 160),
		DecalUScale = 0.25,
		DecalWidth = 0.4,
		DecalWrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapTexture = "r2_map_edge_arrow.tga",
		WorldMapWrapMode = r2.PrimRender.WrapMode.Centered,
		WorldMapUScale = 0.3,
		WorldMapWidth = 3.5,
		WorldMapColor = CRGBA(0, 127, 255, 160)
	},
	ClipDownFacing = true
}

r2.PrimRender.RoadLookInvalid = clone(r2.PrimRender.RegionInvalidLook)
r2.PrimRender.RoadLookInvalid.EdgeLook.WorldMapColor = CRGBA(127, 127, 127, 127)
r2.PrimRender.RoadLookInvalid.EdgeLook.WorldMapTexture = "r2_map_edge_stipple.tga"
r2.PrimRender.RoadLookInvalid.Shape = r2.PrimRender.Shape.PolyLine
r2.PrimRender.RoadLookInvalid.ClipDownFacing = true	

r2.PrimRender.RoadLookInaccessible = clone(r2.PrimRender.RoadLookInvalid)
r2.PrimRender.RoadLookInaccessible.EdgeLook.WorldMapTexture = "*accessibility_texture*"
r2.PrimRender.RoadLookInaccessible.EdgeLook.DecalTexture = "*accessibility_texture*"
r2.PrimRender.RoadLookInaccessible.EdgeLook.WorldMapcolor = CRGBA(255, 0, 255, 255)
r2.PrimRender.RoadLookInaccessible.EdgeLook.WorldMapWidth = 2
r2.PrimRender.RoadLookInaccessible.EdgeLook.WorldMapFiltered = false
r2.PrimRender.RoadLookInaccessible.EdgeLook.DecalFiltered = false
r2.PrimRender.RoadLookInaccessible.ClipDownFacing = true	





-- look for road being drawn
r2.PrimRender.RoadCreateLook = clone(r2.PrimRender.RoadLook)
r2.PrimRender.RoadCreateLook.VertexShapeName = "r2_road_flag.shape"
r2.PrimRender.RoadCreateLook.VertexShapeScale = 0.4
r2.PrimRender.RoadCreateLook.VertexLook.WorldMapTexture ="r2_icon_map_entity_small.tga"
r2.PrimRender.RoadCreateInvalidLook = clone(r2.PrimRender.RegionCreateInvalidLook)



-- look for group
r2.PrimRender.GroupLook = 
{
	Shape = 	r2.PrimRender.Shape.Star,
	VertexLook = 
	{
		DecalTexture = "r2_selection_circle.tga",
		DecalSize = 0.5,
		DecalDistToEdgeDecal = 0.45,
		DecalColor = CRGBA(0, 255, 0, 255),
	},
	FirstVertexLook =
	{
		DecalTexture = "r2_selection_circle_double.tga",
		DecalSize = 0.7,
		DecalDistToEdgeDecal = 0.62,
		DecalColor = CRGBA(0, 255, 0, 255),
	},
	EdgeLook =
	{
		DecalTexture = "r2_arrow.tga",
		DecalUScale = 0.5,
		DecalWidth = 0.4,
		DecalWrapMode = r2.PrimRender.WrapMode.Centered,
		DecalColor = CRGBA(0, 255, 0, 255),
		WorldMapTexture = "r2_map_edge_arrow.tga",
		WorldMapWrapMode = r2.PrimRender.WrapMode.Centered,
		WorldMapUScale = -0.6,
		WorldMapWidth = 2.5,
		WorldMapColor = CRGBA(0, 255, 0, 160)
	},
	ClipDownFacing = true
}

-- look for auto group
r2.PrimRender.AutoGroupLook = clone(r2.PrimRender.GroupLook)
r2.PrimRender.AutoGroupLook.VertexLook.DecalColor = CRGBA(0, 255, 0, 80)
r2.PrimRender.AutoGroupLook.FirstVertexLook.DecalColor = CRGBA(0, 255, 0, 80)
r2.PrimRender.AutoGroupLook.EdgeLook.DecalColor = CRGBA(0, 255, 0, 80)
r2.PrimRender.AutoGroupLook.EdgeLook.WorldMapColor = CRGBA(0, 255, 0, 80)


-- hightlight / selection boxs
r2.PrimRender.SelectBoxLook = 
{
	Shape = 	r2.PrimRender.Shape.ClosedPolyLine,
	VertexLook = 
	{
		DecalTexture = "r2_zone_corner.tga",
		DecalSize = 0.1,
		--DecalDistToEdgeDecal = 0.45,
		DecalColor = CRGBA(255, 0, 0, 255),
	},
	FirstVertexLook =
	{
		DecalTexture = "r2_zone_corner.tga",
		DecalSize = 0.1,
		--DecalDistToEdgeDecal = 0.62,
		DecalColor = CRGBA(255, 0, 0, 255),
	},
	EdgeLook =
	{
		DecalTexture = "r2_zone_edge.tga",
		DecalUScale = 0.5,
		DecalWidth = 0.1,
		DecalWrapMode = r2.PrimRender.WrapMode.Centered,
		DecalColor = CRGBA(255, 0, 0, 255),
		WorldMapTexture = "r2_map_edge_arrow.tga",
		WorldMapWrapMode = r2.PrimRender.WrapMode.Centered,
		WorldMapUScale = -0.6,
		WorldMapWidth = 2.5,
		WorldMapColor = CRGBA(255, 0, 0, 40)
	}
}

-- nb : edge colors ignored for highlight & slect box : those are taken from hightlight & select decal instead (they are animated)

r2.PrimRender.HighlightBoxLook = clone(r2.PrimRender.SelectBoxLook)
r2.PrimRender.HighlightBoxLook.VertexLook.DecalColor = CRGBA(255, 255, 255, 255)
r2.PrimRender.HighlightBoxLook.FirstVertexLook.DecalColor = CRGBA(255, 255, 255, 255)
r2.PrimRender.HighlightBoxLook.EdgeLook.DecalColor = CRGBA(255, 255, 255, 255)
r2.PrimRender.HighlightBoxLook.EdgeLook.WorldMapColor = CRGBA(255, 255, 255, 40)


r2.PrimRender.SelectBoxLook.EdgeLook.DecalTexture = "r2_select_edge.tga"
r2.PrimRender.SelectBoxLook.EdgeLook.DecalUScale = 2
r2.PrimRender.SelectBoxLook.EdgeLook.DecalWidth = 0.15
r2.PrimRender.SelectBoxLook.VertexLook.DecalSize = 0.15
r2.PrimRender.SelectBoxLook.FirstVertexLook.DecalSize = 0.15


r2.PrimRender.ComponentRegionLook = 
{
	Shape =	r2.PrimRender.Shape.ClosedPolyLine,	
	VertexLook = 
	{	
	},
	EdgeLook =
	{
		DecalTexture = "r2_zone_edge.tga",
		DecalColor  = CRGBA(0, 255, 0, 127),
		DecalUScale = 2,
		DecalWidth = 0.1,
		WrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapTexture = "r2_map_zone_edge.tga",
		WorldMapWidth = 1,
		WorldMapColor = CRGBA(0, 255, 0, 32)
	}	
}

r2.PrimRender.ComponentRegionInvalidLook = 
{
	Shape =	r2.PrimRender.Shape.ClosedPolyLine,	
	VertexLook = 
	{	
	},
	EdgeLook =
	{	
		DecalTexture = "r2_zone_edge.tga",
		DecalColor  = CRGBA(255, 0, 0, 127),
		DecalUScale = 2,
		DecalWidth = 0.1,
		WrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapTexture = "r2_map_edge_stipple.tga",
		WorldMapWrapMode = r2.PrimRender.WrapMode.Repeat,
		WorldMapUScale = 0.5,
		WorldMapWidth = 1,
		WorldMapColor = CRGBA(255, 0, 0, 32)
	}	
}



