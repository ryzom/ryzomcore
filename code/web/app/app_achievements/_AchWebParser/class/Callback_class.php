<?php
	/*
	 * Callback container that handles doing the actual callback
	 */
	class Callback {
		private $who;
		private $func;

		function Callback($who,$func) {
			$this->who = $who;
			$this->func = $func;
		}

		function call($what) { // now call it
			eval(''.$this->func.'($what,$this->who,$this);');
		}
	}
?>