


-- tmp for nico's tests
local function formatLevel(level)
	local result = tostring(level)
	while string.len(result) ~= 3 do
		result = "0" .. result
	end
	return result		
end

local function formatSelectionTextName(level, ecosystem)
	return "palette.selection_test.level_" .. formatLevel(level) .. "_" .. tostring(ecosystem)
end

local levelStep = 10
local ecosystemTable = { "", "Desert", "Forest", "Jungle", "Lacustre", "PrimeRoots", "Goo" }

r2.loadPalette = function()
	local entries = {}

	-- test entries  for levels & ecosystems
	if r2.Config.TestPaletteSelection then
		for ecosystem = 1, 4 do
			for level = 1,250, levelStep do
				local npc = 
				{
					Name="Npc",
					Equipment="",
					IsStuck=0,
					SheetClient="fyros_race_stats",
					Level=level,
					Ecosystem = ecosystemTable[ecosystem],
					AiMovement="stand_on_start_point",
					Profile="no_change",
					Angle=0,
					Position={x=0, y=0, z=0}
				}
				r2.addPaletteElement(formatSelectionTextName(level, ecosystem), npc)
			end
		end
	end

	-- base class
	entries.npc = 
	{
		Name="",
		Equipment="",
		SheetClient="undef",
		Level=1,
		AiMovement="stand_on_start_point",
--		AiActivity="guard", user Profile instead
		IsStuck=0,

		GabaritHeight = 0,
		GabaritTorsoWidth = 0,
		GabaritArmsWidth = 0,
		GabaritLegsWidth = 0,
		GabaritBreastSize = 0,

		HairType = 0,
		HairColor = 0,
		Tattoo = 0,
		EyesColor = 0,

		MorphTarget1 = 0,
		MorphTarget2 = 0,
		MorphTarget3 = 0,
		MorphTarget4 = 0,
		MorphTarget5 = 0,
		MorphTarget6 = 0,
		MorphTarget7 = 0,
		MorphTarget8 = 0,

		Sex = 0,
		JacketModel = 0,
		TrouserModel = 0,
		FeetModel = 0,
		HandsModel = 0,
		ArmModel = 0,
		WeaponRightHand = 0,
		WeaponLeftHand = 0,

		JacketColor = 0,
		ArmColor = 0,
		HandsColor = 0,
		TrouserColor = 0,
		FeetColor =0, 

		Function = "",
		Level = 1,
		Profile = "guard",
		Speed = 1,
		Aggro = 30,
		PlayerAttackable = 1,
		BotAttackable = 1
			
--		Angle=0,
--		Position={x=0, y=0, z=0}
	}

	r2.addPaletteElement("palette.entities.npc", entries.npc) 



	
	
	entries.fyros_male = 
	{
		--Customization="toto",
		Name="Fyros Male",
		Class="NpcCustom",
		Type="base",
		Base="palette.entities.npc",
		Equipment="fyros_equipment",
		SheetClient="basic_fyros_male.creature"

	}
	r2.addPaletteElement("palette.entities.players.fyros_male", entries.fyros_male)

	entries.fyros_equipment =
	{
		helmet = 
		{
			["heavy helmet"] = "icfahh.sitem"
		},

		chest_plate = 
		{
			["refugee vest"] =	"icravr.sitem",
			["civilian vest"] =	"icfacv_3.sitem",
			["light vest"] =	"icfalv.sitem",
			["light vest II"] =	"icfalv_3.sitem",
			["medium vest"] =	"icfamv.sitem",
			["medium vest II"] =	"icfamv_3.sitem",
			["heavy vest"] =	"icfahv.sitem",
			["heavy vest II"] =	"icfahv_3.sitem"
		},

		legs = 
		{
			["light pants"] = "icfalp.sitem",
			["medium pants"] = "icfamp.sitem",
			["heavy pants"] = "icfahp.sitem",
			["pants 4"] = "icfacp_3.sitem",
			["pants 5"] = "icfahp_3.sitem",
			["pants 6"] = "icfamp_3.sitem",
			["pants 7"] = "icfalp_3.sitem"
		},

		boots = 
		{
			["light boots"] = "icfalb.sitem",
			["medium boots"] = "icfamb.sitem",
			["heavy boots"] = "icfahb.sitem",
			["boots 4"] = "icfacb_3.sitem",
			["boots 5"] = "icfahb_3.sitem",
			["boots 6"] = "icfamb_3.sitem",
			["boots 7"] = "icfalb_3.sitem"
		},

		gloves = 
		{
			["light gloves"] = "icfalg.sitem",
			["medium gloves"] = "icfamg.sitem",
			["heavy gloves"] = "icfahg.sitem"
		},

		arms_guard = 
		{
			["light sleeves"] = "icfals.sitem",
			["medium sleeves"] = "icfams.sitem",
			["heavy sleeves"] = "icfahs.sitem",
			["sleeves 4"] = "icfacs_3.sitem",
			["sleeves 5"] = "icfahs_3.sitem",
			["sleeves 6"] = "icfams_3.sitem",
			["sleeves 7"] = "icfals_3.sitem"
		},

		right_hand = 
		{
			["dagger"] = "iccm1pd.sitem",
			["mace"] = "iccm1bm.sitem",
			["dagger"] = "iccm1pd.sitem",
			["axe"] = "iccm1sa.sitem",
			["sword"] = "iccm1ss.sitem"
		},

		left_hand = 
		{
			["dagger"] = "iccm1pd.sitem"
		}
	}
	r2.addR2PlayerEquipment(entries.fyros_male, entries.fyros_equipment)

	entries.fyros_female = 
	{
		Name="Fyros Female",
		Class="NpcCustom",
		Type="base",
		Equipment="fyros_equipment",
		Base="palette.entities.npc",
		SheetClient="basic_fyros_female.creature"
	}
	r2.addPaletteElement("palette.entities.players.fyros_female", entries.fyros_female)
	r2.addR2PlayerEquipment(entries.fyros_female, entries.fyros_equipment)


	entries.matis_male = 
	{
		Name="Matis Male",
		Class="NpcCustom",
		Type="base",
		Equipment="fyros_equipment",
		Base="palette.entities.npc",
		SheetClient="basic_matis_male.creature"
		
	}
	r2.addPaletteElement("palette.entities.players.matis_male", entries.matis_male)

	entries.matis_equipment =
	{
		helmet = 
		{
		
		},

		chest_plate = 
		{
			["vest 1"] = "icmacv_3.sitem",
			["vest 2"] = "icmahv_3.sitem",
			["vest 3"] = "icmamv_3.sitem",
			["vest 4"] = "icmalv_3.sitem",
			["vest 5"] = "icravr.sitem"
		},

		legs = 
		{
			["pants 1"] = "icmacp_3.sitem",
			["pants 2"] = "icmahp_3.sitem",
			["pants 3"] = "icmamp_3.sitem",
			["pants 4"] = "icmalp_3.sitem"
		},

		boots = 
		{
			["boots 1"] = "icmacb_3.sitem",
			["boots 2"] = "icmahb_3.sitem",
			["boots 3"] = "icmamb_3.sitem",
			["boots 4"] = "icmalb_3.sitem"
		},

		gloves = 
		{
		
		},

		arms_guard = 
		{
			["sleeves 1"] = "icmacs_3.sitem",
			["sleeves 2"] = "icmahs_3.sitem",
			["sleeves 3"] = "icmams_3.sitem",
			["sleeves 4"] = "icmals_3.sitem"
		},

		right_hand = 
		{
			["dagger"] = "iccm1pd.sitem",
			["mace"] = "iccm1bm.sitem",
			["dagger"] = "iccm1pd.sitem",
			["axe"] = "iccm1sa.sitem",
			["sword"] = "iccm1ss.sitem"
		},

		left_hand = 
		{
			["dagger"] = "iccm1pd.sitem"
		}
	}
	--r2.addR2PlayerEquipment(entries.matis_male, entries.matis_equipment)
	r2.addR2PlayerEquipment(entries.matis_male, entries.fyros_equipment)

	entries.matis_female = 
	{
		Name="Matis Female",
		Class="NpcCustom",
		Type="base",
		Base="palette.entities.npc",
		Equipment="matis_equipment",
		SheetClient="basic_matis_female.creature",

	}
	r2.addPaletteElement("palette.entities.players.matis_female", entries.matis_female)
	r2.addR2PlayerEquipment(entries.matis_female, entries.matis_equipment)

	entries.tryker_male = 
	{
		Name="Tryker Male",
		Class="NpcCustom",
		Type="base",
		Base="palette.entities.npc",
		Equipment="tryker_equipment",
		SheetClient="basic_tryker_male.creature",
	}
	r2.addPaletteElement("palette.entities.players.tryker_male", entries.tryker_male)

	entries.tryker_equipment =
	{
		helmet = 
		{
			
		},

		chest_plate = 
		{
			["vest 1"] = "ictacv_3.sitem",
			["vest 2"] = "ictahv_3.sitem",
			["vest 3"] = "ictamv_3.sitem",
			["vest 4"] = "ictalv_3.sitem",
		},

		legs = 
		{
			["pants 1"] = "ictacp_3.sitem",
			["pants 2"] = "ictahp_3.sitem",
			["pants 3"] = "ictamp_3.sitem",
			["pants 4"] = "ictalp_3.sitem"
		},

		boots = 
		{
			["boots 1"] = "ictacb_3.sitem",
			["boots 2"] = "ictahb_3.sitem",
			["boots 3"] = "ictamb_3.sitem",
			["boots 4"] = "ictalb_3.sitem"
		},

		gloves = 
		{
			
		},

		arms_guard = 
		{
			["sleeves 1"] = "ictacs_3.sitem",
			["sleeves 2"] = "ictahs_3.sitem",
			["sleeves 3"] = "ictams_3.sitem",
			["sleeves 4"] = "ictals_3.sitem"
		},

		right_hand = 
		{
			["dagger"] = "iccm1pd.sitem",
			["mace"] = "iccm1bm.sitem",
			["dagger"] = "iccm1pd.sitem",
			["axe"] = "iccm1sa.sitem",
			["sword"] = "iccm1ss.sitem"
		},

		left_hand = 
		{
			["dagger"] = "iccm1pd.sitem"
		}
	}
	r2.addR2PlayerEquipment(entries.tryker_male, entries.tryker_equipment)

	entries.tryker_female = 
	{
		Name="Tryker Female",
		Class="NpcCustom",
		Type="base",
		Base="palette.entities.npc",
		Equipment="tryker_equipment",
		SheetClient="basic_tryker_female.creature"				
	}
	r2.addPaletteElement("palette.entities.players.tryker_female", entries.tryker_female)
	r2.addR2PlayerEquipment(entries.tryker_female, entries.tryker_equipment)

	entries.zorai_male = 
	{
		Name="Zorai Male",
		Class="NpcCustom",
		Type="base",
		Base="palette.entities.npc",
		Equipment="zorai_equipment",
		SheetClient="basic_zorai_male.creature"
	}
	r2.addPaletteElement("palette.entities.players.zorai_male", entries.zorai_male)

	entries.zorai_equipment =
	{
		helmet = 
		{
			
		},

		chest_plate = 
		{
			["vest 1"] = "icmacv_3.sitem",
			["vest 2"] = "icmahv_3.sitem",
			["vest 3"] = "icmamv_3.sitem",
			["vest 4"] = "icmalv_3.sitem",
		},

		legs = 
		{
			["pants 1"] = "icmacp_3.sitem",
			["pants 2"] = "icmahp_3.sitem",
			["pants 3"] = "icmamp_3.sitem",
			["pants 4"] = "icmalp_3.sitem"
		},

		boots = 
		{
			["boots 1"] = "icmacb_3.sitem",
			["boots 2"] = "icmahb_3.sitem",
			["boots 3"] = "icmamb_3.sitem",
			["boots 4"] = "icmalb_3.sitem"
		},

		gloves = 
		{
		},

		arms_guard = 
		{
			["sleeves 1"] = "icmacs_3.sitem",
			["sleeves 2"] = "icmahs_3.sitem",
			["sleeves 3"] = "icmams_3.sitem",
			["sleeves 4"] = "icmals_3.sitem"
		},

		right_hand = 
		{
			["dagger"] = "iccm1pd.sitem",
			["mace"] = "iccm1bm.sitem",
			["dagger"] = "iccm1pd.sitem",
			["axe"] = "iccm1sa.sitem",
			["sword"] = "iccm1ss.sitem"
		},

		left_hand = 
		{
			["dagger"] = "iccm1pd.sitem"
		}
	}
	r2.addR2PlayerEquipment(entries.zorai_male, entries.zorai_equipment)

	entries.zorai_female = 
	{
		Name="Zorai Female",
		Class="NpcCustom",
		Type="base",
		Base="palette.entities.npc",
		Equipment="zorai_equipment",
		SheetClient="basic_zorai_female.creature"
	}
	r2.addPaletteElement("palette.entities.players.zorai_female", entries.zorai_female)
	r2.addR2PlayerEquipment(entries.zorai_female, entries.zorai_equipment)



