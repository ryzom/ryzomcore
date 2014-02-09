<?php
	abstract class Node {
		/*---------------------------
			This class provides basic functionality common to nodes.

			Every node has an id and a parent.
		---------------------------*/

		protected $id;
		protected $parent;

		function Node() {
			// dummy constructor
		}

		final function getID() {
			return $this->id;
		}

		final function getParent() {
			return $this->parent;
		}

		final function setID($id) {
			$this->id = $id;
		}

		final function setParent($p) {
			$this->parent = $p;
		}
	}
?>