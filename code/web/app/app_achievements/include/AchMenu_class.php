<?php
	class AchMenu extends RenderNodeIterator {
		function AchMenu($open = false,$lang = 'en') {
			global $db;

			$res = $db->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$lang."' AND acl_category=ac_id) WHERE ac_parent IS NULL");
			#MISSING: ORDER by
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchMenuNode($res[$i],$open,$lang);
			}
		}
	}

	class AchMenuNode extends RenderNodeIterator {
		private $id = false;
		private $parent = false;
		private $name = null;
		private $open = false;

		function AchMenuNode(&$data,$open,$lang) {
			global $db;

			$this->id = $data['ac_id'];
			$this->parent = $data['ac_parent'];
			$this->name = $data['acl_name'];

			$this->open = ($open==$data['ac_id']);

			$res = $db->sqlQuery("SELECT * FROM ach_category LEFT JOIN (ach_category_lang) ON (acl_lang='".$lang."' AND acl_category=ac_id) WHERE ac_parent='".$this->id."'");
			#MISSING: ORDER by
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
	}
?>