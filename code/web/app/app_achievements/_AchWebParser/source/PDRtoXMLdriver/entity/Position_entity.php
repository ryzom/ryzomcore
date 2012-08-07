<?php
	class Position extends Entity {
		public $x;
		public $y;
		public $z;
		public $heading;

		function Position() {
			$this->setName("position");
		}
	}
?>