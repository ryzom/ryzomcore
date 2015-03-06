<?php
	class Skill extends Entity {
		public $skill;
		public $current;
		public $base;
		public $maxlvlreached;
		public $xp;
		public $xpnextlvl;

		function Skill() {
			$this->setName("skill");
		}
	}
?>