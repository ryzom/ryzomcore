<?php
	class CSRAtom extends Node implements CSR {

		function CSRAtom($data,$parent) {
			$this->id = $data['atom_id'];
			$this->parent = $parent;
		}

		function grant($pid) {
			$this->clear_all($pid); #empty database
		}
		
		function deny($pid) {
			$this->clear_all($pid); #empty database
		}

		private function clear_all($pid) {
			global $DBc;
			$DBc->sqlQuery("DELETE FROM ach_player_atom WHERE apa_atom='".$this->getID()."' AND apa_player='".$pid."'");
		}
	}
?>