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
		private $node;
		#private $curr;

		function NodeIterator($node) {
			$this->node = $node;
			#$this->curr = 0;
		}

		function hasNext() {
			#$tmp = array_keys($this->nodes);
			#return isset($this->nodes[$tmp[$this->curr]]);
			if($this->node == null) {
				#echo "empty";
				return false;
			}
			
			#if($this->node->getChild() == null) {
			#	return false;
			#}

			#echo "true";

			return true;
		}

		function getNext() {
			#$tmp = array_keys($this->nodes);
			#return $this->nodes[$tmp[$this->curr++]];
			$n = $this->node;
			$this->node = $this->node->getChild();
			return $n->data;
		}
	}
?>