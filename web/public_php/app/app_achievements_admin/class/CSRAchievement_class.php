<?php
	class CSRAchievement extends AchAchievement implements CSR {
		#########################
		# PHP 5.3 compatible
		# CSRDispatcher_trait replaces this in PHP 5.4

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
		#########################
		
		function CSRAchievement($data,$parent) {
			parent::__construct($data,$parent);
		}

		protected function makeChild($d) {
			return new CSRTask($d,$this);
		}

		function grant($pid) {
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->grant($pid);
				$this->setChildDone($curr->getID());
			}

			$this->parent->setChildDone($this->id);
		}

		function deny($pid) {
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->deny($pid);
				$this->setChildOpen($curr->getID());
			}

			$this->parent->setChildOpen($this->id);
		}

		function setTaskDone($id) {
			$this->setChildDone($id);


			$this->parent->addChildDone($this->id);
			
			if(!$this->hasOpen()) {
				$this->parent->removeChildOpen($this->id);
			}
		}

		function setTaskOpen($id) {

			$this->setChildOpen($id);


			$this->parent->addChildOpen($this->id);
			
			if(!$this->hasDone()) {
				$this->parent->removeChildDone($this->id);
			}
		}
	}
?>