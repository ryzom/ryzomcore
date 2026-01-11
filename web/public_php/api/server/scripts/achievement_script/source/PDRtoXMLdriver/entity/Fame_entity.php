<?php
	class Fame extends Entity {
		public $faction;
		public $fame;
		public $famememory;
		public $lastfamechangetrend;

		function Fame() {
			$this->setName("fame");
		}
	}
?>