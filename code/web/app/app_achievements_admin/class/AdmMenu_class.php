<?php
	class AdmMenu extends AchMenu {
		#########################
		# PHP 5.3 compatible
		# AdmDispatcher_trait replaces this in PHP 5.4

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
		
		function AdmMenu($open) {
			parent::__construct($open);

			#$this->drawTree();

			$this->removeChild(0); // unset the auto-generated "summary" node
		}

		protected function makeChild($d) { // override child generator to use admin classes
			return new AdmMenuNode($d,$this);
		}

		function removeNode($id) { // find the node that has the ID we want to delete. If found, call it's delete function.
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$tmp = $res->getParent();
				$tmp->removeChild($id);
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
				$this->addChild($n);
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
			$res = $this->getChildDataByID($id);
			if($res != null) {
				return $res;
			}

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				#echo $curr->getID();
				$tmp = $curr->getNode($id);
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
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$val[] = $curr->getOrder();
			}

			return (max($val)+1);
		}
	}
?>