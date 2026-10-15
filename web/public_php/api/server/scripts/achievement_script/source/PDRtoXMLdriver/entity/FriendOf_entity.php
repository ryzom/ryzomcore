<?php
class FriendOf extends Entity {
	public $id = null;

	function FriendOf() {
		$this->setName("friendof");
	}

	function getRealID() {
		$tmp = explode(":",$this->id);

		return $tmp[0];
	}
}
?>