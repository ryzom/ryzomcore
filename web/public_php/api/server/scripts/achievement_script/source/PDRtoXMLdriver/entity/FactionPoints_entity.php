<?php
	class FactionPoints extends Entity {
		public $faction;
		public $value;

		function FactionPoints() {
			$this->setName("faction_points");
		}
	}
?>