-- bot objects start here -- 
	-- parent of all bot object (like palette.entities.npc for npcs)
	-- abstract element(cannot be instanciated)
	entries.botobject =
	{
	      IsStuck=1,			
              AiMovement="stand_on_start_point", 
              Profile="no_change"	      
	}
	r2.addPaletteElement("palette.entities.botobject", entries.botobject);



	entries.barrel1 =
	{
		Base="palette.entities.botobject",
		Name="Barrel 1",
		SheetClient="object_1_barrel_broken.creature",				
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.barrel1", entries.barrel1);

	entries.jar1 =
	{
		Base="palette.entities.botobject",
		Name="Jar 1",
		SheetClient="object_jar.creature",				
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.jar1", entries.jar1);

	entries.jar2 =
	{
		Base="palette.entities.botobject",
		Name = "Jar 2",
		SheetClient="object_jar_fallen.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.jar2", entries.jar2);

	entries.crate1 =
	{
		Base="palette.entities.botobject",
		Name="Crate 1",
		SheetClient="object_1_crate.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.crate1",entries.crate1)

	entries.crate2 =
	{
		Base="palette.entities.botobject",
		Name="Crate 2",
		SheetClient="object_3_crate.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.crate2",entries.crate2)

	entries.bones1 =
	{
		Base="palette.entities.botobject",
		Name="Bones 1",
		SheetClient="object_bones.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bones1",entries.bones1)
	
	entries.tent1 =
	{
		Base="palette.entities.botobject",
		Name="Tent 1",
		SheetClient="object_tent.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent1",entries.tent1)

	entries.pack1 =
	{
		Base="palette.entities.botobject",
		Name="Pack 1",
		SheetClient="object_pack_1.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.pack1",entries.pack1)

-- test Bot-object

	entries.jar=
	{
		Base="palette.entities.botobject",
		Name = "jar",
		SheetClient="object_jar.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.jar", entries.jar);
	entries.jar_3=
	{
		Base="palette.entities.botobject",
		Name = "jar_3",
		SheetClient="object_jar_3.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.jar_3", entries.jar_3);
	entries.jar_fallen=
	{
		Base="palette.entities.botobject",
		Name = "jar_fallen",
		SheetClient="object_jar_fallen.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.jar_fallen", entries.jar_fallen);
	entries.hut=
	{
		Base="palette.entities.botobject",
		Name = "hut",
		SheetClient="object_hut.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.hut", entries.hut);
	entries.paddock=
	{
		Base="palette.entities.botobject",
		Name = "paddock",
		SheetClient="object_paddock.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.paddock", entries.paddock);
	entries.totem_kami=
	{
		Base="palette.entities.botobject",
		Name = "totem_kami",
		SheetClient="object_totem_kami.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.totem_kami", entries.totem_kami);
	entries.totem_pachyderm=
	{
		Base="palette.entities.botobject",
		Name = "totem_pachyderm",
		SheetClient="object_totem_pachyderm.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.totem_pachyderm", entries.totem_pachyderm);
	entries.tent=
	{
		Base="palette.entities.botobject",
		Name = "tent",
		SheetClient="object_tent.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent", entries.tent);
	entries.tent_fyros=
	{
		Base="palette.entities.botobject",
		Name = "tent_fyros",
		SheetClient="object_tent_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent_fyros", entries.tent_fyros);
	entries.tent_matis=
	{
		Base="palette.entities.botobject",
		Name = "tent_matis",
		SheetClient="object_tent_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent_matis", entries.tent_matis);
	entries.tent_tryker=
	{
		Base="palette.entities.botobject",
		Name = "tent_tryker",
		SheetClient="object_tent_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent_tryker", entries.tent_tryker);
	entries.tent_zorai=
	{
		Base="palette.entities.botobject",
		Name = "tent_zorai",
		SheetClient="object_tent_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent_zorai", entries.tent_zorai);
	entries.totem_kitin=
	{
		Base="palette.entities.botobject",
		Name = "totem_kitin",
		SheetClient="object_totem_kitin.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.totem_kitin", entries.totem_kitin);
	entries.totem_bird=
	{
		Base="palette.entities.botobject",
		Name = "totem_bird",
		SheetClient="object_totem_bird.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.totem_bird", entries.totem_bird);
	entries.tower_ruin=
	{
		Base="palette.entities.botobject",
		Name = "tower_ruin",
		SheetClient="object_tower_ruin.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tower_ruin", entries.tower_ruin);
	entries.stele=
	{
		Base="palette.entities.botobject",
		Name = "stele",
		SheetClient="object_stele.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.stele", entries.stele);
	entries.chariot=
	{
		Base="palette.entities.botobject",
		Name = "chariot",
		SheetClient="object_chariot.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.chariot", entries.chariot);
	entries.chariot_working=
	{
		Base="palette.entities.botobject",
		Name = "chariot_working",
		SheetClient="object_chariot_working.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.chariot_working", entries.chariot_working);
	entries.wind_turbine=
	{
		Base="palette.entities.botobject",
		Name = "wind_turbine",
		SheetClient="object_wind_turbine.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.wind_turbine", entries.wind_turbine);
	entries.street_lamp=
	{
		Base="palette.entities.botobject",
		Name = "street_lamp",
		SheetClient="object_street_lamp.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.street_lamp", entries.street_lamp);
	entries.tomb_1=
	{
		Base="palette.entities.botobject",
		Name = "tomb_1",
		SheetClient="object_tomb_1.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tomb_1", entries.tomb_1);
	entries.tomb_2=
	{
		Base="palette.entities.botobject",
		Name = "tomb_2",
		SheetClient="object_tomb_2.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tomb_2", entries.tomb_2);
	entries.tomb_3=
	{
		Base="palette.entities.botobject",
		Name = "tomb_3",
		SheetClient="object_tomb_3.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tomb_3", entries.tomb_3);
	entries.tomb_4=
	{
		Base="palette.entities.botobject",
		Name = "tomb_4",
		SheetClient="object_tomb_4.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tomb_4", entries.tomb_4);
	entries.tomb_5=
	{
		Base="palette.entities.botobject",
		Name = "tomb_5",
		SheetClient="object_tomb_5.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tomb_5", entries.tomb_5);
	entries.campfire=
	{
		Base="palette.entities.botobject",
		Name = "campfire",
		SheetClient="object_campfire.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.campfire", entries.campfire);
	entries.campfire_out=
	{
		Base="palette.entities.botobject",
		Name = "campfire_out",
		SheetClient="object_campfire_out.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.campfire_out", entries.campfire_out);
	entries.chest=
	{
		Base="palette.entities.botobject",
		Name = "chest",
		SheetClient="object_chest.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.chest", entries.chest);
	entries.chest_old=
	{
		Base="palette.entities.botobject",
		Name = "chest_old",
		SheetClient="object_chest_old.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.chest_old", entries.chest_old);
	entries.stump=
	{
		Base="palette.entities.botobject",
		Name = "stump",
		SheetClient="object_stump.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.stump", entries.stump);
	entries.carrion_mammal=
	{
		Base="palette.entities.botobject",
		Name = "carrion_mammal",
		SheetClient="object_carrion_mammal.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.carrion_mammal", entries.carrion_mammal);
	entries.carrion_insect=
	{
		Base="palette.entities.botobject",
		Name = "carrion_insect",
		SheetClient="object_carrion_insect.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.carrion_insect", entries.carrion_insect);
	entries.bones=
	{
		Base="palette.entities.botobject",
		Name = "bones",
		SheetClient="object_bones.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bones", entries.bones);
	entries.bones_b=
	{
		Base="palette.entities.botobject",
		Name = "bones_b",
		SheetClient="object_bones_b.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bones_b", entries.bones_b);
	entries.barrier=
	{
		Base="palette.entities.botobject",
		Name = "barrier",
		SheetClient="object_barrier.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.barrier", entries.barrier);
	entries.barrier_T=
	{
		Base="palette.entities.botobject",
		Name = "barrier_T",
		SheetClient="object_barrier_T.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.barrier_T", entries.barrier_T);
	entries.house_ruin=
	{
		Base="palette.entities.botobject",
		Name = "house_ruin",
		SheetClient="object_house_ruin.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.house_ruin", entries.house_ruin);
	entries.roadsign=
	{
		Base="palette.entities.botobject",
		Name = "roadsign",
		SheetClient="object_roadsign.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.roadsign", entries.roadsign);
	entries.watch_tower=
	{
		Base="palette.entities.botobject",
		Name = "watch_tower",
		SheetClient="object_watch_tower.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.watch_tower", entries.watch_tower);
	entries.landslide_desert=
	{
		Base="palette.entities.botobject",
		Name = "landslide_desert",
		SheetClient="object_landslide_desert.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.landslide_desert", entries.landslide_desert);
	entries.tent_cosmetics=
	{
		Base="palette.entities.botobject",
		Name = "tent_cosmetics",
		SheetClient="object_tent_cosmetics.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.tent_cosmetics", entries.tent_cosmetics);
	entries.landslide_jungle=
	{
		Base="palette.entities.botobject",
		Name = "landslide_jungle",
		SheetClient="object_landslide_jungle.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.landslide_jungle", entries.landslide_jungle);
	entries.landslide_lake=
	{
		Base="palette.entities.botobject",
		Name = "landslide_lake",
		SheetClient="object_landslide_lake.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.landslide_lake", entries.landslide_lake);
	entries.carapace_bul=
	{
		Base="palette.entities.botobject",
		Name = "carapace_bul",
		SheetClient="object_carapace_bul.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.carapace_bul", entries.carapace_bul);
	entries.carapace_2=
	{
		Base="palette.entities.botobject",
		Name = "carapace_2",
		SheetClient="object_carapace_2.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.carapace_2", entries.carapace_2);
	entries.giant_skull=
	{
		Base="palette.entities.botobject",
		Name = "giant_skull",
		SheetClient="object_giant_skull.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.giant_skull", entries.giant_skull);
	entries.ruin_wall=
	{
		Base="palette.entities.botobject",
		Name = "ruin_wall",
		SheetClient="object_ruin_wall.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.ruin_wall", entries.ruin_wall);
	entries.ruin_wall_b=
	{
		Base="palette.entities.botobject",
		Name = "ruin_wall_b",
		SheetClient="object_ruin_wall_b.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.ruin_wall_b", entries.ruin_wall_b);
	entries.karavan_altar=
	{
		Base="palette.entities.botobject",
		Name = "karavan_altar",
		SheetClient="object_karavan_altar.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_altar", entries.karavan_altar);
	entries.kami_altar=
	{
		Base="palette.entities.botobject",
		Name = "kami_altar",
		SheetClient="object_kami_altar.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.kami_altar", entries.kami_altar);
	entries.milestone=
	{
		Base="palette.entities.botobject",
		Name = "milestone",
		SheetClient="object_milestone.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.milestone", entries.milestone);
	entries.bag_a=
	{
		Base="palette.entities.botobject",
		Name = "bag_a",
		SheetClient="object_bag_a.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bag_a", entries.bag_a);
	entries.bag_b=
	{
		Base="palette.entities.botobject",
		Name = "bag_b",
		SheetClient="object_bag_b.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bag_b", entries.bag_b);
	entries.pack_1=
	{
		Base="palette.entities.botobject",
		Name = "pack_1",
		SheetClient="object_pack_1.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.pack_1", entries.pack_1);
	entries.pack_2=
	{
		Base="palette.entities.botobject",
		Name = "pack_2",
		SheetClient="object_pack_2.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.pack_2", entries.pack_2);
	entries.pack_3=
	{
		Base="palette.entities.botobject",
		Name = "pack_3",
		SheetClient="object_pack_3.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.pack_3", entries.pack_3);
	entries.pack_4=
	{
		Base="palette.entities.botobject",
		Name = "pack_4",
		SheetClient="object_pack_4.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.pack_4", entries.pack_4);
	entries.pack_5=
	{
		Base="palette.entities.botobject",
		Name = "pack_5",
		SheetClient="object_pack_5.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.pack_5", entries.pack_5);
	entries.runic_circle=
	{
		Base="palette.entities.botobject",
		Name = "runic_circle",
		SheetClient="object_runic_circle.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.runic_circle", entries.runic_circle);
	entries.karavan_device=
	{
		Base="palette.entities.botobject",
		Name = "karavan_device",
		SheetClient="object_karavan_device.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_device", entries.karavan_device);
	entries.kitin_egg=
	{
		Base="palette.entities.botobject",
		Name = "kitin_egg",
		SheetClient="object_kitin_egg.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.kitin_egg", entries.kitin_egg);
	entries.bones_homin_a=
	{
		Base="palette.entities.botobject",
		Name = "bones_homin_a",
		SheetClient="object_bones_homin_a.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bones_homin_a", entries.bones_homin_a);
	entries.bones_homin_b=
	{
		Base="palette.entities.botobject",
		Name = "bones_homin_b",
		SheetClient="object_bones_homin_b.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.bones_homin_b", entries.bones_homin_b);
	entries.spot_kitin=
	{
		Base="palette.entities.botobject",
		Name = "spot_kitin",
		SheetClient="object_spot_kitin.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.spot_kitin", entries.spot_kitin);
	entries.spot_goo=
	{
		Base="palette.entities.botobject",
		Name = "spot_goo",
		SheetClient="object_spot_goo.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.spot_goo", entries.spot_goo);
	entries.merchant_melee_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_melee_fyros",
		SheetClient="object_merchant_melee_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_melee_fyros", entries.merchant_melee_fyros);
	entries.merchant_melee_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_melee_matis",
		SheetClient="object_merchant_melee_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_melee_matis", entries.merchant_melee_matis);
	entries.merchant_melee_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_melee_tryker",
		SheetClient="object_merchant_melee_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_melee_tryker", entries.merchant_melee_tryker);
	entries.merchant_melee_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_melee_zorai",
		SheetClient="object_merchant_melee_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_melee_zorai", entries.merchant_melee_zorai);
	entries.merchant_range_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_range_fyros",
		SheetClient="object_merchant_range_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_range_fyros", entries.merchant_range_fyros);
	entries.merchant_range_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_range_matis",
		SheetClient="object_merchant_range_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_range_matis", entries.merchant_range_matis);
	entries.merchant_range_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_range_tryker",
		SheetClient="object_merchant_range_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_range_tryker", entries.merchant_range_tryker);
	entries.merchant_range_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_range_zorai",
		SheetClient="object_merchant_range_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_range_zorai", entries.merchant_range_zorai);
	entries.merchant_armor_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_armor_fyros",
		SheetClient="object_merchant_armor_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_armor_fyros", entries.merchant_armor_fyros);
	entries.merchant_armor_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_armor_matis",
		SheetClient="object_merchant_armor_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_armor_matis", entries.merchant_armor_matis);
	entries.merchant_armor_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_armor_tryker",
		SheetClient="object_merchant_armor_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_armor_tryker", entries.merchant_armor_tryker);
	entries.merchant_armor_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_armor_zorai",
		SheetClient="object_merchant_armor_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_armor_zorai", entries.merchant_armor_zorai);
	entries.merchant_RM_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_RM_fyros",
		SheetClient="object_merchant_RM_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_RM_fyros", entries.merchant_RM_fyros);
	entries.merchant_RM_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_RM_matis",
		SheetClient="object_merchant_RM_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_RM_matis", entries.merchant_RM_matis);
	entries.merchant_RM_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_RM_tryker",
		SheetClient="object_merchant_RM_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_RM_tryker", entries.merchant_RM_tryker);
	entries.merchant_RM_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_RM_zorai",
		SheetClient="object_merchant_RM_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_RM_zorai", entries.merchant_RM_zorai);
	entries.merchant_tool_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tool_fyros",
		SheetClient="object_merchant_tool_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tool_fyros", entries.merchant_tool_fyros);
	entries.merchant_tool_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tool_matis",
		SheetClient="object_merchant_tool_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tool_matis", entries.merchant_tool_matis);
	entries.merchant_tool_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tool_tryker",
		SheetClient="object_merchant_tool_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tool_tryker", entries.merchant_tool_tryker);
	entries.merchant_tool_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tool_zorai",
		SheetClient="object_merchant_tool_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tool_zorai", entries.merchant_tool_zorai);
	entries.merchant_focus_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_focus_fyros",
		SheetClient="object_merchant_focus_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_focus_fyros", entries.merchant_focus_fyros);
	entries.merchant_focus_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_focus_matis",
		SheetClient="object_merchant_focus_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_focus_matis", entries.merchant_focus_matis);
	entries.merchant_focus_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_focus_tryker",
		SheetClient="object_merchant_focus_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_focus_tryker", entries.merchant_focus_tryker);
	entries.merchant_focus_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_focus_zorai",
		SheetClient="object_merchant_focus_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_focus_zorai", entries.merchant_focus_zorai);
	entries.merchant_haircut_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_haircut_fyros",
		SheetClient="object_merchant_haircut_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_haircut_fyros", entries.merchant_haircut_fyros);
	entries.merchant_haircut_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_haircut_matis",
		SheetClient="object_merchant_haircut_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_haircut_matis", entries.merchant_haircut_matis);
	entries.merchant_haircut_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_haircut_tryker",
		SheetClient="object_merchant_haircut_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_haircut_tryker", entries.merchant_haircut_tryker);
	entries.merchant_haircut_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_haircut_zorai",
		SheetClient="object_merchant_haircut_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_haircut_zorai", entries.merchant_haircut_zorai);
	entries.merchant_tatoo_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tatoo_fyros",
		SheetClient="object_merchant_tatoo_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tatoo_fyros", entries.merchant_tatoo_fyros);
	entries.merchant_tatoo_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tatoo_matis",
		SheetClient="object_merchant_tatoo_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tatoo_matis", entries.merchant_tatoo_matis);
	entries.merchant_tatoo_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tatoo_tryker",
		SheetClient="object_merchant_tatoo_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tatoo_tryker", entries.merchant_tatoo_tryker);
	entries.merchant_tatoo_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_tatoo_zorai",
		SheetClient="object_merchant_tatoo_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_tatoo_zorai", entries.merchant_tatoo_zorai);
	entries.merchant_bijoux_fyros=
	{
		Base="palette.entities.botobject",
		Name = "merchant_bijoux_fyros",
		SheetClient="object_merchant_bijoux_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_bijoux_fyros", entries.merchant_bijoux_fyros);
	entries.merchant_bijoux_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_bijoux_matis",
		SheetClient="object_merchant_bijoux_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_bijoux_matis", entries.merchant_bijoux_matis);
	entries.merchant_bijoux_tryker=
	{
		Base="palette.entities.botobject",
		Name = "merchant_bijoux_tryker",
		SheetClient="object_merchant_bijoux_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_bijoux_tryker", entries.merchant_bijoux_tryker);
	entries.merchant_bijoux_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_bijoux_zorai",
		SheetClient="object_merchant_bijoux_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_bijoux_zorai", entries.merchant_bijoux_zorai);
	entries.merchant_bar_matis=
	{
		Base="palette.entities.botobject",
		Name = "merchant_bar_matis",
		SheetClient="object_merchant_bar_matis.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_bar_matis", entries.merchant_bar_matis);
	entries.merchant_bar_zorai=
	{
		Base="palette.entities.botobject",
		Name = "merchant_bar_zorai",
		SheetClient="object_merchant_bar_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.merchant_bar_zorai", entries.merchant_bar_zorai);
	entries.roadsign=
	{
		Base="palette.entities.botobject",
		Name = "roadsign",
		SheetClient="object_roadsign.creature",
		Level=1,
	}
	--r2.addPaletteElement("palette.entities.botobjects.roadsign", entries.roadsign);
	entries.roadsign_fyros=
	{
		Base="palette.entities.botobject",
		Name = "roadsign_fyros",
		SheetClient="object_roadsign_fyros.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.roadsign_fyros", entries.roadsign_fyros);
	entries.roadsign_zorai=
	{
		Base="palette.entities.botobject",
		Name = "roadsign_zorai",
		SheetClient="object_roadsign_zorai.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.roadsign_zorai", entries.roadsign_zorai);
	entries.roadsign_tryker=
	{
		Base="palette.entities.botobject",
		Name = "roadsign_tryker",
		SheetClient="object_roadsign_tryker.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.roadsign_tryker", entries.roadsign_tryker);
	entries.banner_kami=
	{
		Base="palette.entities.botobject",
		Name = "banner_kami",
		SheetClient="object_banner_kami.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.banner_kami", entries.banner_kami);
	entries.banner_karavan=
	{
		Base="palette.entities.botobject",
		Name = "banner_karavan",
		SheetClient="object_banner_karavan.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.banner_karavan", entries.banner_karavan);
	entries.counter=
	{
		Base="palette.entities.botobject",
		Name = "counter",
		SheetClient="object_counter.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.counter", entries.counter);
	entries.homin_body_fyros_H=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_fyros_H",
		SheetClient="object_homin_body_fyros_H.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_fyros_H", entries.homin_body_fyros_H);
	entries.homin_body_fyros_F=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_fyros_F",
		SheetClient="object_homin_body_fyros_F.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_fyros_F", entries.homin_body_fyros_F);
	entries.homin_body_matis_H=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_matis_H",
		SheetClient="object_homin_body_matis_H.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_matis_H", entries.homin_body_matis_H);
	entries.homin_body_matis_F=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_matis_F",
		SheetClient="object_homin_body_matis_F.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_matis_F", entries.homin_body_matis_F);
	entries.homin_body_tryker_H=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_tryker_H",
		SheetClient="object_homin_body_tryker_H.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_tryker_H", entries.homin_body_tryker_H);
	entries.homin_body_tryker_F=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_tryker_F",
		SheetClient="object_homin_body_tryker_F.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_tryker_F", entries.homin_body_tryker_F);
	entries.homin_body_zorai_H=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_zorai_H",
		SheetClient="object_homin_body_zorai_H.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_zorai_H", entries.homin_body_zorai_H);
	entries.homin_body_zorai_F=
	{
		Base="palette.entities.botobject",
		Name = "homin_body_zorai_F",
		SheetClient="object_homin_body_zorai_F.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.homin_body_zorai_F", entries.homin_body_zorai_F);
	entries.karavan_standard=
	{
		Base="palette.entities.botobject",
		Name = "karavan_standard",
		SheetClient="object_karavan_standard.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_standard", entries.karavan_standard);
	entries.karavan_tent=
	{
		Base="palette.entities.botobject",
		Name = "karavan_tent",
		SheetClient="object_karavan_tent.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_tent", entries.karavan_tent);
	entries.karavan_big_wall=
	{
		Base="palette.entities.botobject",
		Name = "karavan_big_wall",
		SheetClient="object_karavan_big_wall.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_big_wall", entries.karavan_big_wall);
	entries.karavan_wall=
	{
		Base="palette.entities.botobject",
		Name = "karavan_wall",
		SheetClient="object_karavan_wall.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_wall", entries.karavan_wall);
	entries.karavan_mirador=
	{
		Base="palette.entities.botobject",
		Name = "karavan_mirador",
		SheetClient="object_karavan_mirador.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_mirador", entries.karavan_mirador);
	entries.karavan_gateway=
	{
		Base="palette.entities.botobject",
		Name = "karavan_gateway",
		SheetClient="object_karavan_gateway.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.karavan_gateway", entries.karavan_gateway);
	entries.kami_standard=
	{
		Base="palette.entities.botobject",
		Name = "kami_standard",
		SheetClient="object_kami_standard.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.kami_standard", entries.kami_standard);
	entries.kami_hut=
	{
		Base="palette.entities.botobject",
		Name = "kami_hut",
		SheetClient="object_kami_hut.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.kami_hut", entries.kami_hut);
	entries.vegetable_wall=
	{
		Base="palette.entities.botobject",
		Name = "vegetable_wall",
		SheetClient="object_vegetable_wall.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.vegetable_wall", entries.vegetable_wall);
	entries.kami_watchtower=
	{
		Base="palette.entities.botobject",
		Name = "kami_watchtower",
		SheetClient="object_kami_watchtower.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.kami_watchtower", entries.kami_watchtower);
	entries.vegetable_gateway=
	{
		Base="palette.entities.botobject",
		Name = "vegetable_gateway",
		SheetClient="object_vegetable_gateway.creature",
		Level=1,
	}
	r2.addPaletteElement("palette.entities.botobjects.vegetable_gateway", entries.vegetable_gateway);

	entries.creature_cbadc1 = { Base="palette.entities.creatures.passive", SheetClient="cbadc1.creature", Level=60, Ecosystem="Desert", Region="c" } r2.addPaletteElement("palette.entities.creatures.cbadc1", entries.creature_cbadc1)
	entries.creature_cbagf3 = { Base="palette.entities.creatures.passive", SheetClient="cbagf3.creature", Level=230, Ecosystem="Goo", Region="f" } r2.addPaletteElement("palette.entities.creatures.cbagf3", entries.creature_cbagf3)
	entries.creature_chafd3 = { Base="palette.entities.creatures.passive", SheetClient="chafd3.creature", Level=130, Ecosystem="Forest", Region="d" } r2.addPaletteElement("palette.entities.creatures.chafd3", entries.creature_chafd3)

	entries.creature_cpagb1 = { Base="palette.entities.creatures.plant", SheetClient="cpagb1.creature", Level=10, Ecosystem="Goo", Region="b" } r2.addPaletteElement("palette.entities.creatures.cpagb1", entries.creature_cpagb1)
	entries.creature_cpbgb1 = { Base="palette.entities.creatures.plant", SheetClient="cpbgb1.creature", Level=10, Ecosystem="Goo", Region="b" } r2.addPaletteElement("palette.entities.creatures.cpbgb1", entries.creature_cpbgb1)
