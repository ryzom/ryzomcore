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

		final function setChildDone($idx) {
			$this->addChildDone($idx);
			$this->removeChildOpen($idx);
		}

		final function addChildDone($idx) {
			echo "try adding done child: ".$idx;
			if(!in_array($idx,$this->child_done)) {
				$this->child_done[] = $idx;
				echo " ... done<br>";
			}
			echo var_export($this->child_done,true);
		}

		final function removeChildDone($idx) {
			echo "try removing done child: ".$idx;
			#$res = array_search($idx,$this->child_done);
			#if($res != false) {
			#	unset($this->child_done[$res]);
			#	echo " ... done<br>";
			#}
			foreach($this->child_done as $key=>$elem) {
				if($elem == $idx) {
					unset($this->child_done[$key]);
					echo " ... done<br>";
					break;
				}
			}
			echo var_export($this->child_done,true);
		}

		final function setChildOpen($idx) {
			$this->addChildOpen($idx);
			$this->removeChildDone($idx);
		}

		final function addChildOpen($idx) {
			echo "try adding open child: ".$idx;
			if(!in_array($idx,$this->child_open)) {
				$this->child_open[] = $idx;
				echo " ... done<br>";
			}
			echo var_export($this->child_open,true);
		}

		final function removeChildOpen($idx) {
			echo "try removing open child: ".$idx;
			
			#$res = array_search($idx,$this->child_open);
			#if($res != false) {
			#	unset($this->child_open[$res]);
			#	echo " ... done<br>";
			#}
			foreach($this->child_open as $key=>$elem) {
				if($elem == $idx) {
					unset($this->child_open[$key]);
					echo " ... done<br>";
					break;
				}
			}
			echo var_export($this->child_open,true);
		}

		/*final function unsetOpen($idx) {
			foreach($this->child_open as $key=>$elem) {
				if($elem == $idx) {
					unset($this->child_open[$key]);
					break;
				}
			}
		}

		final function unsetDone($idx) {
			foreach($this->child_done as $key=>$elem) {
				if($elem == $idx) {
					unset($this->child_done[$key]);
					break;
				}
			}
		}*/
		
		#OVERRIDE Parentum::removeChild()
		function removeChild($id) {
			$n = parent::removeChild($id);
			if($n != false && $n != null) {
				unset($this->child_open[$n->getIdx()]);
				unset($this->child_done[$n->getIdx()]);
			}
			return $n;
		}
	}
?>