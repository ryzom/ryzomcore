<?php
class TPlist extends Entity {
	public $tps;

	function TPlist() {
		$this->setName("TPlist");
		$this->tps = array();
	}

	function hasTP($tp) {
		return in_array($tp,$this->tps);
	}
}
?>