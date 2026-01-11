<?php
class DeathPenalty extends Entity {

	public $NbDeath;
	public $CurrentDeathXP;
	public $DeathXPToGain;
	public $BonusUpdateTime;

	function DeathPenalty() {
		$this->setName("death_penalty");
	}
}
?>