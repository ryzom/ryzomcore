<?php
	header('Content-type: text/xml');

	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	require_once("class/mySQL_class.php");
	require_once("conf.php");

	require_once($_CONF['app_achievements_path']."class/Parentum_abstract.php");
	require_once($_CONF['app_achievements_path']."class/AchList_abstract.php");
	require_once($_CONF['app_achievements_path']."class/Tieable_inter.php");
	require_once($_CONF['app_achievements_path']."class/NodeIterator_class.php");
	#require_once($_CONF['app_achievements_path']."class/Node_trait.php");
	#require_once($_CONF['app_achievements_path']."class/InDev_trait.php");

	require_once($_CONF['app_achievements_path']."class/AchMenu_class.php");
	require_once($_CONF['app_achievements_path']."class/AchMenuNode_class.php");
	require_once($_CONF['app_achievements_path']."class/AchSummary_class.php");
	require_once($_CONF['app_achievements_path']."class/AchCategory_class.php");
	require_once($_CONF['app_achievements_path']."class/AchAchievement_class.php");
	require_once($_CONF['app_achievements_path']."class/AchTask_class.php");
	require_once($_CONF['app_achievements_path']."class/AchObjective_class.php");

	class RUser {
		function RUser() { }

		function getLang() {
			return $_REQUEST['lang'];
		}

		function getID() {
			return 0;
		}

		function getCult() {
			return "%";
		}

		function getCiv() {
			return "%";
		}

		function getRace() {
			return "%";
		}
	}

	$_USER = new RUser();


	//create database connection
	$DBc = new mySQL($_CONF['mysql_error']);
	$DBc->connect($_CONF['mysql_server'],$_CONF['mysql_user'],$_CONF['mysql_pass'],$_CONF['mysql_database']);

	echo '<?xml version="1.0" ?><ryzom_achievements>';

	function print_cat(&$iter3) {
		while($iter3->hasNext()) {
			$curr3 = $iter3->getNext();
			echo "<achievement id='".$curr3->getID()."' parent='".$curr3->getParentID()."' image='".$_CONF['image_url']."pic/icon/".$curr3->getImage()."'><name><![CDATA[".$curr3->getName()."]]></name><ties>";
			if($curr3->getTieRace() != null) {
				echo "<tie type='race'>".$curr3->getTieRace()."</tie>";
			}
			if($curr3->getTieCult() != null) {
				echo "<tie type='cult'>".$curr3->getTieCult()."</tie>";
			}
			if($curr3->getTieCiv() != null) {
				echo "<tie type='civilization'>".$curr3->getTieCiv()."</tie>";
			}
			echo "</ties>";
			$iter4 = $curr3->getIterator();
			while($iter4->hasNext()) {
				$curr4 = $iter4->getNext();
				echo "<task id='".$curr4->getID()."' parent='".$curr4->getParentID()."' value='".$curr4->getValue()."'><name><![CDATA[".$curr4->getDisplayName()."]]></name>";
				$iter5 = $curr4->getIterator();
				while($iter5->hasNext()) {
					$curr5 = $iter5->getNext();
					echo "<objective id='".$curr5->getID()."' type='".$curr5->getDisplay()."' value='".$curr5->getValue()."' meta='".$_CONF['image_url']."pic/icon/".$curr5->getMetaImage()."'><name><![CDATA[".$curr5->getDisplayName()."]]></name></objective>";
				}
				echo "</task>";
			}
			echo "</achievement>";
		}
	}

	$menu = new AchMenu(0);
	$menu->removeChild(0);

	$iter = $menu->getIterator();
	while($iter->hasNext()) {
		$curr = $iter->getNext();
		echo "<category id='".$curr->getID()."' order='".$curr->getOrder()."' image='".$_CONF['image_url']."pic/menu/".$curr->getImage()."'><name><![CDATA[".$curr->getName()."]]></name>";
		$iter2 = $curr->getIterator();
		while($iter2->hasNext()) {
			$curr2 = $iter2->getNext();
			echo "<category id='".$curr2->getID()."' order='".$curr2->getOrder()."' image='".$_CONF['image_url']."pic/menu/".$curr2->getImage()."'><name><![CDATA[".$curr2->getName()."]]></name>";
			$cat = new AchCategory($curr2->getID(),null,null);
			$iter3 = $cat->getIterator();
			print_cat($iter3);
			echo "</category>";
		}

		$cat = new AchCategory($curr->getID(),null,null);
		$iter3 = $cat->getIterator();
		print_cat($iter3);
		echo "</category>";
	}

	echo "</ryzom_achievements>";

	die();
?>
