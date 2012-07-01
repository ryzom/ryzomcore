<?php
	trait InDev {
		protected $dev;

		function inDev() {
			return ($this->dev == 1);
		}

		function getDev() {
			return $this->dev;
		}
	}
?>