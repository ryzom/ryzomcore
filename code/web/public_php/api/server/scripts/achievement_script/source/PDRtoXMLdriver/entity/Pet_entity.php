<?php
	class Pet extends Entity {
		public $pet;
		public $ticketpetsheetid;
		public $petsheetid;
		public $pricev;
		public $ownerid;
		public $stablealias;
		public $landscape_x;
		public $landscape_y;
		public $landscape_z;
		public $utc_deathtick;
		public $petstatus;
		public $slot;
		public $istpallowed;
		public $satiety;
		public $customname;

		function Pet() {
			$this->setName("pet");
			#echo "created";
		}
	}
?>