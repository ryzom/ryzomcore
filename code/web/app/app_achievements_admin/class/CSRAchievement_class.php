<?php
	class CSRAchievement extends AchAchievement implements CSR {
		
		function CSRAchievement(&$data) {
			parent::__construct($data);
		}

		function grant($pid) {
			foreach($this->nodes as $elem) {
				$elem->grant($pid);
			}
		}

		function deny($pid) {
			foreach($this->nodes as $elem) {
				$elem->deny($pid);
			}
		}
	}
?>