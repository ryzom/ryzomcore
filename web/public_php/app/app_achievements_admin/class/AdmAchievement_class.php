<?php
	class AdmAchievement extends AchAchievement implements ADM {
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
		
		function AdmAchievement($data,$parent) {
			parent::__construct($data,$parent);
		}

		protected function makeChild($d) {
			return new AdmTask($d,$this);
		}

		function getLang($lang) {
			global $DBc;

			$res = $DBc->sqlQuery("SELECT * FROM ach_achievement_lang WHERE aal_achievement='".$this->getID()."' AND aal_lang='".$lang."'");

			return array(0=>$res[0]['aal_name'],1=>$res[0]['aal_template']);
		}

		function setLang($lang,$txt,$tpl) {
			global $DBc,$_USER;

			$DBc->sqlQuery("INSERT INTO ach_achievement_lang (aal_achievement,aal_lang,aal_name,aal_template) VALUES ('".$this->getID()."','".$DBc->sqlEscape($lang)."','".$DBc->sqlEscape($txt)."',".mkn($tpl).") ON DUPLICATE KEY UPDATE aal_name='".$DBc->sqlEscape($txt)."',aal_template=".mkn($tpl)."");

			if($_USER->getLang() == $lang) {
				$this->name = $txt;
				$this->template = $tpl;
			}
		}
		
		#@overrides AdmDispatcher::insertNode()
		function insertNode($n) {
			$n->insert();
			$this->addOpen($n);
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_achievement WHERE aa_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_objective WHERE ao_metalink='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_achievement_lang WHERE NOT EXISTS (SELECT * FROM ach_achievement WHERE aa_id=aal_achievement)");
			
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->delete_me();
				$this->removeChild($curr->getID());
			}
		}

		function update() {
			global $DBc;

			$DBc->sqlQuery("UPDATE ach_achievement SET aa_category='".$this->getCategory()."',aa_parent=".mkn($this->getParentID()).",aa_image='".$DBc->sqlEscape($this->getImage())."',aa_dev='".$this->getDev()."',aa_sticky='".$DBc->sqlEscape($this->getSticky())."' WHERE aa_id='".$this->getID()."'");

			#MISSING: update lang entry
			$DBc->sqlQuery("INSERT INTO ach_achievement_lang (aal_achievement,aal_lang,aal_name,aal_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).") ON DUPLICATE KEY UPDATE aal_name='".$DBc->sqlEscape($this->getName())."',aal_template=".mkn($this->getTemplate())."");
		}

		function insert() {
			global $DBc;

			$this->dev = 1;

			$DBc->sqlQuery("INSERT INTO ach_achievement (aa_category,aa_parent,aa_image,aa_dev,aa_sticky) VALUES ('".$this->getCategory()."',".mkn($this->getParentID()).",'".$DBc->sqlEscape($this->getImage())."','1','".$DBc->sqlEscape($this->getSticky())."')");
			$id = $DBc->insertID();
			$this->setID($id);

			$DBc->sqlQuery("INSERT INTO ach_achievement_lang (aal_achievement,aal_lang,aal_name,aal_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).")");
		}


		function setCategory($c) {
			$this->category = $c;
		}

		function setImage($i) {
			$this->image = $i;
		}

		function setName($n) {
			$this->name = $n;
		}

		function setTemplate($t) {
			$this->template = $t;
		}

		function orderTasks() {

			$i = 0;
			$start = $this->findParentID(null);

			while($start != null) {
				$start->setTorder($i);
				$start->update();
				$i++;
				#echo $i;
				$start = $this->findParentID($start->getID());
			}
		}

		private function findParentID($id) {
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->getParentID() == $id) {
					return $curr;
				}
			}

			return null;
		}

		function setParentID($p) {
			$this->parent_id = $p;
		}

		function setSticky($s) {
			$this->sticky = $s;
		}
	}
?>