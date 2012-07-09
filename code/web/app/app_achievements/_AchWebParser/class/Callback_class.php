<?php
	class Callback {
		private $who;
		private $func;

		function Callback($who,$func) {
			$this->who = $who;
			$this->func = $func;
		}

		function call($what) {
			eval(''.$this->func.'($what,$this->who,$this);');
		}
	}
?>