<?php
	trait InDev {
		/*---------------------------
			This trait provides basic functionality used to
			handle "in development" flags.
		---------------------------*/

		protected $dev;

		function inDev() {
			return ($this->dev == 1);
		}

		function getDev() {
			return $this->dev;
		}

		function setInDev($tf) {
			if($tf == true) {
				$this->setDev(1);
			}
			else {
				$this->setDev(0);
			}

			$this->update();
		}

		function setDev($d) {
			$this->dev = $d;
		}
	}
?>