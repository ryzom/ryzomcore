<?php
	class PermanentMod extends Entity {
		public $score;
		public $value;

		function PermanentMod() {
			$this->setName("permanentmodifiers");
		}
	}
?>