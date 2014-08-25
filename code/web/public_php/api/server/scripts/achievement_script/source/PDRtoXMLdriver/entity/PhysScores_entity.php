<?php
	class PhysScores extends Entity {
		public $score;
		public $current;
		public $base;
		public $max;
		public $baseregeneraterepos;
		public $baseregenerateaction;
		public $currentregenerate;

		function PhysScores() {
			$this->setName("phys_scores");
		}
	}
?>