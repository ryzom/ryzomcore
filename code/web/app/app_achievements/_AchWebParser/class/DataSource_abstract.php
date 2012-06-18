<?php
	abstract class DataSource {
		private $data;
		private $types = array();
		private $write = false;

		function DataSource() {
			require_once(dirname(__FILE__)."/conf.php");

			$this->types = $CONF["types"];
			$this->write = $CONF["write"];
			$this->data = array();
		}

		function freeData($ident) {
			unset $this->data[$ident];
		}

		function getTypes() {
			return $this->types;
		}

		function isWriteable() {
			return $this->write;
		}

		function getData($ident,$type,$mode,$cond) {
			if(!isset($this->data[$ident])) {
				$this->data[$ident] = array();
			}

			if(!isset($this->data[$ident][$type])) {
				$this->loadData($ident,$type);
			}
			
			if($mode == "*") {
				return $this->data[$ident][$type]->getRows($cond);
			}
			else {
				return $this->data[$ident][$type]->countRows($cond);
			}
		}

		abstract function loadData($ident,$type);
		abstract function writeData($ident,$type,$keys,$data);
	}
?>