<?php
	interface CSR {
		function grant($player_id);

		function deny($player_id);
	}
?>