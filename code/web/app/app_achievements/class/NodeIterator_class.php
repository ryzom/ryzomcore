<?php
	class NodeIterator {
		private $nodes;
		private $curr;

		function NodeIterator(&$nodes) {
			$this->nodes = $nodes;
			$this->curr = 0;
		}

		function hasNext() {
			$tmp = array_keys($this->nodes);
			return isset($this->nodes[$tmp[$this->curr]]);
		}

		function getNext() {
			$tmp = array_keys($this->nodes);
			return $this->nodes[$tmp[$this->curr++]];
		}

		function first() {
			$this->curr = 0;
		}
	}
?>