<?php
	trait Node {
		protected $idx;
		protected $id;
		protected $parent;

		final function getID() {
			return $this->id;
		}

		final function getIdx() {
			return $this->idx;
		}

		final function getParent() {
			return $this->parent;
		}

		final function setIdx($i) {
			$this->idx = $i;
		}

		final function setID($id) {
			$this->id = $id;
		}

		final function setParent($p) {
			$this->parent = $p;
		}
	}
?>