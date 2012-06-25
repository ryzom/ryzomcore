<?php
	trait Node {
		protected $id;
		protected $parent;

		final function getID() {
			return $this->id;
		}

		final function getParent() {
			return $this->parent;
		}

		final protected function setID($id) {
			$this->id = $id;
		}

		final protected function setParent($p) {
			$this->parent = $p;
		}
	}
?>