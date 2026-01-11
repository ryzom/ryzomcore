<?php
	trait InDev {
		/*---------------------------
			This trait provides basic functionality used to
			handle "in development" flags.
		---------------------------*/

		protected $dev;

		final function inDev() {
			return ($this->dev == 1);
		}

		final function getDev() {
			return $this->dev;
		}

		final function setInDev($tf) {
			if($tf == true) {
				$this->setDev(1);
			}
			else {
				$this->setDev(0);
			}

			$this->update();
		}

		final function setDev($d) {
			$this->dev = $d;
		}
	}
?>