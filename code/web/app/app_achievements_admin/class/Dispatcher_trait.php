<?php
	trait Dispatcher {
		function insertNode(&$n) {
			#MISSING: set this as parent
			$n->insert();
			$this->nodes[] = $n;
		}

		function removeNode($id) {
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$this->removeNode($res);
			}
		}

		function updateNode($id,$data) {
			$res = $this->getNode($id);
			if($res != null) {
				#MISSING: set new data
				#
				$res->update();
			}
		}

		function getNode($id) {
			return $this->getIdx($id);
		}
	}
?>