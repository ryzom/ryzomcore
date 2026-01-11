<?php
	class SkillPoints extends Entity {
		public $skill;
		public $value;

		function SkillPoints() {
			$this->setName("skillpoints");
		}
	}
?>