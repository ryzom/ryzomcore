<?php
	class AdmCategory extends AchCategory implements AdmDispatcher {
		
		function AdmCategory($id,$cult = null,$civ = null) {
			parent::__construct($id,$cult,$civ);
		}

		protected function makeChild(&$d) {
			return new AdmAchievement($d,$this);
		}

		function insertNode(&$n) {
			$n->insert();
			$this->nodes[] = $n;
		}

		function removeNode($id) {
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$this->unsetChild($id);
			}
		}

		function updateNode($id,$data) {
			$res = $this->getNode($id);
			if($res != null) {
				#MISSING: set new data
				#aa_id 	aa_category 	aa_parent 	aa_tie_race 	aa_tie_cult 	aa_tie_civ 	aa_image 	aa_dev
				$res->update();
			}
		}

		function getNode($id) { // try to find the Achievement node that has the given ID. Return null on failure.
			foreach($this->nodes as $elem) {
				if($elem->getID == $id) {
					return $elem;
				}
			}

			return null;
		}

		function unsetChild($id) { // remove child with given ID from nodes list; unset should destruct it.
			foreach($this->nodes as $key=>$elem) {
				if($elem->getID() == $id) {
					unset($this->nodes[$key]);
					return null;
				}
			}
		}

	}
?>