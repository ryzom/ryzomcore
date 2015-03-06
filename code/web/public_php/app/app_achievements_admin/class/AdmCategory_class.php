<?php
	class AdmCategory extends AchCategory {
		#########################
		# PHP 5.3 compatible
		# AdmDispatcher_trait replaces this in PHP 5.4


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
		
		function AdmCategory($id,$race,$cult = null,$civ = null) {
			parent::__construct($id,$race,$cult,$civ);
		}

		protected function makeChild($d) {
			$a = new AdmAchievement($d,$this);
			return $a;
		}
		
		#@overrides AdmDispatcher::insertNode()
		function insertNode($n) {
			$n->insert();
			$this->addOpen($n);
		}

		function setLang($lang,$txt) {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_category_lang (acl_category,acl_lang,acl_name) VALUES ('".$this->getID()."','".$DBc->sqlEscape($lang)."','".$DBc->sqlEscape($txt)."') ON DUPLICATE KEY UPDATE acl_name='".$DBc->sqlEscape($txt)."'");
		}

		function getLang($lang) {
			global $DBc;

			$res = $DBc->sqlQuery("SELECT acl_name FROM ach_category_lang WHERE acl_category='".$this->getID()."' AND acl_lang='".$DBc->sqlEscape($lang)."'");
			return $res[0]['acl_name'];
		}

		function update() {
			global $DBc;
		}
	}
?>