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
$_ADMIN = new RyzomAdmin($user);

if($_ADMIN->isIG()) {
	die("IG disabled for admin tool!");
}

require_once("class/mySQL_class.php");

#require_once("include/ach_render_admin.php");
#require_once("include/ach_render_csr.php");
require_once($_CONF['app_achievements_path']."include/ach_render_common.php");

require_once($_CONF['app_achievements_path']."class/AVLTree_class.php");
require_once($_CONF['app_achievements_path']."class/Parentum_abstract.php");
require_once($_CONF['app_achievements_path']."class/AchList_abstract.php");
require_once($_CONF['app_achievements_path']."class/Tieable_inter.php");
require_once($_CONF['app_achievements_path']."class/NodeIterator_class.php");
require_once($_CONF['app_achievements_path']."class/Node_trait.php");
require_once($_CONF['app_achievements_path']."class/InDev_trait.php");

require_once($_CONF['app_achievements_path']."class/AchMenu_class.php");
require_once($_CONF['app_achievements_path']."class/AchMenuNode_class.php");
require_once($_CONF['app_achievements_path']."class/AchCategory_class.php");
require_once($_CONF['app_achievements_path']."class/AchAchievement_class.php");
require_once($_CONF['app_achievements_path']."class/AchPerk_class.php");
require_once($_CONF['app_achievements_path']."class/AchObjective_class.php");

require_once("class/ADM_inter.php");
require_once("class/AdmDispatcher_trait.php");
require_once("class/AdmMenu_class.php");
require_once("class/AdmMenuNode_class.php");
require_once("class/AdmCategory_class.php");
require_once("class/AdmAchievement_class.php");
require_once("class/AdmPerk_class.php");
require_once("class/AdmObjective_class.php");
require_once("class/AdmAtom_class.php");

require_once("class/CSRDispatcher_trait.php");
require_once("class/CSR_inter.php");
#require_once("class/CSRMenu_class.php");
require_once("class/CSRCategory_class.php");
require_once("class/CSRAchievement_class.php");
require_once("class/CSRPerk_class.php");
require_once("class/CSRObjective_class.php");
require_once("class/CSRAtom_class.php");

$DBc = ryDB::getInstance("app_achievements");
#$DBc = ryDB::getInstance("ahufler");

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

<style>
	h1 {
		margin-top:0px;
	}
</style>

<center><table width='100%'>
	<tr>
		<td valign='top' width='200px'><div style='font-weight:bold;font-size:14px;'>";
		
		if($_ADMIN->isAdmin()) {
			$c .= "<b>Admin</b><br>
			<ul>
				<li><a href='?mode=menu'>menu settings</a></li>
				<li><a href='?mode=ach'>achievement settings</a></li>
			</ul><p />";
		}
		if($_ADMIN->isCSR()) {
			$c .= "<b>CSR</b><br>
			<ul>
				<li><a href='?mode=player'>player administration</a></li>
			</ul><p />";
		}

		
#$c .= ach_render_menu();
		
