<?php
	class CSRObjective extends AchObjective implements CSR {
		use CSRDispatcher;

		#private $nodes;
		
		function CSRObjective($data,$parent) {
			$this->init();
			parent::__construct($data,$parent);

			global $DBc;

			$res = $DBc->sqlQuery("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->getID()."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addChild($this->makeChild($res[$i]));
			}
		}

		protected function makeChild($d) {
			return new CSRAtom($d,$this);
		}

		function grant($pid) {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$this->getID()."','".$pid."','".time()."')");
			$this->done = 1;
			$this->progress = $this->value;

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->grant($pid);
			}
		}

		function deny($pid) {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_player_objective WHERE apo_objective='".$this->getID()."' AND apo_player='".$pid."'");
			$this->done = 0;
			$this->progress = 0;

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->deny($pid);
			}
		}
	}
?>