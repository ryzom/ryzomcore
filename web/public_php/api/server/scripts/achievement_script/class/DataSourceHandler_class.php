<?php
	/*
	 * This class is the wrapper for all loaded datasources. It will store them and pass the "drive" command on to them.
	 */
	class DataSourceHandler {
		private $source;

		function DataSourceHandler() {
			$this->source = array();
		}

		function addSource($src) {
			$this->source[] = $src;
		}

		function drive($cdata) { // tell the datasources to start reading data
			foreach($this->source as $elem) {
				$elem->drive($cdata);
			}
		}

	}
?>