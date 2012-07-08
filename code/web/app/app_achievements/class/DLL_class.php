<?php
	class DLL {
		private $first;
		private $last;
		private $size;
		private $avl;

		function DLL() {
			$this->avl = new AVLTree();

			$this->first = null;
			$this->last = null;

			$this->size = 0;
		}

		function getIterator() {
			return new NodeIterator($this->first);
		}

		final function getSize() {
			return $this->size;
		}

		final function isEmpty() {
			return ($this->size == 0);
		}

		function getFirst() {
			return $this->first;
		}

		function getLast() {
			return $this->last;
		}

		function addNode($data,$before = null) {
			if($this->findNode($data->getID()) != null) {
				return false;	
			}
			$n = new DLLnode($data);
			if($before == null) {
				//insert as last
				if($this->last != null) {
					$this->last->setChild($n);
				}
				$n->setParent($this->last);
				$this->last = $n;
			}
			else {
				//insert before
				$b = $this->findNode($before);
				if($b != null) {
					if($b == $this->first) {
						$this->first = $n;
					}
					$tmp = $b->getParent();
					$b->setParent($n);
					$n->setChild($b);
					if($tmp != null) {
						$tmp->setChild($n);
					}
				}
			}

			if($this->first == null) {
				$this->first = $n;
			}

			$this->avl->insert($n);
			$this->size++;
			
			#$this->avl->inorder();
			#echo "<br>";
		}

		function removeNode($id) {
			echo "rid: ".$id."<br>";
			$this->avl->inorder();
			echo "<br>";

			$n = $this->findNode($id);
			if($n != null) {
				echo "removed; ";
				$p = $n->getParent();
				$c = $n->getChild();
				
				if($c != null) {
					$c->setParent($p);
					if($p != null) {
						$p->setChild($c);
					}
				}
				else {
					if($p != null) {
						$p->setChild(null);
					}
					$this->last = $p;
				}

				if($p == null) {
					if($c != null) {
						$c->setParent(null);
						$this->first = $c;
					}
					else {
						$this->first = null;
					}
				}

				$this->avl->remove($id);
				$this->size--;
			}

			$this->avl->inorder();
			echo "<br>";
		}

		function findNode($id) {
			return $this->avl->find($id);
		}

		/*function storeOrder() {
			$iter = $this->getIterator();
			$i = 0;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->StoreOrder($i);
				$i++;
			}
		}*/
	}

	class DLLnode {
		private $parent;
		private $child;
		public $data;

		function DLLNode($d) {
			$this->parent = null;
			$this->child = null;
			$this->data = $d;
		}

		final function getParent() {
			return $this->parent;
		}

		final function setParent($p) {
			$this->parent = $p;
		}

		final function getChild() {
			return $this->child;
		}

		final function setChild($c) {
			$this->child = $c;
		}

		final function getIterator() {
			return new NodeIterator($this);
		}

		#function ListStoreOrder($i) {
		#	$this->data->setListOrder($i);
		#	$this->data->update();
		#}

		function getID() {
			return $this->data->getID();
		}
	}
?>