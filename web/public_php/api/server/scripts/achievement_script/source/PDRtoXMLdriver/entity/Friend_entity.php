<?php
class Friend extends Entity {
	public $id = null;

	function Friend() {
		$this->setName("friend");
	}

	function getRealID() {
		$tmp = explode(":",$this->id);

		return $tmp[0];
	}
}
?>