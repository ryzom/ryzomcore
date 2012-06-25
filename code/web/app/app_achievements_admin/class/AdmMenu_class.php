<?php
	class AdmMenu extends AchMenu implements AdmDispatcher {
		
		function AdmMenu($open) {
			parent::__construct($open);

			unset($this->nodes[0]); // unset the auto-generated "summary" node
		}

		protected function makeChild($d) { // override child generator to use admin classes
			return new AdmMenuNode($d,$this);
		}

		function removeNode($id) { // find the node that has the ID we want to delete. If found, call it's delete function.
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$this->unsetChild($id);
			}
		}

		function insertNode(&$n) {
			if($n->getParentID() != null) {
				$res = $this->getNode($n->getParentID());
				if($res != null) {
					$n->setParent($res);
					$res->insertChild($n);
				}
			}
			else {
				$n->setParent($this);
				$n->insert();
				$this->nodes[] = $n;
			}
		}

		function updateNode($id,$data) { #MISSING: data handling...
			$res = $this->getNode($id);
			if($res != null) {
				$res->setName($data['acl_name']);
				$res->setImage($data['ac_image']);
				$res->update();
			}
		}

		function swapOrder($a,$b) {
			$tmp_a = $this->getNode($a);
			if($tmp_a != null) {
				$tmp_b = $this->getNode($a);
				if($tmp_b != null) {
					$tmp = $tmp_b->getOrder();
					$tmp_b->setOrder($tmp_a->getOrder());
					$tmp_a->setOrder($tmp);

					if($tmp_a->getParentID() == $tmp_b->getParentID()) {
						$tmp_a->getParent()->swapChild($a,$b);
					}
				}
			}
		}

		function getNode($id) { // try to find the MenuNode that has the given ID. Return null on failure.
			foreach($this->nodes as $elem) {
				$tmp = $elem->getNode($id);
				if($tmp != null) {
					return $tmp;
				}
			}

			return null;
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

		function unsetChild($id) { // remove child with given ID from nodes list; unset should destruct it.
			foreach($this->nodes as $key=>$elem) {
				if($elem->getID() == $id) {
					unset($this->nodes[$key]);
					return null;
				}
			}
		}
	}
?>