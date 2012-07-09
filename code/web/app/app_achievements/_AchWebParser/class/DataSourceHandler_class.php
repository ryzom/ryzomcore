<?php
	class DataSourceHandler {
		private $source;

		function DataSourceHandler() {
			$this->source = array();
		}

		function addSource($src) {
			$this->source[] = $src;
		}

		function drive($cid) {
			foreach($this->source as $elem) {
				$elem->drive($cid);
			}
		}

	}
?>