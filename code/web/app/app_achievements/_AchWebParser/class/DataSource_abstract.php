<?php
	abstract class DataSource {
		private $types = array();
		private $priority = array();
		private $write = false;
		#MISSING: offered values

		function DataSource() {

		}

		function getTypes() {
			return $this->types;
		}

		function getPriority($type) {
			return $this->priority[$type];
		}

		abstract function getData($type,$ident,$field = array());

		abstract function writeData($type,$ident,$field = array(),$value = array());
	}
?>