<?php
	class RyzomUser {
		private $data;
		
		function RyzomUser($data) {
			$this->data = $data;
		}

		function getID() {
			return $this->data["id"];
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

		function getRace() {
			return 'r_'.$this->data['race'];
		}

		function getCiv() {
			return 'c_'.$this->data['civ'];
		}

		function getCult() {
			return 'c_'.$this->data['cult'];
		}

		function getName() {
			return $this->data['char_name'];
		}
	}
?>