<?php
	interface CSR {
		function grant($player);

		function deny($player);

		function getID();
	}
?>