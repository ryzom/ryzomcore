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
			return $this->data['race'];
		}

		function getCiv() {
			return $this->data['civilization'];
		}

		function getCult() {
			return $this->data['cult'];
		}

		function getName() {
			return $this->data['name'];
		}
	}
?>