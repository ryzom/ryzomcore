<?php
	class AchMenuNode extends RenderNodeIterator {
		protected $id;
		protected $parent_id;
		protected $name;
		protected $open;
		protected $image;
		protected $order;
		protected $dev;

		function AchMenuNode(&$data) {
			global $DBc,$_USER;

			$this->id = $data['ac_id'];
			$this->parent_id = $data['ac_parent'];
			$this->name = $data['acl_name'];
			$this->image = $data['ac_image'];
			$this->order = $data['ac_order'];
			$this->open = ($this->id == $data['open']);
			$this->dev = $data['ac_dev'];

			$res = $DBc->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$_USER->getLang()."' AND acl_category=ac_id) WHERE ac_parent='".$this->id."' ORDER by ac_order ASC, acl_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$res[$i]['open'] = $data['open'];
				$this->nodes[] = $this->makeChild($res[$i]);
			}
		}

		protected function makeChild(&$a) {
			return new AchMenuNode($a);
		}

		function getID() {
			return $this->id;
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
			
			foreach($this->nodes as $elem) {
				$res = $elem->hasOpenCat();
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

		function inDev() { // check if dev flag is set
			return ($this->dev == 1);
		}

		function getDev() {
			return $this->dev;
		}
	}
?>