<?php
	class XMLapi extends DataSource {
		function XMLapi() {
			$this->types[] = "c_stats";
			$this->types[] = "c_items";

			$this->write = false;
		}

		function getData() {

		}

		function writeData() {
			return false;
		}

	}
?>