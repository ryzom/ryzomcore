<?php
	class LastLogStats extends Entity {
		public $logintime;
		public $duration = 0;
		public $logofftime;

		function LastLogStats() {
			$this->setName("lastlogstats");
		}
	}
?>