$c .= "</div></td>
		<td valign='top'>";

		if($_REQUEST['mode'] == "menu" && $_ADMIN->isAdmin()) {
			$c .= "<h1>Menu Settings</h1>";

			$user = array();
			$user['id'] = 1;
			$user['lang'] = 'en';
			$user['name'] = 'Talvela';
			$user['race'] = "r_matis";
			$user['civilization'] = "c_neutral";
			$user['cult'] = "c_neutral";

			$_USER = new RyzomUser($user);

			require_once("include/adm_render_menu.php");
			$menu = new AdmMenu(false);
			
			if($_REQUEST['act'] == "insert") {
				$n = new AdmMenuNode(array(),null);
				$n->setID(null);
				$n->setInDev(true);
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

		if($_REQUEST['mode'] == "ach" && $_ADMIN->isAdmin()) {
			$c .= "<h1>Achievement Settings</h1>";

			$user = array();
			$user['id'] = 0;
			$user['lang'] = 'en';
			$user['name'] = 'Talvela';
			$user['race'] = "r_matis";
			$user['civilization'] = "c_neutral";
			$user['cult'] = "c_neutral";

			$_USER = new RyzomUser($user);

			//menu
			require_once("include/adm_render_ach.php");
			$menu = new AdmMenu($_REQUEST['cat']);

			$c .= "<center><table>
			<tr>
				<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
			

			$c .= adm_render_menu($menu);
				
			$c .= "</div></td>
					<td width='645px' valign='top'>";
			
			$open = $menu->getOpenCat();

			if($open != 0) {
				$cat = new AdmCategory($open,$_REQUEST['cult'],$_REQUEST['civ']);

				if($_REQUEST['act'] == "ach_move") {
					$ach = $cat->getChildByID($_REQUEST['id']);
					if($ach != null) {
						$ach->setCategory($_REQUEST['new_cat']);
						$ach->update();
						$cat->removeChild($ach->getID());
					}
				}

				if($_REQUEST['act'] == "ach_insert") {
					$ach = new AdmAchievement(array(),$cat);
					$ach->setCategory($cat->getID());
					$ach->setName($_REQUEST['aal_name']);
					$ach->setTemplate($_REQUEST['aal_template']);
					$ach->setTieCult($_REQUEST['aa_tie_cult']);
					$ach->setTieCiv($_REQUEST['aa_tie_civ']);
					$ach->setImage($_REQUEST['aa_image']);
					
					$cat->insertNode($ach);

					$perk = new AdmPerk(array(),$ach);
					$perk->setAchievement($ach->getID());
					$perk->setName($_REQUEST['apl_name']);
					$perk->setTemplate($_REQUEST['apl_name']);
					$perk->setValue($_REQUEST['ap_value']);

					$ach->insertNode($perk);
				}

				if($_REQUEST['act'] == "ach_update") {
					$ach = $cat->getChildByID($_REQUEST['id']);
					
					if($ach != null) {
						$ach->setName($_REQUEST['aal_name']);
						$ach->setTemplate($_REQUEST['aal_template']);
						$ach->setTieCult($_REQUEST['aa_tie_cult']);
						$ach->setTieCiv($_REQUEST['aa_tie_civ']);
						$ach->setImage($_REQUEST['aa_image']);

						$ach->update();
					}
				}

				if($_REQUEST['act'] == "perk_insert") {
					$ach = $cat->getChildByID($_REQUEST['id']);
					if($ach != null) {
						$perk = new AdmPerk(array(),$ach);
						$perk->setAchievement($ach->getID());
						$perk->setName($_REQUEST['apl_name']);
						$perk->setTemplate($_REQUEST['apl_name']);
						$perk->setValue($_REQUEST['ap_value']);
						#MISSING: parent
						#$perk->setCondition($_REQUEST['ap_condition']);
						#$perk->setConditionValue($_REQUEST['ap_condition_value']);

						$ach->insertNode($perk);
					}
				}

				if($_REQUEST['act'] == "perk_update") {
					$perk = $cat->getElementByPath($_REQUEST['id']);
					
					if($perk != null) {
						$perk->setName($_REQUEST['apl_name']);
						$perk->setTemplate($_REQUEST['apl_template']);
						$perk->setValue($_REQUEST['ap_value']);
						#$perk->setParentID($_REQUEST['ap_parent']);
						$perk->setCondition($_REQUEST['ap_condition']);
						$perk->setConditionValue($_REQUEST['ap_condition_value']);

						$perk->update();
					}
				}

				if($_REQUEST['act'] == "obj_insert") {
					$perk = $cat->getElementByPath($_REQUEST['id']);
					
					if($perk != null) {
						$obj = new AdmObjective(array(),$perk);
						$obj->setName($_REQUEST['aol_name']);
						$obj->setCondition($_REQUEST['ao_condition']);
						$obj->setValue($_REQUEST['ao_value']);
						$obj->setDisplay($_REQUEST['ao_display']);
						$obj->setMetalink($_REQUEST['ao_metalink']);

						$perk->insertNode($obj);
					}
				}

				if($_REQUEST['act'] == "obj_update") {
					$obj = $cat->getElementByPath($_REQUEST['id']);
					
					if($obj != null) {
						$obj->setName($_REQUEST['aol_name']);
						$obj->setCondition($_REQUEST['ao_condition']);
						$obj->setValue($_REQUEST['ao_value']);
						$obj->setDisplay($_REQUEST['ao_display']);
						$obj->setMetalink($_REQUEST['ao_metalink']);

						$obj->update();
					}
				}

				if($_REQUEST['act'] == "delete") {
					$elem = $cat->getElementByPath($_REQUEST['id']);
					$par = $elem->getParent();
					$par->removeNode($elem->getID());
				}

				if($_REQUEST['act'] == "dev") {
					$curr = $cat->getElementByPath($_REQUEST['id']);
					$curr->setInDev(($_REQUEST['state'] != 1));
				}
				
				$c .= adm_render_category($cat);
			}

			#a:p:o:a

			

			$c .= "</td>
				</tr>
			</table></center>";

				//category
		}

		if($_REQUEST['mode'] == "player" && $_ADMIN->isCSR()) {
			$c .= "<h1>Player Administration</h1>";

			$DBc_char = new mySQL($_CONF['mysql_error']);
			$DBc_char->connect($_CONF['char_mysql_server'],$_CONF['char_mysql_user'],$_CONF['char_mysql_pass'],$_CONF['char_mysql_database']);
			//menu
			require_once("include/adm_render_csr.php");

			if(!is_user($_REQUEST['pid'])) { // no user ID
				$c .= csr_render_find_player();
			}
			else {
				$user = array();
				$user['id'] = $_REQUEST['pid'];
				$user['lang'] = 'en';
				$user['name'] = 'Talvela';
				$user['race'] = "r_matis";
				$user['civilization'] = "c_neutral";
				$user['cult'] = "c_neutral";

				$_USER = new RyzomUser($user);

				$menu = new AchMenu($_REQUEST['cat']);

				$open = $menu->getOpenCat();

				if($open != 0) {
					$cat = new CSRCategory($open,$_REQUEST['cult'],$_REQUEST['civ']);

					if($_REQUEST['grant'] != "") {
						$cat->grantNode($_REQUEST['grant'],$_USER->getID());
					}

					if($_REQUEST['deny'] != "") {
						$cat->denyNode($_REQUEST['deny'],$_USER->getID());
					}
				}

				$c .= "<center><table>
				<tr>
				<td colspan='2' align='left'>".csr_render_yubopoints($user['id'])."</td>
			</tr>
				<tr>
					<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
				

				$c .= csr_render_menu($menu);
					
				$c .= "</div></td>
						<td width='645px' valign='top'>";
				
				$open = $menu->getOpenCat();

				if($open != 0) {
					$c .= csr_render_category($cat);
				}

				$c .= "</td>
					</tr>
				</table></center>";
			}

		}

#$c .= ach_render_content();

$c .= "</td>
	</tr>
</table></center>";


echo ryzom_app_render("achievements admin", $c, $_ADMIN->isIG());

?>
