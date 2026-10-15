<?php
	class AVLTree {
		/*---------------------------
			AVL trees are balanced B-Trees. Please refer to http://en.wikipedia.org/wiki/AVL_tree

			This implementation allows the functions insert, find and remove. Please
			note, that remove does not rebalance the tree, since node removal is
			quite rare in this project.
		---------------------------*/

		private $root;
		private $debug;
		
		function AVLTree($log = false) {
			$this->root = null;
			$this->debug = $log;
		}

		function preorder() {
			$this->AVLpreorder($this->root);
		}

		private function AVLpreorder($p) { // recursive; output preorder representation
			if($p != null) {
				echo $p->getID().", ";
				$this->AVLpreorder($p->getLeft());
				$this->AVLpreorder($p->getRight());
			}
		}

		function inorder() {
			$this->AVLinorder($this->root);
		}

		private function AVLinorder($p) { // recursive; output postorder representation
			if($p != null) {
				$this->AVLinorder($p->getLeft());
				echo $p->getID().", ";
				$this->AVLinorder($p->getRight());
			}
		}

		function insert($node) { // insert a new node
			if($this->root == null) {
				$this->root = new AVLTreeNode($node);
			}
			else {
				$this->root = $this->AVLinsert($this->root,new AVLTreeNode($node));
			}
		}

		function remove($id) { // remove a node
			$n = $this->AVLfind($id,$this->root);

			if($n != null) {
				$this->AVLremove($this->root,$n);
				return $n->getNode();
			}
			return null;
		}

		function find($id) { // find a node
			$res = $this->AVLfind($id,$this->root);
			if($res != null) {
				return $res->getNode();
			}
			return null;
		}

		private function AVLfind($id,$n) { // recursive; search for a node
			if($n != null) {
				if($n->getID() != $id) {
					if($n->getID() > $id) {
						$n = $this->AVLfind($id,$n->getLeft());
					}
					else {
						$n = $this->AVLfind($id,$n->getRight());
					}
				}
			}

			return $n;
		}

		private function AVLremove($r,$n) { // remove a node from the actual tree
			if($n->getLeft() == null || $n->getRight() == null) {
				$s = $n;
			}
			else {
				$s = $this->Successor($n);
				$n->setNode($r->getNode());
			}

			if($r->getLeft() != null) {
				$p = $s->getLeft();
			}
			else {
				$p = $s->getRight();
			}

			if($p != null) {
				$p->setParent($s->getParent());
			}

			if($r->getParent() == null) {
				$r = $p;
			}
			else {
				$tmp = $s->getParent();
				if($s == $tmp->getLeft()) {
					$tmp->setLeft($p);
				}
				else {
					$tmp->setRight($p);
				}
			}

			return $r;
		}

		private function AVLinsert($r,$n) { // insert a node into the actual tree
			if($r == null) {
				$r = $n;
			}
			else {
				if($n->getID() < $r->getID()) {
					$r->setLeft($this->AVLinsert($r->getLeft(),$n));
					
					$r = $this->balance($r); // rebalance
				}
				elseif($n->getID() > $r->getID()) {
					$r->setRight($this->AVLinsert($r->getRight(),$n));
					
					$r = $this->balance($r); // rebalance
				}

				$r->setHeight(max($r->getHeightLeft(),$r->getHeightRight())+1);
			}

			return $r;
		}

		private function balance($r) { // do a rebalancation of the tree
			if($r->bal() == -2) {
				$lc = $r->getLeft();
				if($lc->getHeightLeft() >= $lc->getHeightRight()) {
					$r = $this->RotateToRight($r);
				}
				else {
					$r = $this->DoubleRotateLeftRight($r);
				}
			}

			if($r->bal() == 2) {
				$rc = $r->getRight();
				if($rc->getHeightRight() >= $rc->getHeightLeft()) {
					$r = $this->RotateToLeft($r);
				}
				else {
					$r = $this->DoubleRotateRightLeft($r);
				}
			}

			return $r;
		}

		private function Successor($r) { // find the successor for a node
			if($r->getRight() != null) {
				return $this->Minimum($r->getRight());
			}
			else {
				$n = $r->getParent();

				while($n != null && $r == $n->getRight()) {
					$r = $n;
					$n = $n->getParent();
				}
				return $n;
			}
		}

		private function Minimum($r) { // find the minimum of a tree
			if($r == null) {
				return null;
			}

			if($r->getLeft() == null) {
				return $r;
			}
			
			$p = $r;
			while($p->getLeft() != null) {
				$p = $p->getLeft();
			}

			return $p;
		}

		//rotations

		private function RotateToRight($r) {
			if($this->debug) {
				echo "rotaRight<br>";
			}
			$v = $r->getLeft();
			$r->setLeft($v->getRight());
			$v->setRight($r);

			$r->setHeight(max($r->getHeightLeft(),$r->getHeightRight())+1);
			$v->setHeight(max($v->getHeightLeft(),$r->getHeight())+1);

			return $v;
		}

		private function DoubleRotateLeftRight($r) {
			$r->setLeft($this->RotateToLeft($r->getLeft()));
			return $this->RotateToRight($r);
		}

		private function RotateToLeft($r) {
			if($this->debug) {
				echo "rotaLeft<br>";
			}
			$v = $r->getRight();
			$r->setRight($v->getLeft());
			$v->setLeft($r);

			$r->setHeight(max($r->getHeightLeft(),$r->getHeightRight())+1);
			$v->setHeight(max($v->getHeightRight(),$r->getHeight())+1);

			return $v;
		}

		private function DoubleRotateRightLeft($r) {
			$r->setRight($this->RotateToRight($r->getRight()));
			return $this->RotateToLeft($r);
		}

		//end rotations
	}

	/*
	 * AVL tree nodes
	 */

	class AVLTreeNode {
			private $height;
			private $left;
			private $right;
			private $node;
			private $parent;

			function AVLTreeNode($node) {
				$this->height = 0;
				$this->left = null; // left child
				$this->right = null; // right child
				$this->node = $node; // actual data stored
				$this->parent = null; // parent node
			}

			function getParent() {
				return $this->parent;
			}

			function setParent($p) {
				$this->parent = $p;
			}

			function getNode() {
				return $this->node;
			}

			function getLeft() {
				return $this->left;
			}

			function getRight() {
				return $this->right;
			}

			function getHeightLeft() {
				if($this->left == null) {
					return -1;
				}
				return $this->left->getHeight();
			}

			function getHeightRight() {
				if($this->right == null) {
					return -1;
				}
				return $this->right->getHeight();
			}

			function bal() { // calculate value to eval balancing
				$r = -1;
				$l = -1;

				if($this->right != null) {
					$r = $this->right->getHeight();
				}
				
				if($this->left != null) {
					$l = $this->left->getHeight();
				}
				
				return ($r-$l);
			}

			function getID() {
				return $this->node->getID();
			}

			function getHeight() {
				return $this->height;
			}

			function setHeight($h) {
				$this->height = $h;
			}

			function setRight($r) {
				$this->right = $r;
				if($r != null) {
					$r->setParent($this);
				}
			}

			function setLeft($l) {
				$this->left = $l;
				if($l != null) {
					$l->setParent($this);
				}
			}

			function setNode($n) {
				$this->node = $n;
			}
		}
?>