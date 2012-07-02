<?php
	class CSRPerk extends AchPerk implements CSR {
		use CSRDispatcher;
		
		function CSRPerk($data,$parent) {
			$this->init();
			parent::__construct($data,$parent);
		}

		protected function makeChild($d) {
			return new CSRObjective($d,$this);
		}

		function grant($pid) {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$this->getID()."','".$pid."','".time()."')");
			$this->done = time();
			#echo $this->idx."<br>";
			$this->parent->setPerkDone($this->idx);

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->grant($pid);
			}
		}

		function deny($pid) {
			global $DBc;
			
			$DBc->sqlQuery("DELETE FROM ach_player_perk WHERE app_perk='".$this->getID()."' AND app_player='".$pid."'");
			$this->done = 0;
			$this->parent->setPerkOpen($this->idx);

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->deny($pid);
			}
		}
	}
?>