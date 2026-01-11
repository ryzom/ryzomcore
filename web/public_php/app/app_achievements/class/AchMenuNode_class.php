<?php
	class AchMenuNode extends Parentum {
		#########################
		# PHP 5.3 compatible
		# InDev_trait replaces this in PHP 5.4
		protected $dev;

		function inDev() {
			return ($this->dev == 1);
		}

		function getDev() {
			return $this->dev;
		}

		function setInDev($tf) {
			if($tf == true) {
				$this->setDev(1);
			}
			else {
				$this->setDev(0);
			}

			$this->update();
		}

		function setDev($d) {
			$this->dev = $d;
		}
		#########################

		protected $parent_id;
		protected $name;
		protected $open;
		protected $image;
		protected $order;

		function AchMenuNode($data,$parent) {
			global $DBc,$_USER,$_CONF;

			parent::__construct();

			$this->setParent($parent);
			$this->setID($data['ac_id']);
			$this->parent_id = $data['ac_parent'];
			$this->name = $data['acl_name'];
			$this->image = $data['ac_image'];
			$this->order = $data['ac_order'];
			$this->open = ($this->id == $data['open']);
			$this->dev = $data['ac_dev'];

			if($this->name == null) {
				$res = $DBc->sqlQuery("SELECT * FROM ach_category_lang WHERE acl_lang='".$_CONF['default_lang']."' AND acl_category='".$this->id."'");
				$this->name = $res[0]['acl_name'];
			}

			$res = $DBc->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$_USER->getLang()."' AND acl_category=ac_id) WHERE ac_parent='".$this->id."' ORDER by ac_order ASC, acl_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$res[$i]['open'] = $data['open'];
				$this->addChild($this->makeChild($res[$i]));
			}
		}
		
		#@override Parentum::makeChild()
		protected function makeChild($a) {
			return new AchMenuNode($a,$this);
		}

		function getName() {
			return $this->name;
		}

		function getParentID() {
			return $this->parent_id;
		}

		function hasOpenCat() { // finds the currently open MenuNode and returns it's ID. If not found the result will be 0 instead.
			if($this->open) {
				return $this->id;
			}
			
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$res = $curr->hasOpenCat();
				if($res != 0) {
					return $res;
				}
			}
			return 0;
		}

		function isOpen() {
			return $this->open;
		}

		function getImage() {
			return $this->image;
		}

		function getOrder() {
			return $this->order;
		}
	}
?>