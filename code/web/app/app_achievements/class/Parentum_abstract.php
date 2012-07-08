<?php
	abstract class Parentum extends Node {
		/*---------------------------
			This class allows external access to the child-node list.
			Use the NodeIterator to iterate through the list since
			the numeric array keys might have gaps due to node removals!

			Once init() has been called, an AVLTree is used to support the
			functions removeChild() and findChild(). init() must be called
			before adding any nodes!
		---------------------------*/
		protected $nodes;

		function Parentum() {
			parent::__construct();
			$this->nodes = new DLL(); // Doubly Linked List
		}

		abstract protected function makeChild($args); // overwriteable child generator; allows to define child type (eg.: admin classes that inherit from base class)

		function isEmpty() {
			return $this->nodes->isEmpty();
		}

		function addChild($data) {
			$this->nodes->addNode($data);
		}

		function removeChild($id) {
			$this->nodes->removeNode($id);
		}

		function getChildByID($id) {
			return $this->nodes->findNode($id);
		}

		function getChildDataByID($id) {
			$tmp = $this->getChildByID($id);
			if($tmp != null) {
				return $tmp->data;
			}
			return null;
		}

		function getIterator() {
			return $this->nodes->getIterator();
		}
	}
?>