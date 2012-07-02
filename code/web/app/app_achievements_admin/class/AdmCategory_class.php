<?php
	class AdmCategory extends AchCategory {
		use AdmDispatcher;
		
		function AdmCategory($id,$cult = null,$civ = null) {
			$this->init();
			parent::__construct($id,$cult,$civ);
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