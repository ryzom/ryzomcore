<?php
	abstract class RenderNodeIterator {
		protected $nodes = array();

		function getSize() {
			return sizeof($this->nodes);
		}

		function getChild($i) {
			return $this->nodes[$i];
		}

		function isEmpty() {
			return (sizeof($this->nodes) == 0);
		}

		function getChildren() {
			return $this->nodes;
		}
	}

	abstract class AchList extends RenderNodeIterator {
		protected $child_done = array();
		protected $child_open = array();

		function getDone() {
			return $this->child_done;
		}

		function getOpen() {
			return $this->child_open;
		}

		function hasOpen() {
			return (sizeof($this->child_open) != 0);
		}

		function hasDone() {
			return (sizeof($this->child_done) != 0);
		}
	}
?>