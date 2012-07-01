<?php
	abstract class Parentum {
		/*---------------------------
			This class allows external access to the child-node list.
			Use the NodeIterator to iterate through the list since
			the numeric array keys might have gaps due to node removals!

			Once init() has been called, an AVLTree is used to support the
			functions removeChild() and findChild(). init() must be called
			before adding any nodes!
		---------------------------*/
		protected $nodes = array();
		protected $avl = null;

		protected function init() {
			#echo "init()";
			$this->nodes = array();
			$this->avl = new AVLTree();
		}

		abstract protected function makeChild($args); // overwriteable child generator; allows to define child type (eg.: admin classes that inherit from base class)

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
			$n->setIdx($tmp);
			$this->nodes[] = $n;
			if($this->avl != null) {
				$this->avl->insert($n);
			}
			return $tmp;
		}

		#function drawTree() {
		#	$this->avl->inorder();
		#}

		function removeChild($id) {
			if($this->isEmpty()) {
				return null;
			}

			if($this->avl == null) {
				return false;
			}
			$n = $this->avl->remove($id);

			#echo var_export($n,true);
			if($n != null) {
				if($n->getIdx() != null) {
					unset($this->nodes[$n->getIdx()]);
				}

				return $n;
			}

			return null;
		}

		final function getChildByID($id) {
			if($this->isEmpty()) {
				return null;
			}

			if($this->avl == null) {
				return false;
			}

			#$this->avl->inorder();

			return $this->avl->find($id);
		}

		final function getChildByIdx($idx) {
			if($this->isEmpty()) {
				return null;
			}
			return $this->nodes[$idx];
		}
	}
?>