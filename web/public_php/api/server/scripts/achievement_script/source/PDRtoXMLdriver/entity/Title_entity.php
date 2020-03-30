<?php
class Title extends Entity {
	public $title_id;
	public $title;

	function Title() {
		$this->setName("title");
		$this->title_id = "";
		$this->title = null;
	}

	function loadID() {
		global $DBc;

		$res = $DBc->sendSQL("SELECT t_id FROM ryzom_title WHERE t_male='".$DBc->mre($this->title)."' OR t_female='".$DBc->mre($this->title)."'","ARRAY");

		$this->title_id = $res[0]['t_id'];
	}
}
?>