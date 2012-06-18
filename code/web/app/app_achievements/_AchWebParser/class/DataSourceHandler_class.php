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

		function freeData($ident) {
			foreach($source as $elem) {
				$elem->freeData($ident);
			}
		}

		function getData($ident,$query) { // SELECT ? FROM c_items WHERE q>='300'
			$matches = array();
			preg_match("#SELECT (\?|\*) FROM ([^ ]+) WHERE ([.]*)#", $query, $matches);

			$mode = $matches[1];
			$type = $matches[2];
			$cond = $matches[3];

			$tmp = $this->getDataSource($type);
			if($tmp == false) { // no datasource available for this ident
				return false;
			}

			return $tmp->getData($ident,$type,$mode,$cond);
		}

		function writeData($ident,$query) { // INSERT INTO c_cache () VALUES ()
			$matches = array();
			preg_match("#INSERT INTO ([^ ]+) \(([^\)]*)\) VALUES \(([^\)]*)\)#", $query, $matches);

			$type = $matches[1];
			$keys = $matches[2];
			$data = $matches[3];

			$tmp = $this->getDataSource($type);
			if($tmp == false) { // no datasource available for this ident
				return false;
			}

			if(!$tmp->isWriteable()) { // can't write here
				return false;
			}

			return $tmp->writeData($ident,$type,$keys,$data);
		}


		private function getDataSource(&$ident) {
			if(!$this->alloc[$ident]) {
				return false; //unknown type
			}

			return $this->source[$this->alloc[$ident]];
		}
	}
?>