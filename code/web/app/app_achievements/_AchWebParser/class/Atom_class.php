<?php
	class Atom {
		private $ruleset;
		private $ruleset_parsed;

		private $id;
		private $objective;

		function Atom(&$data) {
			$this->ruleset = $data['atom_ruleset'];
			$this->ruleset_parsed = false;

			$this->id = $data['atom_id'];
			$this->objective = $data['atom_objective'];
		}

		private function parseRuleset() {
			#WORKPAD:####
			/*
			Trigger:
				by value
				(by event)

			Sources:
				XML
				valuecache
				ring_open
				(Achievement Service)
					(Mirror Service)
			
			Keywords:
				VALUE
				GRANT:EVENT player_death
				DENY:TIMER 3600
				RESET
				RESET_ALL
				UNLOCK
				UNLOCK_ALL

				IF
				SCRIPT
				MSG
			
			VALUE dappers = c_money
			IF(dappers >= 5000) {
				GRANT
			}
			
			VALUE tmp = c_fame[scorchers]
			IF(tmp == 0) {
				DENY:3600
			}
			
			VALUE x = c_pos_x
			VALUE y = c_pos_y
			SCRIPT inside(x,y) {
				IF(MSG == "Majestic Garden") {
					GRANT
				}
			}

			EVENT player_death
			ON player_death {
				UNLOCK
			}

			EVENT region_changed
			ON region_changed {
				IF(MSG == "Majestic Garden") {
					GRANT
				}
			}
			*/
			#############


			VALUE var = name

			IF(statement) {

			}

			SCRIPT script(a,r,g,s) {
				MSG
			}

			EVENT name
			
			ON name {
				MSG
			}

			GRANT
			GRANT:EVENT name
			GRANT:TIMER seconds

			DENY
			DENY:EVENT name
			DENY:TIMER seconds

			RESET
			RESET_ALL
			UNLOCK
			UNLOCK_ALL
		}

		function evalRuleset($user) {
			global $DBc,$_DATA;

			if($this->ruleset_parsed == false) {
				$this->parseRuleset();
			}

			try {
				return eval($this->ruleset_parsed);
			}
			catch(Exception $e) {
				return $e->getMessage()
			}
		}

		private function grant($user,$condition) {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$user."','".date()."','".$DBc->mre($condition)."','GRANT')","NONE");
		}

		private function deny($user,$condition) {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$user."','".date()."','".$DBc->mre($condition)."','DENY')","NONE");
		}

		private function reset_($user) {
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$user."'","NONE");
		}

		private function reset_all() {

		}

		private function unlock($user) {
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$user."' AND apa_state='DENY'","NONE");
		}

		private function unlock_all() {

		}

		function getID() {
			return $this->id;
		}

		function getObjective() {
			return $this->objective;
		}
	}
?>