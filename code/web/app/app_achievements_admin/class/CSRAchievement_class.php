<?php
	class CSRAchievement extends AchAchievement implements CSR {
		use CSRDispatcher;
		
		function CSRAchievement($data,$parent) {
			parent::__construct($data,$parent);
		}

		protected function makeChild($d) {
			return new CSRPerk($d,$this);
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

		function setPerkDone($id) {
			echo "perk<br>";
			$this->setChildDone($id);

			echo "ach<br>";

			$this->parent->addChildDone($this->id);
			
			if(!$this->hasOpen()) {
				$this->parent->removeChildOpen($this->id);
			}
		}

		function setPerkOpen($id) {
			echo "perk<br>";
			$this->setChildOpen($id);
			echo "ach<br>";

			$this->parent->addChildOpen($this->id);
			
			if(!$this->hasDone()) {
				$this->parent->removeChildDone($this->id);
			}
		}
	}
?>