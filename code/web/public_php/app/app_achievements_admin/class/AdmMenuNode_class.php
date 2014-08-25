<?php
	class AdmMenuNode extends AchMenuNode implements ADM {
		private $ach_count;
	
		function AdmMenuNode($data,$parent) {
			parent::__construct($data,$parent);

			global $DBc;

			$res = $DBc->sqlQuery("SELECT count(*) as anz FROM ach_achievement WHERE aa_category='".$this->id."'");
			$this->ach_count = $res[0]['anz'];
		}

		protected function makeChild($d) { // override child generator to use admin classes
			return new AdmMenuNode($d,$this);
		}

		function hasAchievements() {
			if($this->ach_count != 0) {
				return true;
			}
			else {
				$iter = $this->getIterator();
				while($iter->hasNext()) {
					$elem = $iter->getNext();
					$res = $elem->hasAchievements();
					if($res == true) {
						return true;
					}
				}

				return false;
			}
		}

		function getNode($id) { // try to find the child node that has the given ID. Return null on failure.
			$res = $this->getChildDataByID($id);
			if($res != null) {
				return $res;
			}

			$iter = $this->getIterator();
			while($iter->hasNext()) { // check children
				$curr = $iter->getNext();
				$tmp = $curr->getNode($id);
				if($tmp != null) {
					return $tmp;
				}
			}

			return null;
		}

		function delete_me() { // remove this node
			global $DBc;

			// remove from database
			$DBc->sqlQuery("DELETE FROM ach_category WHERE ac_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_category WHERE ac_parent='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_category_lang WHERE NOT EXISTS (SELECT * FROM ach_category WHERE ac_id=acl_category)");
			
			// call delete function for all children
			foreach($this->nodes as $elem) {
				$elem->delete_me();
				$this->removeChild($elem->getID());
			}
		}

		function insertChild(&$n) { // insert a new child
			// insert command to create database entry
			$n->insert();

			// set the new child's parent and add it to the node list
			$n->setParent($this);
			$this->addChild($n);
		}

		function update() {
			global $DBc,$_USER;

			$DBc->sqlQuery("UPDATE ach_category SET ac_parent=".mkn($this->getParentID()).",ac_order='".$this->getOrder()."',ac_image=".mkn($this->getImage()).",ac_dev='".$this->getDev()."' WHERE ac_id='".$this->getID()."'");
			#echo "<br>".$this->getImage()." =>UPDATE ach_category SET ac_parent=".mkn($this->getParentID()).",ac_order='".$this->getOrder()."',ac_image=".mkn($this->getImage()).",ac_dev='".$this->getDev()."' WHERE ac_id='".$this->getID()."'";

			#MISSING: update lang entry
			$DBc->sqlQuery("INSERT IGNORE INTO ach_category_lang (acl_category,acl_lang,acl_name) VALUES ('".$this->getID()."','".$_USER->getLang()."','".$DBc->sqlEscape($this->getName())."') ON DUPLICATE KEY UPDATE acl_name='".$DBc->sqlEscape($this->getName())."'");
		}

		function insert() { // write $this to the database as a new entry
			global $DBc,$_USER;

			$this->setOrder($this->parent->getNextOrder());

			$DBc->sqlQuery("INSERT INTO ach_category (ac_parent,ac_order,ac_image,ac_dev) VALUES (".mkn($this->getParentID()).",'".$this->getOrder()."',".mkn($this->getImage()).",'1')");
			$id = $DBc->insertID();
			$this->setID($id);
			#MISSING: insert lang entry
			$DBc->sqlQuery("INSERT INTO ach_category_lang (acl_category,acl_lang,acl_name) VALUES ('".$this->getID()."','".$_USER->getLang()."','".$DBc->sqlEscape($this->getName())."')");
			
		}

		private function setOrder($o) {
			$this->order = $o;
			$this->update();
		}

		function swapChild($a,$b) {
			$ids = array();
			foreach($this->nodes as $key=>$elem) {
				if($a == $elem->getID() || $b == $elem->getID()) {
					$ids[] = $key;
				}

				if(sizeof($ids) == 2) {
					break;
				}
			}

			$tmp = $this->nodes[$ids[0]];
			$this->nodes[$ids[0]] = $this->nodes[$tmp[1]];
			$this->nodes[$ids[1]] = $tmp;
		}

		function setName($n) {
			$this->name = $n;
		}

		function setImage($i) {
			if($i == null || strtolower($i) == "null") {
				$this->image = null;
			}
			else {
				$this->image = $i;
			}
		}


		function setParentID($p) {
			if($p == null || strtolower($p) == "null") {
				$this->parent_id = null;
			}
			else {
				$this->parent_id = $p;
			}
		}


		function getNextOrder() {
			if($this->isEmpty()) {
				return 0;
			}

			$val = array();
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$elem = $iter->getNext();
				$val[] = $elem->getOrder();
			}

			return (max($val)+1);
		}
	}

?>