<?php
	$BASE_PATH = dirname(__FILE__);

	require_once($BASE_PATH."/entity/FactionPoints_entity.php");
	require_once($BASE_PATH."/entity/Fame_entity.php");
	require_once($BASE_PATH."/entity/Item_entity.php");
	require_once($BASE_PATH."/entity/LastLogStats_entity.php");
	require_once($BASE_PATH."/entity/Mission_entity.php");
	require_once($BASE_PATH."/entity/PermanentMod_entity.php");
	require_once($BASE_PATH."/entity/Pet_entity.php");
	require_once($BASE_PATH."/entity/PhysCharacs_entity.php");
	require_once($BASE_PATH."/entity/PhysScores_entity.php");
	require_once($BASE_PATH."/entity/SkillPoints_entity.php");
	require_once($BASE_PATH."/entity/Skills_entity.php");
	require_once($BASE_PATH."/entity/SpentSkillPoints_entity.php");
	require_once($BASE_PATH."/entity/Position_entity.php");
	require_once($BASE_PATH."/entity/Gear_entity.php");
	require_once($BASE_PATH."/entity/SkillList_entity.php");
	require_once($BASE_PATH."/entity/MissionList_entity.php");

	class PDRtoXMLdriver extends SourceDriver {
		private $conf;
		private $ignore;
		private $ignore_block;
		private $lock;
		private $open;
		private $entity;
		private $inv;
		private $iblock;
		private $gear;
		private $skills;

		function PDRtoXMLdriver() {
			require_once("conf.php");
			
			$this->conf = $_CONF;


			$this->lock = 0;
			$this->open = null;
			$this->entity = null;
			$this->inv = null;
			$this->iblock = false;

			$this->gear = new Gear();
			$this->skills = new SkillList();
			$this->mission = new MissionList();
			
			//these nodes are ignored, but children are processed
			$this->ignore = array();
			$this->ignore[] = "XML";
			$this->ignore[] = "ENTITYBASE";
			$this->ignore[] = "NORMALPOSITIONS";
			$this->ignore[] = "_VEC";
			$this->ignore[] = "SESSIONID";
			#$this->ignore[] = "POSSTATE";
			$this->ignore[] = "_PLAYERROOM";
			$this->ignore[] = "_INVENTORYID";
			$this->ignore[] = "_PHYSCHARACS";
			$this->ignore[] = "_PHYSSCORES";
			$this->ignore[] = "_SKILLS";
			$this->ignore[] = "_FAMES";
			
			//these nodes are ignored, as well as their children
			$this->ignore_block = array();
			$this->ignore_block[] = "_MEMORIZEDPHRASES";
			$this->ignore_block[] = "_FORBIDPOWERDATES";
			$this->ignore_block[] = "_INEFFECTIVEAURAS";
			$this->ignore_block[] = "_CONSUMABLEOVERDOSEENDDATES";
			$this->ignore_block[] = "_MODIFIERSINDB";
			$this->ignore_block[] = "_MISSIONS";
			$this->ignore_block[] = "_ITEMSINSHOPSTORE";
			$this->ignore_block[] = "RINGREWARDPOINTS";
			$this->ignore_block[] = "_PACT";
			$this->ignore_block[] = "_KNOWNPHRASES";
			$this->ignore_block[] = "STARTINGCHARACTERISTICVALUES";
			$this->ignore_block[] = "_ENCYCLOCHAR";
			$this->ignore_block[] = "_GAMEEVENT";
			$this->ignore_block[] = "_ENTITYPOSITION";
		}

		function drive($cid) {
			global $CONF,$_DISPATCHER;

			echo "kk";

			#$uid = floor($cid/16);
			#$slot = ($cid%16);

			#$file = $this->conf['xml_dir']."account_".$uid."_".$slot."_pdr.xml";
			$file = $_REQUEST['file'];

			$xml_parser = xml_parser_create();
			xml_set_object($xml_parser,$this);
			xml_set_element_handler($xml_parser, "startElement", "endElement");

			if(!xml_parse($xml_parser, file_get_contents($file))) {
				#error
				echo "error";
			}
			xml_parser_free($xml_parser);

			$_DISPATCHER->dispatchEntity($this->gear->getName(),$this->gear);
			$_DISPATCHER->dispatchEntity($this->skills->getName(),$this->skills);
			$_DISPATCHER->dispatchEntity($this->skills->mission(),$this->mission);
		}

		function startElement($parser, $name, $attrs) {
			global $_DISPATCHER;

			if($this->lock == 1) {
				return null;
			}
			
			if(in_array($name,$this->ignore)) {
				return null;
			}

			if(in_array($name,$this->ignore_block)) {
				$this->lock = 1;
				return null;
			}
			
			/* faction points */
			if($name == "FACTIONPOINTS") {
				$this->open = "FACTIONPOINTS";
				return null;
			}

			if($this->open == "FACTIONPOINTS") {
				if($name == "__KEY__") {
					$this->entity = new FactionPoints();
					$this->entity->faction = $attrs["VALUE"];
					return null;
				}

				if($name == "__VAL__") {
					$this->entity->value = $attrs["VALUE"];
					$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
					$this->entity = null;
					return null;
				}

				return null;
			}

			/* Fame */
			if($name == "_FAME") {
				$this->open = "_FAME";
				return null;
			}

			if($this->open == "_FAME") {
				if($name == "__KEY__") {
					$this->entity = new Fame();
					$this->entity->faction = $attrs["VALUE"];
					return null;
				}

				if($name == "FAME") {
					$this->entity->fame = $attrs["VALUE"];
					return null;
				}
				if($name == "FAMEMEMORY") {
					$this->entity->famememory = $attrs["VALUE"];
					return null;
				}
				if($name == "LASTFAMECHANGETREND") {
					$this->entity->lastfamechangetrend = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* last log stats */
			if($name == "_LASTLOGSTATS") {
				$this->open = "_LASTLOGSTATS";
				$this->entity = new LastLogStats();
				return null;
			}

			if($this->open == "_LASTLOGSTATS") {
				if($name == "LOGINTIME") {
					$this->entity->logintime = $attrs["VALUE"];
					return null;
				}
				if($name == "DURATION") {
					$this->entity->duration = $attrs["VALUE"];
					return null;
				}
				if($name == "LOGOFFTIME") {
					$this->entity->logofftime = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* mission */
			if($name == "_MISSIONHISTORIES") {
				$this->open = "_MISSIONHISTORIES";
				return null;
			}

			if($this->open == "_MISSIONHISTORIES") {
				if($name == "__KEY__") {
					$this->entity = new Mission();
					$this->entity->mission = $attrs["VALUE"];
					return null;
				}

				if($name == "SUCCESSFULL") {
					$this->entity->successfull = $attrs["VALUE"];
					return null;
				}
				if($name == "UTC_LASTSUCCESSDATE") {
					$this->entity->utc_lastsuccessdate = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* permanent mod */
			if($name == "SCOREPERMANENTMODIFIERS") {
				$this->open = "SCOREPERMANENTMODIFIERS";
				return null;
			}

			if($this->open == "SCOREPERMANENTMODIFIERS") {
				if($name == "__KEY__") {
					$this->entity = new PermanentMod();
					$this->entity->score = $attrs["VALUE"];
					return null;
				}

				if($name == "__VAL__") {
					$this->entity->value = $attrs["VALUE"];
					$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
					$this->entity = null;
					return null;
				}

				return null;
			}

			/* pet */
			if($name == "_PLAYERPETS") {
				$this->open = "_PLAYERPETS";
				return null;
			}

			if($this->open == "_PLAYERPETS") {
				if($name == "__KEY__") {
					$this->entity = new Pet();
					$this->entity->pet = $attrs["VALUE"];
					return null;
				}

				if($name == "TICKETPETSHEETID") {
					$this->entity->ticketpetsheetid = $attrs["VALUE"];
					return null;
				}
				if($name == "PETSHEETID") {
					$this->entity->petsheetid = $attrs["VALUE"];
					return null;
				}
				if($name == "PRICE") {
					$this->entity->price = $attrs["VALUE"];
					return null;
				}
				if($name == "OWNERID") {
					$this->entity->ownerid = $attrs["VALUE"];
					return null;
				}
				if($name == "STABLEALIAS") {
					$this->entity->stablealias = $attrs["VALUE"];
					return null;
				}
				if($name == "LANDSCAPE_X") {
					$this->entity->landscape_x = $attrs["VALUE"];
					return null;
				}
				if($name == "LANDSCAPE_Y") {
					$this->entity->landscape_y = $attrs["VALUE"];
					return null;
				}
				if($name == "LANDSCAPE_Z") {
					$this->entity->landscape_z = $attrs["VALUE"];
					return null;
				}
				if($name == "UTC_DEATHTICK") {
					$this->entity->utc_deathtick = $attrs["VALUE"];
					return null;
				}
				if($name == "PETSTATUS") {
					$this->entity->petstatus = $attrs["VALUE"];
					return null;
				}
				if($name == "SLOT") {
					$this->entity->slot = $attrs["VALUE"];
					return null;
				}
				if($name == "ISTPALLOWED") {
					$this->entity->istpallowed = $attrs["VALUE"];
					return null;
				}
				if($name == "SATIETY") {
					$this->entity->satiety = $attrs["VALUE"];
					return null;
				}
				if($name == "CUSTOMNAME") {
					$this->entity->customname = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* physical characteristics */
			if($name == "_PHYSICALCHARACTERISTICS") {
				$this->open = "_PHYSICALCHARACTERISTICS";
				return null;
			}

			if($this->open == "_PHYSICALCHARACTERISTICS") {
				if($name == "__KEY__") {
					$this->entity = new PhysCharacs();
					$this->entity->charac = $attrs["VALUE"];
					return null;
				}

				if($name == "__VAL__") {
					$this->entity->value = $attrs["VALUE"];
					$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
					$this->entity = null;
					return null;
				}

				return null;
			}

			/* physical scores */
			if($name == "PHYSICALSCORES") {
				$this->open = "PHYSICALSCORES";
				return null;
			}

			if($this->open == "PHYSICALSCORES") {
				if($name == "__KEY__") {
					$this->entity = new PhysScores();
					$this->entity->score = $attrs["VALUE"];
					return null;
				}

				if($name == "CURRENT") {
					$this->entity->current = $attrs["VALUE"];
					return null;
				}
				if($name == "BASE") {
					$this->entity->base = $attrs["VALUE"];
					return null;
				}
				if($name == "MAX") {
					$this->entity->max = $attrs["VALUE"];
					return null;
				}
				if($name == "BASEREGENERATEREPOS") {
					$this->entity->baseregeneraterepos = $attrs["VALUE"];
					return null;
				}
				if($name == "BASEREGENERATEACTION") {
					$this->entity->baseregenerateaction = $attrs["VALUE"];
					return null;
				}
				if($name == "CURRENTREGENERATE") {
					$this->entity->currentregenerate = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* skill points */
			if($name == "SKILLPOINTS") {
				$this->open = "SKILLPOINTS";
				return null;
			}

			if($this->open == "SKILLPOINTS") {
				if($name == "__KEY__") {
					$this->entity = new SkillPoints();
					$this->entity->skill = $attrs["VALUE"];
					return null;
				}

				if($name == "__VAL__") {
					$this->entity->value = $attrs["VALUE"];
					$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
					$this->entity = null;
					return null;
				}

				return null;
			}

			/* spent skill points */
			if($name == "SPENTSKILLPOINTS") {
				$this->open = "SPENTSKILLPOINTS";
				return null;
			}

			if($this->open == "SPENTSKILLPOINTS") {
				if($name == "__KEY__") {
					$this->entity = new SpentSkillPoints();
					$this->entity->skill = $attrs["VALUE"];
					return null;
				}

				if($name == "__VAL__") {
					$this->entity->value = $attrs["VALUE"];
					$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
					$this->entity = null;
					return null;
				}

				return null;
			}

			/* skills */
			if($name == "SKILLS") {
				$this->open = "SKILLS";
				return null;
			}

			if($this->open == "SKILLS") {
				if($name == "__KEY__") {
					$this->entity = new Skills();
					$this->entity->skill = $attrs["VALUE"];
					return null;
				}

				if($name == "BASE") {
					$this->entity->base = $attrs["VALUE"];
					return null;
				}
				if($name == "CURRENT") {
					$this->entity->current = $attrs["VALUE"];
					return null;
				}
				if($name == "MAXLVLREACHED") {
					$this->entity->maxlvlreached = $attrs["VALUE"];
					return null;
				}
				if($name == "XP") {
					$this->entity->xp = $attrs["VALUE"];
					return null;
				}
				if($name == "XPNEXTLVL") {
					$this->entity->xpnextlvl = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* Position */
			if($name == "POSSTATE") {
				$this->open = "POSSTATE";
				$this->entity = new Position();
				return null;
			}

			if($this->open == "POSSTATE") {
				if($name == "X") {
					$this->entity->x = $attrs["VALUE"];
					return null;
				}
				if($name == "Y") {
					$this->entity->y = $attrs["VALUE"];
					return null;
				}
				if($name == "Z") {
					$this->entity->z = $attrs["VALUE"];
					return null;
				}
				if($name == "HEADING") {
					$this->entity->heading = $attrs["VALUE"];
					return null;
				}

				return null;
			}

			/* items */
			
			if($name == "ROOMINVENTORY") {
				$this->inv = "room";
				return null;
			}

			if($name == "INVENTORY") {
				$this->iblock = true;
				return null;
			}

			if($this->iblock == true) {
				if($name == "__KEY__") {
					$this->inv = $attrs["VALUE"];
				}
				if($name == "__VAL__") {
					return null;
				}
			}

			if($name == "_ITEMS" || $name == "_ITEM") {
				$this->open = "_ITEM";
				$this->entity = new Item();
				$this->entity->inventory = $this->inv;
				return null;
			}

			if($this->open == "_ITEM") {
				if($name == "_CRAFTPARAMETERS") {
					$this->icraft = true;
					return null;
				}
				
				if($this->icraft == true) {
					$this->entity->_craftparameters[strtolower($name)] = $attrs["VALUE"];
					return null;
				}

				if($name == "_ITEMID") {
					$this->entity->_itemid = $attrs["VALUE"];
					return null;
				}
				if($name == "_SHEETID") {
					$this->entity->_sheetid = $attrs["VALUE"];
					return null;
				}
				if($name == "_LOCSLOT") {
					$this->entity->_locslot = $attrs["VALUE"];
					return null;
				}
				if($name == "_HP") {
					$this->entity->_hp = $attrs["VALUE"];
					return null;
				}
				if($name == "_RECOMMENDED") {
					$this->entity->_recommended = $attrs["VALUE"];
					return null;
				}
				if($name == "_CREATORID") {
					$this->entity->_creatorid = $attrs["VALUE"];
					return null;
				}
				if($name == "_PHRASEID") {
					$this->entity->_phraseid = $attrs["VALUE"];
					return null;
				}
				if($name == "_REFINVENTORYSLOT") {
					$this->entity->_refinventoryslot = $attrs["VALUE"];
					return null;
				}
				if($name == "REFINVENTORYID") {
					$this->entity->refinventoryid = $attrs["VALUE"];
					return null;
				}
				if($name == "_USENEWSYSTEMREQUIREMENT") {
					$this->entity->_usenewsystemrequirement = $attrs["VALUE"];
					return null;
				}
				if($name == "_REQUIREDSKILLLEVEL") {
					$this->entity->_requiredskilllevel = $attrs["VALUE"];
					return null;
				}
				if($name == "_CUSTOMTEXT") {
					$this->entity->_customtext = $attrs["VALUE"];
					return null;
				}
				if($name == "_LOCKEDBYOWNER") {
					$this->entity->_lockedbyowner = $attrs["VALUE"];
					return null;
				}
				if($name == "_DROPABLE") {
					$this->entity->_dropable = $attrs["VALUE"];
					return null;
				}
				if($name == "STACKSIZE") {
					$this->entity->stacksize = $attrs["VALUE"];
					return null;
				}
			}





			if($attrs["VALUE"] != "") {
				$_DISPATCHER->dispatchValue(strtolower($name),$attrs["VALUE"]);	
			}
		}

		function endElement($parser, $name) {
			global $_DISPATCHER;

			if(in_array($name,$this->ignore_block)) {
				$this->lock = 0;
			}
			
			/* faction points */
			if($name == "FACTIONPOINTS") {
				$this->open = null;
				return null;
			}

			/* fame */
			if($name == "__VAL__" && $this->open == "FAME") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				$this->entity = null;
				return null;
			}

			if($name == "_FAME") {
				$this->open = null;
				return null;
			}

			/* last log stats */
			if($name == "_LASTLOGSTATS") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				$this->entity = null;
				return null;
			}

			/* mission */
			if($name == "__VAL__" && $this->open == "_MISSIONHISTORIES") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				$this->mission->missions[] = $this->entity;
				$this->entity = null;
				return null;
			}

			if($name == "_MISSIONHISTORIES") {
				$this->open = null;
				return null;
			}

			/* permanent mod */
			if($name == "SCOREPERMANENTMODIFIERS") {
				$this->open = null;
				return null;
			}

			/* pet */
			if($name == "__VAL__" && $this->open == "_PLAYERPETS") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				#echo "dispatched";
				$this->entity = null;
				return null;
			}

			if($name == "_PLAYERPETS") {
				$this->open = null;
				return null;
			}

			/* physical characteristics */
			if($name == "_PHYSICALCHARACTERISTICS") {
				$this->open = null;
				return null;
			}

			/* physical scores */
			if($name == "__VAL__" && $this->open == "PHYSICALSCORES") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				$this->entity = null;
				return null;
			}

			if($name == "PHYSICALSCORES") {
				$this->open = null;
				return null;
			}

			/* skill points */
			if($name == "SKILLPOINTS") {
				$this->open = null;
				return null;
			}

			/* spent skill points */
			if($name == "SPENTSKILLPOINTS") {
				$this->open = null;
				return null;
			}

			/* skills */
			if($name == "__VAL__" && $this->open == "SKILLS") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				$this->skills->skills[] = $this->entity;
				$this->entity = null;
				return null;
			}

			if($name == "SKILLS") {
				$this->open = null;
				return null;
			}

			/* position */
			if($name == "POSSTATE") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				$this->entity = null;
				return null;
			}

			/* items */
			if($name == "_ITEMS") {
				$_DISPATCHER->dispatchEntity($this->entity->getName(),$this->entity);
				if($this->entity->refinventoryid != null) {
					$this->gear->items[] = $this->entity;
				}
				$this->entity = null;
				return null;
			}

			if($name == "INVENTORY") {
				$this->iblock = false;
				return null;
			}

			if($name == "_ITEM" || $name == "_ITEMS") {
				$this->open = null;
				return null;
			}

			
		}

		
	}
?>