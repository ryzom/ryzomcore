<?php

error_reporting(E_ALL ^ E_NOTICE);
ini_set("display_errors","1");

define('APP_NAME', 'app_achievements_admin');

require_once('../webig/config.php');
include_once('../webig/lang.php');
include_once('lang.php');
require_once('conf.php');

// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate($user, false);

$user = array();
$user['id'] = 1;
$user['lang'] = 'en';
$user['name'] = 'Talvela';
$user['race'] = "r_matis";
$user['civilization'] = "c_neutral";
$user['cult'] = "c_neutral";
$user['admin'] = true;

require_once($_CONF['app_achievements_path']."class/RyzomUser_class.php");
require_once("class/RyzomAdmin_class.php");
$_USER = new RyzomAdmin($user);

require_once("include/ach_render_admin.php");
#require_once("include/ach_render_csr.php");

require_once($_CONF['app_achievements_path']."class/RenderNodeIterator_abstract.php");
require_once($_CONF['app_achievements_path']."class/NodeIterator_class.php");
require_once($_CONF['app_achievements_path']."class/AchList_abstract.php");
require_once($_CONF['app_achievements_path']."class/Tieable_inter.php");

require_once($_CONF['app_achievements_path']."class/AchMenu_class.php");
require_once($_CONF['app_achievements_path']."class/AchMenuNode_class.php");
#require_once($_CONF['app_achievements_path']."class/AchCategory_class.php");
#require_once($_CONF['app_achievements_path']."class/AchAchievement_class.php");
#require_once($_CONF['app_achievements_path']."class/AchPerk_class.php");
#require_once($_CONF['app_achievements_path']."class/AchObjective_class.php");

require_once("class/ADM_inter.php");
require_once("class/AdmDispatcher_inter.php");
require_once("class/AdmMenu_class.php");
require_once("class/AdmMenuNode_class.php");
#require_once("class/AdmCategory_class.php");
#require_once("class/AdmAchievement_class.php");
#require_once("class/AdmPerk_class.php");
#require_once("class/AdmObjective_class.php");

/*require_once("class/CSR_inter.php");
require_once("class/CSRMenu_class.php");
require_once("class/CSRCategory_class.php");
require_once("class/CSRAchievement_class.php");
require_once("class/CSRPerk_class.php");
require_once("class/CSRObjective_class.php");*/

if($_USER->isIG()) {
	die("IG disabled for admin tool!");
}

$DBc = ryDB::getInstance("ahufler");

function mkn($x) {
	if($x == null || strtolower($x) == "null") {
		return "NULL";
	}
	else {
		return "'".mysql_real_escape_string($x)."'";
	}
}


$c = "<script type='text/javascript'>
		<!--
		function hs(id,mod) {
			if(document.getElementById(id).style.display == 'none') {
				document.getElementById(id).style.display=mod;
			}
			else {
				document.getElementById(id).style.display='none';
			}
		}

		function hs_force(id,mod,show) {
			if(show == true) {
				document.getElementById(id).style.display=mod;
			}
			else {
				document.getElementById(id).style.display='none';
			}
		}
</script>

<center><table width='100%'>
	<tr>
		<td valign='top' width='230px'><div style='font-weight:bold;font-size:14px;'>";
		
		if($_USER->isAdmin()) {
			$c .= "<b>Admin</b><br>
			<ul>
				<li><a href='?mode=menu'>menu settings</a></li>
				<li><a href='?mode=ach'>achievement settings</a></li>
			</ul><p />";
		}
		if($_USER->isCSR()) {
			$c .= "<b>CSR</b><br>
			<ul>
				<li><a href='?mode=player'>administrate player</a></li>
			</ul><p />";
		}

		
#$c .= ach_render_menu();
		
$c .= "</div></td>
		<td valign='top'>";

		if($_REQUEST['mode'] == "menu" && $_USER->isAdmin()) {
			$menu = new AdmMenu(false);
			
			if($_REQUEST['act'] == "insert") {
				$n = new AdmMenuNode(array(),null);
				$n->setID(null);
				$n->setDev(1);
				$n->setName($_REQUEST['acl_name']);
				$n->setImage($_REQUEST['ac_image']);
				$n->setParentID($_REQUEST['ac_parent']);

				$menu->insertNode($n);
			}

			if($_REQUEST['act'] == "delete") {
				$menu->removeNode($_REQUEST['ac_id']);
			}

			if($_REQUEST['act'] == "update") {
				$menu->updateNode($_REQUEST['ac_id'],array("acl_name"=>$_REQUEST['acl_name'],"ac_image"=>$_REQUEST['ac_image']));
			}

			if($_REQUEST['act'] == "dev") {
				$curr = $menu->getNode($_REQUEST['ac_id']);
				$curr->setInDev(($_REQUEST['state'] != 1));
			}


			$c .= adm_render_menu($menu);
		}

#$c .= ach_render_content();

$c .= "</td>
	</tr>
</table></center>";


echo ryzom_app_render("achievements admin", $c, $_USER->isIG());

?>
