<?php
	class DataSourceHandler {
		private $source;
		private $alloc;

		function DataSourceHandler() {
			$this->source = array();
			$this->alloc = array();
		}

		function registerDataSource($src) {
			$i = sizeof($this->source);
			$this->source[$i] = $src;
			foreach($src->getTypes() as $elem) {
				//add to list
				$this->alloc[$elem] = $i;
			}
		}

		function getData($ident,$field) {
			$type = false;
			$tmp = $this->getDataSource($field,$type);
			if($tmp == false) {
				return false;
			}
			return $tmp->getData($field,$ident,$type);
		}

		function writeData($ident,$field,$data) {
			$type = false;
			$tmp = $this->getDataSource($field,$type);
			if($tmp == false) {
				return false;
			}
			return $tmp->writeData($field,$ident,$data,$type);
		}


		private function getDataSource(&$field,&$type) {
			$type = $field;
			//allow wildcard datafields
			$tmp = explode(":",$field);
			if(sizeof($tmp) > 1) {
				$field = $tmp[1];
				$type = $tmp[0]."*";
			}

			if(!$this->alloc[$type]) {
				return false; //unknown type
			}

			return $this->source[$this->alloc[$type]];
		}
	}
?>