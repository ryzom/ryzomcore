<?php
	class XMLNode {
		private $name;
		private $value = null;
		private $args = array();
		private $children = array();
		private $parent = null;

		function XMLNode($n = null,$v = null,$p = null) {
			if(is_numeric($n)) {
				$this->name = "_".$n;
			}
			else {
				$this->name = $n;
			}
			$this->value = htmlspecialchars($v);
			$this->parent = $p;
		}

		function getParent() {
			return $this->parent;
		}

		function getName() {
			return $this->name;
		}

		function setName($n) {
			if(is_numeric($n)) {
				$this->name = "_".$n;
			}
			else {
				$this->name = $n;
			}
		}

		function setValue($v) {
			$this->value = htmlspecialchars($v);
		}

		function addArg($k,$v) {
			$this->args[$k] = $v;
		}

		function getArg($k) {
			return $this->args[$k];
		}

		function clearArg($k) {
			unset($this->args[$k]);
		}

		function addChild($c) {
			$this->children[] = $c;
		}

		function generate($indent) {
			$xml = "";
			#for($i=0;$i<$indent;$i++) {
				$xml .= $indent;
			#}
			$xml .= "<".strtolower($this->name);
			foreach($this->args as $key=>$elem) {
				$xml .= ' '.strtolower($key).'="'.$elem.'"';
			}

			if(sizeof($this->children) > 0) {
				$xml .= ">\n";
				foreach($this->children as $elem) {
					$xml .= $elem->generate($indent.'	');
				}
				#for($i=0;$i<$indet;$i++) {
					$xml .= $indent;
				#}
				$xml .= "</".strtolower($this->name).">\n";
			}
			elseif($this->value !== null) {
				$xml .= ">".$this->value."</".strtolower($this->name).">\n";
			}
			else {
				$xml .= " />\n";
			}

			return $xml;
		}
	}
?>