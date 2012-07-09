<?php
	class AdmAtom extends Node implements ADM {
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

		function update() {
			#$DBc->sqlQuery("UPDATE ach_atom SET atom_mandatory='".."',atom_ruleset='".."',atom_ruleset_parsed='".."' WHERE atom_id='".$this->id."'");
		}

		function insert() {
			#$DBc->sqlQuery("INSERT INTO ach_atom (atom_objective,atom_mandatory,atom_ruleset,atom_ruleset_parsed) VALUES ('".."','".."','".."','".."')");
			$id = mysql_insert_id();
			$this->setID($id);
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

		private function parse() {
			/*VALUE _money AS $money {
				CACHE blach AS $test

				if($money >= 10000 && $test == 0) {
					GRANT
					FINAL
				}
				else {
					CACHE blach SET $money
				}
			}

			$res = $this->ruleset;
			
			#VALUE ([^ ]+) AS ([^ ]+) {#
			$match = array();
			preg_match_all("#VALUE ([^ ]+) AS ([^ ]+) {#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$func = "_".md5(microtime());

				$tmp = '$this->registerValue("'.$match[1][$key].'","'.$func.'");

				function '.$func.'('.$match[2][$key].',$_P,$_CB) {
					$_IDENT = "'.$match[1][$key].'";';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#ENTITY ([^ ]+) AS ([^ ]+) {#
			$match = array();
			preg_match_all("#ENTITY ([^ ]+) AS ([^ ]+) {#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$func = "_".md5(microtime());

				$tmp = '$this->registerEntity("'.$match[1][$key].'","'.$func.'");

				function '.$func.'('.$match[2][$key].',$_P,$_CB) {
					$_IDENT = "'.$match[1][$key].'";';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#EVENT ([^ ]+) AS ([^ ]+) {#
			$match = array();
			preg_match_all("#EVENT ([^ ]+) AS ([^ ]+) {#",$this->ruleset,$match);
			foreach($match[0] as $key=>$elem) {
				$func = "_".md5(microtime());

				$tmp = '$this->registerEvent("'.$match[1][$key].'","'.$func.'");

				function '.$func.'('.$match[2][$key].',$_P,$_CB) {
					$_IDENT = "'.$match[1][$key].'";';

				//replace
				$res = str_replace($elem,$tmp,$res);
			}

			#CACHE ([^ ]+) AS ([^ ]+)#

			#GRANT#

			#FINAL#

			#CACHE ([^ ]+) SET ([^ ]+)#*/
		}
	}
?>