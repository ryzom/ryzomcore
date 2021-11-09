<?php
	class SpentSkillPoints extends Entity {
		public $skill;
		public $value;

		function SpentSkillPoints() {
			$this->setName("spentskillpoints");
		}
	}
?>