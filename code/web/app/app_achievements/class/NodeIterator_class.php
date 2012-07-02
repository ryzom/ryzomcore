<?php
	class NodeIterator {
		/*---------------------------
			The NodeIterator can be used just like a foreach() loop to iterate
			arrays.

			Sample:
			$iter = new NodeIterator(array());
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				// ...
			}
		---------------------------*/
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