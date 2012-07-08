<?php
	trait AdmDispatcher {
		/*---------------------------
			The AdminDispatcher trait provides functions to pass
			manipulation requests on to child nodes.

			Classes using this trait must be inherited from Parentum!

			The "path" is definded a an enumeration of id numbers along
			the structural path, seperated by ";",
			
			eg.: 1;2;5;18
			This will try to return the node with the id "18" that is a child
			of "5", which is child of "2", which is child of "1". "1" in this
			case is the first instance that calls getElementByPath() without
			arguments.
		---------------------------*/

		function insertNode($n) {
			$n->setParent($this);
			$n->insert();
			$this->addChild($n);
		}

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
	}
?>