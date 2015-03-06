<?php
	class RyzomAdmin extends RyzomUser {
		function RyzomAdmin($data) {
			parent::__construct($data);
		}

		function isAdmin() {
			return true;
		}

		function isCSR() {
			return true;
		}
	}
?>