<?php
	class XMLfile {
		private $ident;
		private $xml;

		private $curr;

		function XMLfile($i) {
			$this->ident = $i;
			$this->xml = new XMLNode($this->ident);
			$this->curr = $this->xml;
		}

		function getIdent() {
			return $this->ident;
		}

		function addXML($name,$attrs,$open) {
			if($open == true) {
				if($name == "__KEY__") {
					$x = explode(".",$attrs["VALUE"]);
					if(sizeof($x) > 1) {
						$v = $x[1];
						$a = array("sheetid"=>$attrs["VALUE"]);
					}
					else {
						$v = $attrs["VALUE"];
						$a = array();
					}
					$this->curr = new XMLNode($v,null,$this->curr);
					foreach($a as $key=>$elem) {
						$this->curr->addArg($key,$elem);
					}
					$tmp = $this->curr->getParent();
					$tmp->addChild($this->curr);
				}
				elseif($name == "__VAL__") {
					$this->curr->setValue($attrs["VALUE"]);
				}
				else {
					$this->curr = new XMLNode($name,null,$this->curr);
					if(isset($attrs["VALUE"])) {
						$this->curr->addArg("value",$attrs["VALUE"]);
					}
					$tmp = $this->curr->getParent();
					$tmp->addChild($this->curr);
				}
			}
			else {
				if($name == "__KEY__") {
					// do nothing
				}
				elseif($name == "__VAL__") {
					$this->curr = $this->curr->getParent();
				}
				elseif($name == $this->curr->getName()) {
					if($this->curr->getArg("value") !== null) {
						$this->curr->setValue($this->curr->getArg("value"));
						$this->curr->clearArg("value");
					}
					$this->curr = $this->curr->getParent();
				}
				else {
					$this->curr = $this->curr->getParent();
				}
			}
		}

		function generate($i) {
			return $this->xml->generate($i);
		}

	}
?>