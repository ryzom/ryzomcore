<?php
	class Mission extends Entity {
		public $mission;
		public $successful;
		public $utc_lastsuccessdate;

		function Mission() {
			$this->setName("mission");
		}
	}
?>