<?php
	class CSRCategory extends AchCategory implements CSR {
		function grantNode($path,$player) {
			#echo "start: ".$path." id: ".$this->getID()."<br>";
			if(is_numeric($path)) {
				//it's me (id == numeric)
				if($this->getID() == $path) {
					$this->grant($player);
					#echo "grant()<br>";
				}
			}
			else {
				//get child with the next level id and dispatch
				$tmp = explode(";",$path);

				$c = $this->getChildDataByID($tmp[1]);
				#echo "...".$tmp[1];
				if($c != null) { // check if it's really own child
					unset($tmp[0]);
					$c->grantNode(implode(";",$tmp),$player);
					#echo "grantNode()<br>";
				}
			}
			#echo "end<br>";
		}

		function denyNode($path,$player) {
			if(is_numeric($path)) {
				//it's me (id == numeric)
				if($this->getID() == $path) {
					$this->deny($player);
				}
			}
			else {
				//get child with the next level id and dispatch
				$tmp = explode(";",$path);

				if($tmp[0] == $this->getID()) { // it's my id!

					$c = $this->getChildDataByID($tmp[1]);
					if($c != null) { // check if it's really own child
						unset($tmp[0]);
						$c->denyNode(implode(";",$tmp),$player);
					}
				}
			}
		}

		function getPath($path = "") {
			if($path != "") {
				$path = ";".$path;
			}

			$path = $this->getID().$path;
			
			if($this->hasParent()) {
				$path = $this->parent->getPath($path);
			}

			return $path;
		}

		private function hasParent() {
			return ($this->parent != null);
		}
		
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
