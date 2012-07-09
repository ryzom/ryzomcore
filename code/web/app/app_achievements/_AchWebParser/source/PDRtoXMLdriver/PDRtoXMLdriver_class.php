<?php
	class PDRtoXMLdriver extends SourceDriver {
		private $conf;
		private $ignore;
		private $ignore_block;
		private $lock;

		function PDRtoXMLdriver() {
			require_once("conf.php");
			
			$this->conf = $_CONF;


			$this->lock = 0;

			$this->ignore = array();
			$this->ignore[] = "XML";
			$this->ignore[] = "ENTITYBASE";
			$this->ignore[] = "NORMALPOSITIONS";
			$this->ignore[] = "_VEC";
			$this->ignore[] = "SESSIONID";
			$this->ignore[] = "POSSTATE";
			$this->ignore[] = "_PLAYERROOM";
			$this->ignore[] = "_INVENTORYID";
			$this->ignore[] = "_PHYSCHARACS";
			$this->ignore[] = "_PHYSSCORES";
			$this->ignore[] = "_SKILLS";
			$this->ignore[] = "_FAMES";

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
			global $CONF;

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

			if($attrs["VALUE"] != "") {
				echo "dispatching";
				$_DISPATCHER->dispatchValue(strtolower($name),$attrs["VALUE"]);	
			}
		}

		function endElement($parser, $name) {
			if(in_array($name,$this->ignore_block)) {
				$this->lock = 0;
			}
		}

		
	}
?>