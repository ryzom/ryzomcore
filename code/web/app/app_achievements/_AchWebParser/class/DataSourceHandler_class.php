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
				if(!is_array($this->alloc[$elem])) {
					$this->alloc[$elem] = array();
				}
				//add to list
				$this->alloc[$elem][$src->getPriority($elem)] = $i;
			}
		}

		function getData($type,$field,$ident) {
			return $this->getDataSource($type)->getData($type,$field,$ident);
		}

		private function getDataSource($type) {
			//find the highest priority datasource for given type
			if(!$this->alloc[$type]) {
				return false; //unknown type
			}
			$pos = array_keys($this->alloc[$type]);
			if(sizeof($pos) == 0) {
				return false; //no datasource for type // should not happen since type is defined by datasource
			}
			return $this->alloc[$type][$pos[0]];
		}
	}
?>