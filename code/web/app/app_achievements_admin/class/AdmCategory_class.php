<?php
	class AdmCategory extends AchCategory {
		/*function insertNode($n) {
			$n->setParent($this);
			$n->insert();
			$this->addChild($n);
		}*/

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
		
		function AdmCategory($id,$race,$cult = null,$civ = null) {
			parent::__construct($id,$race,$cult,$civ);
		}

		protected function makeChild($d) {
			return new AdmAchievement($d,$this);
		}
		
		#@overrides AdmDispatcher::insertNode()
		function insertNode($n) {
			$n->insert();
			$this->addOpen($n);
		}
	}
?>