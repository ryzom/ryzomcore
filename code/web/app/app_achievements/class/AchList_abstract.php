<?php
	abstract class AchList extends Parentum {
		protected $child_done = array();
		protected $child_open = array();

		final function getDone() {
			return new NodeIterator($this->child_done);
		}

		final function getOpen() {
			return new NodeIterator($this->child_open);
		}

		final function hasOpen() {
			return (sizeof($this->child_open) != 0);
		}

		final function hasDone() {
			return (sizeof($this->child_done) != 0);
		}

		final function addOpen($n) {
			$this->child_open[] = $this->addChild($n);
		}

		final function addDone($n) {
			$this->child_done[] = $this->addChild($n);
		}

		function removeChild() {
			$idx = $this->getIdx($id);
			if($idx != null) {
				unset($this->child_open[$idx]);
				unset($this->child_done[$idx]);
				parent::removeIdx($idx);
			}
		}
	}
?>