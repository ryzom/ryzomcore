<?php
	class AchMenu extends RenderNodeIterator {
		var $open;

		function AchMenu($open = false,$lang = 'en') {
			global $db;

			$this->open = $open;

			$res = $db->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$lang."' AND acl_category=ac_id) WHERE ac_parent IS NULL ORDER by ac_order ASC, acl_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchMenuNode($res[$i],$open,$lang);
			}
		}

		function getCat() {
			return $this->open;
		}
	}

	class AchMenuNode extends RenderNodeIterator {
		private $id;
		private $parent;
		private $name;
		private $open;
		private $image;
		private $order;

		function AchMenuNode(&$data,$open,$lang) {
			global $db;

			$this->id = $data['ac_id'];
			$this->parent = $data['ac_parent'];
			$this->name = $data['acl_name'];
			$this->image = $data['ac_image'];
			$this->order = $data['ac_order'];
			$this->open = ($open==$data['ac_id']);

			$res = $db->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$lang."' AND acl_category=ac_id) WHERE ac_parent='".$this->id."' ORDER by ac_order ASC, acl_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchMenuNode($res[$i],$open,$lang);
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