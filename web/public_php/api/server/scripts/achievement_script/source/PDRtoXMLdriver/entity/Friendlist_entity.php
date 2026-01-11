<?php
class Friendlist extends Entity {
	public $friends = array();
	public $friendof = array();
	public $confirmed = false;

	function Friendlist() {
		$this->setName("friendlist");
	}

	function countConfirmed() {
		if($this->confirmed == false) {
			$count = 0;
			foreach($this->friends as $elem) {
				$id = $elem->getRealID();
				foreach($this->friendof as $elem2) {
					if($elem2->getRealID() == $id) {
						$count++;
					}
				}
			}

			$this->confirmed = $count;
		}

		return $this->confirmed;
	}
}
?>