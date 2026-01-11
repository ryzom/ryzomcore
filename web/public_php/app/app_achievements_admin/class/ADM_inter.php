<?php
	interface ADM {
		/*---------------------------
			The admin interface defines the basic operations every node must handle.
			These are needed to invoke the database operations insert, update and delete.
		---------------------------*/
		function delete_me();

		function update();

		function insert();

		function getID(); // needed to identify a node
	}
?>