end

--debugInfo(colorTag(0, 255, 0) .. "Building palette table")
-- palette hierarchy for the UI
r2.Palette = 
{
	UIPath="ui:interface:r2ed_palette:content:sbtree_entities:entity_enclosing:tree_list",
	StrId ="uiR2EDentities",
	Entries = 
	{		
		npc = 
		{		
			instances =
			{
				{Id="palette.entities.players.fyros_male", Translation="uiR2EDnpc_fyros_h"},
				{Id="palette.entities.players.fyros_female", Translation="uiR2EDnpc_fyros_f"},
				{Id="palette.entities.players.matis_male", Translation="uiR2EDnpc_matis_h"},
				{Id="palette.entities.players.matis_female", Translation="uiR2EDnpc_matis_f"},
				{Id="palette.entities.players.tryker_male", Translation="uiR2EDnpc_tryker_h"},
				{Id="palette.entities.players.tryker_female", Translation="uiR2EDnpc_tryker_f"},
				{Id="palette.entities.players.zorai_male", Translation="uiR2EDnpc_zorai_h"},
				{Id="palette.entities.players.zorai_female", Translation="uiR2EDnpc_zorai_f"},				
			}
		},
		botObjects = 
		{
			instances =
			{
				{Id="palette.entities.botobjects.jar", Translation="uiR2EDbotObjJar"},
				{Id="palette.entities.botobjects.jar_3", Translation="uiR2EDbotObjJar3"},
				{Id="palette.entities.botobjects.jar_fallen", Translation="uiR2EDbotObjJarFallen"},
				{Id="palette.entities.botobjects.chest", Translation="uiR2EDbotObjChest"},
				{Id="palette.entities.botobjects.chest_old", Translation="uiR2EDbotObjChestOld"},
				{Id="palette.entities.botobjects.chariot", Translation="uiR2EDbotObjChariot"},
				{Id="palette.entities.botobjects.chariot_working", Translation="uiR2EDbotObjChariotWorking"},
				{Id="palette.entities.botobjects.campfire", Translation="uiR2EDbotObjCampFire"},
				{Id="palette.entities.botobjects.campfire_out", Translation="uiR2EDbotObjCampfireOut"},
			}
		},
		creatures_passive =
		{
			instances =
			{
				{Id="palette.entities.creatures.cpagb1", Translation="uiR2EDcreature_cpagb1"},
				{Id="palette.entities.creatures.cpbgb1", Translation="uiR2EDcreature_cpbgb1"},
			}
		},
		creatures_predators =
		{
			instances =
			{
				{Id="palette.entities.creatures.cbadc1", Translation="uiR2EDcreature_cbadc1"},
				{Id="palette.entities.creatures.cbagf3", Translation="uiR2EDcreature_cbagf3"},
				{Id="palette.entities.creatures.chafd3", Translation="uiR2EDcreature_chafd3"},
			}
		},
	}
}


r2.PaletteIdToTranslation = {} -- maps each palette id to its translation, filled at build time by r2_ui_palette.lua
							   -- this table may be access by C++ to give a name to newinstances created by user
r2.PaletteIdToGroupTranslation = {} -- the same thing for group of NPC
r2.PaletteIdToType = {}
-- add test entries for levels

if r2.Config.TestPaletteSelection then
	-- TMP for nico test
	r2.Palette.Entries.selection_test = 
	{
		instances = {}
	}
	for ecosystem = 1, 4 do
		for level = 1, 250, levelStep do
			table.insert(r2.Palette.Entries.selection_test.instances, { Id=formatSelectionTextName(level, ecosystem), DirectName="fyros level " .. formatLevel(level) .. ", ecosystem = " ..  ecosystemTable[ecosystem]})
		end
	end
end

--runCommand("luaObject","r2.Palette")
--debugInfo(colorTag(0, 255, 0) .. "Palette table built")

