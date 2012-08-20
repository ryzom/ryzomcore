<?php
	/*
	 * Unlike normal values, entities may contain several values. This is their wrapper.
	 */
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