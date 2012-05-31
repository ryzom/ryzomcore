<?php
	class AchMenu extends RenderNodeIterator {
		var $open;

		function AchMenu($open = false) {
			global $DBc,$_USER;

			$this->open = $open;
			
			$tmp = array();
			$tmp['ac_id'] = 0;
			$tmp['ac_parent'] = null;
			$tmp['acl_name'] = get_translation('ach_summary',$_USER->getLang());
			$tmp['ac_image'] = "";
			$tmp['ac_order'] = -1;
			$this->nodes[] = new AchMenuNode($tmp,$open,$lang);

			$res = $DBc->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$_USER->getLang()."' AND acl_category=ac_id) WHERE ac_parent IS NULL ORDER by ac_order ASC, acl_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchMenuNode($res[$i],$open);
			}
		}

		function getOpen() {
			return $this->open;
		}

		function getOpenCat() {
			foreach($this->nodes as $elem) {
				$res = $elem->hasOpenCat();
				if($res != 0) {
					return $res;
				}
			}
			return 0;
		}
	}

	class AchMenuNode extends RenderNodeIterator {
		private $id;
		private $parent;
		private $name;
		private $open;
		private $image;
		private $order;

		function AchMenuNode(&$data,$open) {
			global $DBc,$_USER;

			$this->id = $data['ac_id'];
			$this->parent = $data['ac_parent'];
			$this->name = $data['acl_name'];
			$this->image = $data['ac_image'];
			$this->order = $data['ac_order'];
			$this->open = ($this->id == $open);

			$res = $DBc->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$_USER->getLang()."' AND acl_category=ac_id) WHERE ac_parent='".$this->id."' ORDER by ac_order ASC, acl_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchMenuNode($res[$i],$open);
			}
		}

		function getID() {
			return $this->id;
		}

		function getName() {
			return $this->name;
		}

		function getParent() {
			return $this->parent;
		}

		function hasOpenCat() {
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
	}
?>