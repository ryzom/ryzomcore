<?php
	class AdmAchievement extends AchAchievement implements ADM {
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
		
		function AdmAchievement($data,$parent) {
			parent::__construct($data,$parent);
		}

		protected function makeChild($d) {
			return new AdmPerk($d,$this);
		}
		
		#@overrides AdmDispatcher::insertNode()
		function insertNode($n) {
			$n->insert();
			$this->addOpen($n);
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_achievement WHERE aa_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_achievement WHERE apa_id='".$this->getID()."'");
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

			$DBc->sqlQuery("UPDATE ach_achievement SET aa_category='".$this->getCategory()."',aa_parent=NULL,aa_tie_race=".mkn($this->getTieRace()).",aa_tie_cult=".mkn($this->getTieCult()).",aa_tie_civ=".mkn($this->getTieCiv()).",aa_image='".$DBc->sqlEscape($this->getImage())."',aa_dev='".$this->getDev()."' WHERE aa_id='".$this->getID()."'");

			#MISSING: update lang entry
			$DBc->sqlQuery("INSERT INTO ach_achievement_lang (aal_achievement,aal_lang,aal_name,aal_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).") ON DUPLICATE KEY UPDATE aal_name='".$DBc->sqlEscape($this->getName())."',aal_template=".mkn($this->getTemplate())."");
		}

		function insert() {
			global $DBc;

			$this->dev = 1;

			$DBc->sqlQuery("INSERT INTO ach_achievement (aa_category,aa_parent,aa_tie_race,aa_tie_cult,aa_tie_civ,aa_image,aa_dev) VALUES ('".$this->getCategory()."',NULL,".mkn($this->getTieRace()).",".mkn($this->getTieCult()).",".mkn($this->getTieCiv()).",'".$DBc->sqlEscape($this->getImage())."','1')");
			$id = $DBc->insertID();
			$this->setID($id);

			$DBc->sqlQuery("INSERT INTO ach_achievement_lang (aal_achievement,aal_lang,aal_name,aal_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).")");
		}


		function setCategory($c) {
			$this->category = $c;
		}

		function setTieRace($t) {
			$this->tie_race = $t;
		}

		function setTieCiv($t) {
			$this->tie_civ = $t;
		}

		function setTieCult($t) {
			$this->tie_cult = $t;
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

		function orderPerks() {
			$iter = $this->getIterator();
			$i = 0;
			while($iter->hasNext()) {
				$curr = $iter->getNext();

				$curr->setPorder($i);
			}
		}
	}
?>