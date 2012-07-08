<?php
	class AdmCategory extends AchCategory {
		use AdmDispatcher;
		
		function AdmCategory($id,$race,$cult = null,$civ = null) {
			parent::__construct($id,$race,$cult,$civ);
		}

		protected function makeChild($d) {
			return new AdmAchievement($d,$this);
		}
		
		#@overrides AdmDispatcher::insertNode()
		function insertNode($n) {
			$n->insert();
			$this->addOpen($n);
		}
	}
?>