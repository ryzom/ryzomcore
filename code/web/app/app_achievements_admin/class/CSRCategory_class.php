<?php
	class CSRCategory extends AchCategory implements CSR {
		use CSRDispatcher;
		
		function CSRCategory($id,$race,$cult = null,$civ = null) {
			parent::__construct($id,$race,$cult,$civ);
		}

		protected function makeChild($d) {
			return new CSRAchievement($d,$this);
		}

		function grant($id) {
			return false; // category can't grant!
		}

		function deny($id) {
			return false; // category can't deny!
		}

		/*function setAchOpen($idx,$state) {
			if($state == false) {
				$this->unsetOpen($idx);
				if(!in_array($idx,$this->child_done)) {
					$this->child_done[] = $idx;
				}
			}
			else {
				if(!in_array($idx,$this->child_open)) {
					$this->child_open[] = $idx;
				}
			}
		}

		function setAchDone($idx,$state) {
			if($state == false) {
				$this->unsetDone($idx);
				if(!in_array($idx,$this->child_open)) {
					$this->child_open[] = $idx;
				}
			}
			else {
				if(!in_array($idx,$this->child_done)) {
					$this->child_done[] = $idx;
				}
			}
		}*/
	}
?>
