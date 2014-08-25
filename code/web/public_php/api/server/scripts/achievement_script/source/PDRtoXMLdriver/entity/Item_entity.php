<?php
	class Item extends Entity {
		public $inventory = "";

		public $_itemid = 0;
		public $_sheetid = "";
		public $_locslot = 0;
		public $_hp = 0;
		public $_recommended = "1";
		public $_creatorid = "(0x0000000000:00:00:00)";
		public $_phraseid = "";
		public $_dropable = null;
		public $stacksize = 1;
		public $_usenewsystemrequirement = 1;
		public $_requiredskilllevel = 0;
		public $_customtext = "";
		public $_lockedbyowner = 0;

		public $_refinventoryslot = null;
		public $refinventoryid = null;

		public $_craftparameters = array();

		function Item() {
			$this->setName("item");
		}
	}
?>