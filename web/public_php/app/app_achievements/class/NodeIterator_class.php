<?php
	class NodeIterator {
		/*---------------------------
			The NodeIterator can be used to iterate linked lists.

			Sample:
			$iter = new NodeIterator(array());
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				// ...
			}
		---------------------------*/
		private $node;

		function NodeIterator($node) {
			$this->node = $node;
		}

		function hasNext() {
			if($this->node == null) {
				return false;
			}

			return true;
		}

		function getNext() {
			$n = $this->node;
			$this->node = $this->node->getChild();
			return $n->data;
		}
	}
?>