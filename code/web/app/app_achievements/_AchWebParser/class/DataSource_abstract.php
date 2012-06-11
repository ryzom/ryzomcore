<?php
	abstract class DataSource {
		private $types = array();
		private $write = false;

		function DataSource() {
			require_once(dirname(__FILE__)."/conf.php");

			$this->types = $CONF["types"];
			$this->write = $CONF["write"];
		}

		function getTypes() {
			return $this->types;
		}

		function isWriteable() {
			return $this->write;
		}

		abstract function getData($ident,$field,$type);

		abstract function writeData($ident,$field,$data,$type);
	}
?>