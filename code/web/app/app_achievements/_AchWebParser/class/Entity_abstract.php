<?php
	abstract class Entity {
		private $name;

		function getName() {
			return $this->name;
		}

		function setName($n) {
			$this->name = $n;
		}
	}
?>