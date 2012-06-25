<?php
	class AdmMenuNode extends AchMenuNode implements ADM { #MISSING: da fehlt die komplette logik für sub-sub-menüs!!! DU VOLLHIRT!
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
				foreach($this->nodes as $elem) {
					$res = $elem->hasAchievements();
					if($res == true) {
						return true;
					}
				}

				return false;
			}
		}

		function getNode($id) { // try to find the child node that has the given ID. Return null on failure.
			if($id == $this->getID()) { // found!
				return $this;
			}
			else {
				foreach($this->nodes as $elem) { // check children
					$tmp = $elem->getNode($id);
					if($tmp != null) {
						return $tmp;
					}
				}
				return null;
			}
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
				$this->unsetChild($elem->getID());
			}
		}

		function unsetChild($id) { // remove child with given ID from nodes list; unset should destruct.
			foreach($this->nodes as $key=>$elem) {
				if($elem->getID() == $id) {
					unset($this->nodes[$key]);
					return null;
				}
			}
		}

		function insertChild(&$n) { // insert a new child
			// insert command to create database entry
			$n->insert();

			// set the new child's parent and add it to the node list
			$n->setParent($this);
			$this->nodes[] = $n;
		}

		function update() {
			global $DBc,$_USER;

			$DBc->sqlQuery("UPDATE ach_category SET ac_parent=".mkn($this->getParentID()).",ac_order='".$this->getOrder()."',ac_image=".mkn($this->getImage()).",ac_dev='".$this->getDev()."' WHERE ac_id='".$this->getID()."'");

			#MISSING: update lang entry
			$DBc->sqlQuery("INSERT IGNORE INTO ach_category_lang (acl_category,acl_lang,acl_name) VALUES ('".$this->getID()."','".$_USER->getLang()."','".mysql_real_escape_string($this->getName())."') ON DUPLICATE KEY UPDATE acl_name='".mysql_real_escape_string($this->getName())."'");
		}

		function insert() { // write $this to the database as a new entry
			global $DBc,$_USER;

			$this->setOrder($this->parent->getNextOrder());

			$DBc->sqlQuery("INSERT INTO ach_category (ac_parent,ac_order,ac_image,ac_dev) VALUES (".mkn($this->getParentID()).",'".$this->getOrder()."',".mkn($this->getImage()).",'1')");
			$id = mysql_insert_id();
			$this->setID($id);
			#MISSING: insert lang entry
			$DBc->sqlQuery("INSERT INTO ach_category_lang (acl_category,acl_lang,acl_name) VALUES ('".$this->getID()."','".$_USER->getLang()."','".mysql_real_escape_string($this->getName())."')");
			
		}

		function setInDev($tf) {
			if($tf == true) {
				$this->setDev(1);
			}
			else {
				$this->setDev(0);
			}

			$this->update();
		}

		private function setDev($d) {
			$this->dev = $d;
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

		function setParent(&$p) {
			$this->parent = $p;
		}

		function setParentID($p) {
			if($p == null || strtolower($p) == "null") {
				$this->parent_id = null;
			}
			else {
				$this->parent_id = $p;
			}
		}

		function setID($id) {
			$this->id = $id;
		}

		function getNextOrder() {
			if($this->isEmpty()) {
				return 0;
			}

			$val = array();
			foreach($this->nodes as $elem) {
				$val[] = $elem->getOrder();
			}

			return (max($val)+1);
		}
	}

?>