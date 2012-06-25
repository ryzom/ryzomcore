<?php
	abstract class Parentum {
		/*---------------------------
			This class allows external access to the child-node list.
			Use the NodeIterator to iterate through the list since
			the numeric array keys might have gaps due to node removals!
		---------------------------*/
		protected $nodes = array();

		abstract protected function makeChild($args); // overwriteable child generator; allows to define child type (eg.: admin classes that inherit base class)

		final function getSize() {
			return sizeof($this->nodes);
		}

		final function isEmpty() {
			return (sizeof($this->nodes) == 0);
		}

		final function getIterator() {
			return new NodeIterator($this->nodes);
		}

		final function addChild($n) {
			$tmp = sizeof($this->nodes);
			$this->nodes[] = $n;
			return $tmp;
		}

		function removeChild($id) {
			$this->removeIdx($this->getIdx($id));
		}

		function removeNode($n) {
			$this->removeIdx($this->findNode($n));
		}

		final protected function removeIdx($idx) {
			if($idx != null) {
				unset($this->nodes[$idx]);
			}
		}

		final protected function findNode($n) {
			foreach($this->nodes as $key=>$elem) {
				if($this->nodes[$key] === $n) {
					return $key;
				}
			}

			return null;
		}

		final protected function findNodeIdx($idx) {
			return $this->nodes[$idx];
		}

		final protected function getIdx($id) {
			foreach($this->nodes as $key=>$elem) {
				if($elem->getID() == $id) {
					return $key;
				}
			}

			return null;
		}
	}
?>