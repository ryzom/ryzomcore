<?php
	class XMLapi extends DataSource {
		private $xml_path;

		function XMLapi() {
			parent::__construct();

			$this->xml_path = $CONF['xml_path'];
		}

		function loadData($ident,$type) {
			switch($type) {
				case 'c_stats':
				case 'c_connection':
				case 'c_gear':
				case 'c_fp':
				case 'c_skills':
				case 'c_fame':
				case 'c_pet':
					$path = $this->xml_path."full/".$ident.".xml";
					break;
				case 'c_items':
					$path = $this->xml_path."item/".$ident.".xml";
					break;
				default:
					return false;
					break;
			}

			$xml = new SimpleXMLElement(file_get_contents($path));

			switch($type) {
				case 'c_stats':
					$this->loadStats($xml);
					break;
				case 'c_connection':
					$this->loadConnection($xml);
					break;
				case 'c_gear':
					$this->loadGear($xml);
					break;
				case 'c_fp':
					$this->loadFp($xml);
					break;
				case 'c_skills':
					$this->loadSkills($xml);
					break;
				case 'c_fame':
					$this->loadFame($xml);
					break;
				case 'c_pet':
					$this->loadPet($xml);
					break;
				case 'c_items':
					$this->loadItems($xml);
					break;
				default:
					return false;
					break;
			}
		}

		private function getItems(&$xml) {
			$dta = array();

			$r = $xml->xpath('item');
			$dta[] = $r->attributes();

			return $dta;
		}

		private function loadGear(&$xml) {
			$this->data['c_gear'] = $this->getItems($xml);
		}

		private function loadFp(&$xml) {
			$dta = array();

			$r = $xml->xpath('faction_points');
			foreach($r->children() as $elem) {
				$dta[] = array($elem->getName()=>$elem);
			}

			$this->data['c_fp'] = $dta;
		}

		private function loadSkills(&$xml) {
			$dta = array();

			$r = $xml->xpath('skills');
			foreach($r->children() as $elem) {
				$dta[] = array($elem->getName()=>$elem);
			}

			$this->data['c_skills'] = $dta;
		}

		private function loadPet(&$xml) {
			$dta = array();

			$r = $xml->xpath('pet');
			foreach($r as $pet) {
				$tmp = $pet->attributes();
				$child = $pet->children();
				$tmp = array_merge($tmp,$child[0]->attributes());
				
				$dta[] = $tmp;
			}

			$this->data['c_pet'] = $dta;
		}

		private function loadFame(&$xml) {
			$dta = array();

			$r = $xml->xpath('fames');
			foreach($r->children() as $elem) {
				$dta[] = array($elem->getName()=>$elem);
			}

			$this->data['c_fame'] = $dta;
		}

		private function loadItems(&$xml) {
			$this->data['c_items'] = $this->getItems($xml);
		}

		private function loadConnection(&$xml) {
			$dta = array();

			$r = $xml->xpath('log');

			while(list( , $node) = each($r)) {
				#$attr = $node->attributes();
				#$dta[] = array("in"=>$attr[],"out"=>$attr[1],"duration"=>$attr[2]);
				$dta[] = $node->attributes();
			}

			$this->data['c_connection'] = $dta;
		}

		private function loadStats(&$xml) {
			$dta = array();

			$slist = array(	'name',
							'shard',
							'uid',
							'slot',
							'cid',
							'race',
							'gender',
							'titleid',
							'played_time',
							'latest_login',
							'latest_logout',
							'hair_type',
							'hair_color',
							'tattoo',
							'eyes_color',
							'gabarit_height',
							'gabarit_torso_width',
							'gabarit_arms_width',
							'gabarit_legs_width',
							'gabarit_breast_size',
							'morph1',
							'morph2',
							'morph3',
							'morph4',
							'morph5',
							'morph6',
							'morph7',
							'morph8',
							'gid',
							'name',
							'icon',
							'money',
							'cult',
							'civ',
							'constitution',
							'metabolism',
							'intelligence',
							'wisdom',
							'strength',
							'wellbalanced',
							'dexterity',
							'will',
							'building');

			foreach($slist as $elem) {
				$r = $xml->xpath($elem);
				$dta[$elem] = $r[0];
			}

			#<position x="17027.559" y="-32943.011" head="-1.981871"/>
			$r = $xml->xpath("position");
			$attr = $r->attributes();
			$dta['pos_x'] = $attr['x'];
			$dta['pos_y'] = $attr['y'];
			$dta['pos_head'] = $attr['head'];
			
			#<hitpoints max="3941">3941</hitpoints>
			#<stamina max="1790">1790</stamina>
			#<sap max="1890">1890</sap>
			#<focus max="2750">2750</focus>
			$r = $xml->xpath("hitpoints");
			$attr = $r->attributes();
			$dta['hitpoints'] = $attr['hitpoints'];

			$r = $xml->xpath("stamina");
			$attr = $r->attributes();
			$dta['stamina'] = $attr['stamina'];

			$r = $xml->xpath("sap");
			$attr = $r->attributes();
			$dta['sap'] = $attr['sap'];

			$r = $xml->xpath("focus");
			$attr = $r->attributes();
			$dta['focus'] = $attr['focus'];

			$this->data['c_stats'] = array($dta);
		}

		function writeData($ident,$field,$data,$type) {
			return false;
		}

	}
?>