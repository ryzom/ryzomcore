<?php
	interface Tieable {
		/*---------------------------
			The Tieable interface is needed to define methods required
			by Parentum classes that have child nodes that vary depending
			on a user's cult of civ allegiance.
		---------------------------*/

		function isTiedCult();

		function isTiedCiv();

		function getCurrentCiv();

		function getCurrentCult();
	}
?>