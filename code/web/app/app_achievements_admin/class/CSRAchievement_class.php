<?php
	class CSRAchievement extends AchAchievement implements CSR {
		use CSRDispatcher;
		
		function CSRAchievement($data,$parent) {
			$this->init();
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
				$this->setChildDone($curr->getIdx());
			}

			$this->parent->setChildDone($this->idx);
		}

		function deny($pid) {
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->deny($pid);
				$this->setChildOpen($curr->getIdx());
			}

			$this->parent->setChildOpen($this->idx);
		}

		function setPerkDone($idx) {
			echo "perk<br>";
			$this->setChildDone($idx);

			echo "ach<br>";

			$this->parent->addChildDone($this->idx);
			
			if(!$this->hasOpen()) {
				$this->parent->removeChildOpen($this->idx);
			}
		}

		function setPerkOpen($idx) {
			echo "perk<br>";
			$this->setChildOpen($idx);
			echo "ach<br>";

			$this->parent->addChildOpen($this->idx);
			
			if(!$this->hasDone()) {
				$this->parent->removeChildDone($this->idx);
			}
		}
	}
?>