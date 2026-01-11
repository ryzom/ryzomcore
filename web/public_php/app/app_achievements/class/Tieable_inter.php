<?php
	interface Tieable {
		/*---------------------------
			The Tieable interface is needed to define methods required
			by Parentum classes that have child nodes that vary depending
			on a user's cult of civ allegiance.
		---------------------------*/

		function hasTieRace_open();

		function hasTieAlign_open();

		function hasTieRace_done();

		function hasTieAlign_done();

		function hasTieRaceDev();

		function hasTieAlignDev();

		function isTiedRace_open($r);

		function isTiedAlign_open($cult,$civ);

		function isTiedRace_done($r);

		function isTiedAlign_done($cult,$civ);

		#function getCurrentCiv();

		#function getCurrentCult();
	}
?>