<?php
	/*
	 * Unlike normal values, entities may contain several values. This is their wrapper.
	 */
	abstract class Entity {
		private $name;
		private $_dataset = array();

		function getName() {
			return $this->name;
		}

		function setName($n) {
			$this->name = $n;
		}

		function setData($key,$data) {
			$this->_dataset[$key] = $data;
		}

		function getData($key) {
			return $this->_dataset[$key];
		}
	}
?>