<?php
	class RyzomUser {
		private $data;
		
		function RyzomUser($data) {
			$this->data = $data;
		}

		function getID() {
			return 1;
		}

		function getLang() {
			return $this->data['lang'];
		}

		function isIG() {
			return $this->data['ig'];
		}

		function getParam($p) {
			return $this->data[$p];
		}
	}
?>