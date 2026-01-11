<?php
	class AdmAtom extends Node implements ADM {
		#########################
		# PHP 5.3 compatible
		# AdmDispatcher_trait replaces this in PHP 5.4

		function insertNode($n) {
			$n->setParent($this);
			$n->insert();
			$this->addChild($n);
		}

		function removeNode($id) {
			$res = $this->getChildDataByID($id);
			if($res != null) {
				$res->delete_me();
				$this->removeChild($id);
			}
		}

		function updateNode($id) { // PROBABLY USELESS!
			$res = $this->getChildDataByID($id);
			if($res != null) {
				$res->update();
			}
		}

		function getPathID($path = "") {
			if($path != "") {
				$path = ";".$path;
			}
			$path = $this->getID().$path;
			if($this->parent != null) {
				return $this->parent->getPathID($path);
			}

			return $path;
		}

		function getElementByPath($pid) {
			$tmp = explode(";",$pid);
			if($tmp[0] == $this->getID()) {
				if(sizeof($tmp) > 1) {
					$c = $this->getChildDataByID($tmp[1]);
					if($c != null) {
						unset($tmp[0]);
						return $c->getElementByPath(implode(";",$tmp));
					}
					return null;
				}
				else {
					return $this;
				}
			}
			return null;
		}
		#########################

		protected $objective;
		protected $mandatory;
		protected $ruleset;
		protected $ruleset_parsed;
		protected $parent_id;
		
		function AdmAtom($data,$parent) {
			$this->parent = $parent;
			$this->id = $data['atom_id'];
			$this->objective = $data['atom_objective'];
			$this->mandatory = $data['atom_mandatory'];
			$this->ruleset = $data['atom_ruleset'];
			$this->ruleset_parsed = $data['atom_ruleset_parsed'];
		}

		function delete_me() { // aaaaand... it's gone ^^
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_atom WHERE atom_id='".$this->id."'");
			$DBc->sqlQuery("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."'");
		}

		function update() { // write updated data to database
			global $DBc;
			
			$DBc->sqlQuery("UPDATE ach_atom SET atom_mandatory='".$this->getMandatory()."',atom_ruleset='".$DBc->sqlEscape($this->getRuleset())."',atom_ruleset_parsed='".$DBc->sqlEscape($this->getRulesetParsed())."' WHERE atom_id='".$this->id."'");
		}

		function insert() { // insert atoms as new row
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_atom (atom_objective,atom_mandatory,atom_ruleset,atom_ruleset_parsed) VALUES ('".$this->getObjective()."','".$this->getMandatory()."','".$DBc->sqlEscape($this->getRuleset())."','".$DBc->sqlEscape($this->getRulesetParsed())."')");
			$id = $DBc->insertID();
			$this->setID($id);
		}

		function getObjective() {
			return $this->objective;
		}

		function setObjective($o) {
			$this->objective = $o;
		}

		function setMandatory($ft) {
			if($ft == true) {
				$this->mandatory = 1;
			}
			else {
				$this->mandatory = 0;
			}
		}

		function setRuleset($r) {
			$this->ruleset = $r;
			$this->parse();
		}

		function getMandatory() {
			return $this->mandatory;
		}

		function isMandatory() {
			return ($this->mandatory == 1);
		}

		function getRuleset() {
			return $this->ruleset;
		}

		function getRulesetParsed() {
			return $this->ruleset_parsed;
		}

		private function parse() { // parsing the ruleset
			/*
VALUE _money AS $money {
	
	CACHE blach AS $test;

	if($money >= 10000 && $test == 0) {
		RESET;
		GRANT $money UNTIL TIMER:3600;
		FINAL;
	}
	else {
		CACHE blach SET $money;
	}

	SCRIPT wealth($money) AS $res;

	if($res == "lol") {
		DENY;
	}
}

ENTITY _pos AS $pos {
	SCRIPT inside($pos,"majestic_garden") AS $region;

	if($region == true) {
		GRANT;
	}
}
*/

			$res = $this->ruleset;
			
			#VALUE ([^ ]+) AS ([$][^ ]+) {#
			$match = array();
			preg_match_all("#VALUE ([^ ]+) AS ([$][^ ]+) {#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$func = "_".md5(microtime());

				$tmp = '$this->registerValue("'.$match[1][$key].'","'.$func.'");

function '.$func.'('.$match[2][$key].',$_P,$_CB) {
	global $_CACHE;
	$_IDENT = "'.$match[1][$key].'";';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#ENTITY ([^ ]+) AS ([$][^ ]+) {#
			$match = array();
			preg_match_all("#ENTITY ([^ ]+) AS ([$][^ ]+) {#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$func = "_".md5(microtime());

				$tmp = '$this->registerEntity("'.$match[1][$key].'","'.$func.'");

function '.$func.'('.$match[2][$key].',$_P,$_CB) {
	global $_CACHE;
	$_IDENT = "'.$match[1][$key].'";';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#EVENT ([^ ]+) AS ([$][^ ]+) {#
			$match = array();
			preg_match_all("#EVENT ([^ ]+) AS ([$][^ ]+) {#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$func = "_".md5(microtime());

				$tmp = '$this->registerEvent("'.$match[1][$key].'","'.$func.'");

function '.$func.'('.$match[2][$key].',$_P,$_CB) {
	global $_CACHE;
	$_IDENT = "'.$match[1][$key].'";';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#GRANT ([^;]*);#
			$match = array();
			preg_match_all("#GRANT ([^;]*);#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$tmp = '$_P->grant('.$match[1][$key].');';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#GRANT;#
			$match = array();
			preg_match_all("#GRANT;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->grant();';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#DENY;#
			$match = array();
			preg_match_all("#DENY;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->deny();';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#UNLOCK;#
			$match = array();
			preg_match_all("#UNLOCK;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->unlock();';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#RESET;#
			$match = array();
			preg_match_all("#RESET;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->reset_();';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#UNLOCK_ALL;#
			$match = array();
			preg_match_all("#UNLOCK_ALL;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->unlock_all();';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#RESET_ALL;#
			$match = array();
			preg_match_all("#RESET_ALL;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->reset_all();';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#FINAL VALUE;#
			$match = array();
			preg_match_all("#FINAL VALUE;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->unregisterValue($_IDENT,$_CB);';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#FINAL ENTITY;#
			$match = array();
			preg_match_all("#FINAL ENTITY;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->unregisterEntity($_IDENT,$_CB);';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}
			#FINAL EVENT;#
			$match = array();
			preg_match_all("#FINAL EVENT;#",$this->ruleset,$match);
			foreach($match[0] as $elem) {
				$tmp = '$_P->unregisterEvent($_IDENT,$_CB);';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#CACHE ([^ ]+) AS ([$][^ ]+);#
			$match = array();
			preg_match_all("#CACHE ([^ ]+) AS ([$][^;]+);#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$tmp = $match[2][$key].' = $_CACHE->getData(\''.$match[1][$key].'\');';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#CACHE ([^ ]+) SET ([$][^ ]+);#
			$match = array();
			preg_match_all("#CACHE ([^ ]+) SET ([$][^;]+);#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$tmp = '$_CACHE->writeData(\''.$match[1][$key].'\','.$match[2][$key].');';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#SCRIPT ([^ ]+) AS ([$][^ ]+);#
			$match = array();
			preg_match_all("#SCRIPT ([^\(]+)\(([^\)]*)\) AS ([$][^;]+);#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$tmp = '@include_once("script/'.$match[1][$key].'_script.php");
	'.$match[3][$key].' = '.$match[1][$key].'('.$match[2][$key].');';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			$this->ruleset_parsed = $res;
		}
	}
?>