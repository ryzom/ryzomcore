<?php
	interface AdmDispatcher {
		/*---------------------------
			The admin dispatcher allows us to perform operations on child nodes.
		---------------------------*/
		function insertNode(&$n);
		function removeNode($id);
		function updateNode($id,$data);
		function getNode($id);
	}
?>