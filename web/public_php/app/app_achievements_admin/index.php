<?php

$microstart = explode(' ',microtime());
$start_time = $microstart[0] + $microstart[1];

error_reporting(E_ALL ^ E_NOTICE);
ini_set("display_errors","1");

define('APP_NAME', 'app_achievements_admin');

require_once('../config.php');
include_once('../lang.php');
include_once('lang.php');
require_once('conf.php');

// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate($user, true);

#echo var_export($user,true);

/*$user = array();
$user['id'] = 1;
$user['lang'] = 'en';
$user['name'] = 'Talvela';
$user['race'] = "r_matis";
$user['civilization'] = "c_neutral";
$user['cult'] = "c_neutral";
$user['admin'] = true;*/

require_once($_CONF['app_achievements_path']."class/RyzomUser_class.php");
require_once("class/RyzomAdmin_class.php");
$_ADMIN = new RyzomAdmin($user);

if($_ADMIN->isIG()) {
	die("IG disabled for admin tool!");
}

require_once("class/mySQL_class.php");

require_once($_CONF['app_achievements_path']."include/ach_render_common.php");

require_once($_CONF['app_achievements_path']."class/DLL_class.php");
require_once($_CONF['app_achievements_path']."class/Node_abstract.php");
require_once($_CONF['app_achievements_path']."class/AVLTree_class.php");
require_once($_CONF['app_achievements_path']."class/Parentum_abstract.php");
require_once($_CONF['app_achievements_path']."class/AchList_abstract.php");
require_once($_CONF['app_achievements_path']."class/Tieable_inter.php");
require_once($_CONF['app_achievements_path']."class/NodeIterator_class.php");
#require_once($_CONF['app_achievements_path']."class/InDev_trait.php");

require_once($_CONF['app_achievements_path']."class/AchMenu_class.php");
require_once($_CONF['app_achievements_path']."class/AchMenuNode_class.php");
require_once($_CONF['app_achievements_path']."class/AchCategory_class.php");
require_once($_CONF['app_achievements_path']."class/AchAchievement_class.php");
require_once($_CONF['app_achievements_path']."class/AchTask_class.php");
require_once($_CONF['app_achievements_path']."class/AchObjective_class.php");
require_once($_CONF['app_achievements_path']."class/AchSummary_class.php");

require_once("class/ADM_inter.php");
#require_once("class/AdmDispatcher_trait.php");
require_once("class/AdmMenu_class.php");
require_once("class/AdmMenuNode_class.php");
require_once("class/AdmCategory_class.php");
require_once("class/AdmAchievement_class.php");
require_once("class/AdmTask_class.php");
require_once("class/AdmObjective_class.php");
require_once("class/AdmAtom_class.php");

#require_once("class/CSRDispatcher_trait.php");
require_once("class/CSR_inter.php");
require_once("class/CSRCategory_class.php");
require_once("class/CSRAchievement_class.php");
require_once("class/CSRTask_class.php");
require_once("class/CSRObjective_class.php");
require_once("class/CSRAtom_class.php");

$DBc = ryDB::getInstance("app_achievements");

function mkn($x) { // make NULL function for SQL
	global $DBc;
	if($x == null || strtolower($x) == "null" || $x == "") {
		return "NULL";
	}
	else {
		return "'".$DBc->sqlEscape($x)."'";
	}
}


$c = "<script type='text/javascript'>

		function hs(id,mod) { // hide / show
			if(document.getElementById(id).style.display == 'none') {
				document.getElementById(id).style.display=mod;
			}
			else {
				document.getElementById(id).style.display='none';
			}
		}

		function hs_force(id,mod,show) { // hide / show forced
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
				<li><a href='?mode=atom'>trigger settings</a></li>
				<li><a href='?mode=lang'>language editor</a></li>
				<li><a href='?mode=stats'>statistics</a></li>
			</ul><p />";
		}
		if($_ADMIN->isCSR()) {
			$c .= "<b>CSR</b><br>
			<ul>
				<li><a href='?mode=player'>player administration</a></li>
			</ul><p />";
		}


		
$c .= "</div></td>
		<td valign='top'>";

		/*if($_REQUEST['mode'] == "insert_fame" && $_ADMIN->isAdmin()) {
			$ftpl = 'ENTITY fame AS $fame {
	if($fame->faction == ".faction" && ceil($fame->fame/6000) >= [0]) {
		GRANT;
		FINAL ENTITY;
	}
}';

			$res = $DBc->sqlQuery("SELECT at_id,atl_name FROM ach_achievement,ach_task,ach_task_lang WHERE at_achievement=aa_id AND ((aa_category<'24' AND aa_category>'15') OR aa_category='56') AND atl_task=at_id AND atl_lang='en'");

			for($i=0;$i<sizeof($res);$i++) {
				$DBc->sqlQuery("INSERT INTO ach_objective (ao_task,ao_condition,ao_value,ao_display,ao_metalink) VALUES ('".$res[$i]['at_id']."','all',NULL,'hidden',NULL)");
				$nid = $DBc->insertID();
				$DBc->sqlQuery("INSERT INTO ach_atom (atom_objective,atom_mandatory,atom_ruleset) VALUES ('".$nid."','0','".$DBc->sqlEscape(str_replace('[0]',$res[$i]['atl_name'],$ftpl))."')");

				$c .= "INSERT INTO ach_objective (ao_task,ao_condition,ao_value,ao_display,ao_metalink) VALUES ('".$res[$i]['at_id']."','all',NULL,'hidden',NULL)<br>";

				$c .= "INSERT INTO ach_atom (atom_objective,atom_mandatory,atom_ruleset) VALUES ('".$nid."','0','".$DBc->sqlEscape(str_replace('[0]',$res[$i]['atl_name'],$ftpl))."')<p>";
			}
		}

		if($_REQUEST['mode'] == "enable_fame" && $_ADMIN->isAdmin()) {
			$res = $DBc->sqlQuery("SELECT aa_id FROM ach_achievement WHERE (aa_category<'24' AND aa_category>'14') OR aa_category='56'");

			for($i=0;$i<sizeof($res);$i++) {
				$DBc->sqlQuery("UPDATE ach_achievement SET aa_dev='0' WHERE aa_id='".$res[$i]['aa_id']."'");

				$DBc->sqlQuery("UPDATE ach_task SET at_dev='0' WHERE at_achievement='".$res[$i]['aa_id']."'");
			}
		}*/

		/*
		 * translation
		 */
		if($_REQUEST['mode'] == "lang" && $_ADMIN->isAdmin()) {
			$c .= "<h1>Language Editor</h1>";

			$user = array();
			$user['id'] = 0;
			$user['lang'] = 'en';
			$user['name'] = 'Talvela';
			$user['race'] = "matis";
			$user['civ'] = "neutral";
			$user['cult'] = "neutral";

			$_USER = new RyzomUser($user);

			//menu
			require_once("include/adm_render_lang.php");
			$menu = new AdmMenu($_REQUEST['cat']);

			$c .= "<center><table>
			<tr>
				<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
			

			$c .= adm_render_menu($menu);
				
			$c .= "</div></td>
					<td width='645px' valign='top'>";
			
			$open = $menu->getOpenCat();

			if($open != 0) {
				$cat = new AdmCategory($open,'%','%','%');

				if($_REQUEST['act'] == "cat_save") {
					if(is_array($_REQUEST['c_name'])) {
						foreach($_REQUEST['c_name'] as $key=>$elem) {
							$cat->setLang($key,$_REQUEST['c_name'][$key]);
						}
					}
				}

				if($_REQUEST['act'] == "ach_save") {
					$ach = $cat->getElementByPath($_REQUEST['id']);

					if(is_array($_REQUEST['a_name'])) {
						foreach($_REQUEST['a_name'] as $key=>$elem) {
							$ach->setLang($key,$_REQUEST['a_name'][$key],$_REQUEST['a_tpl'][$key]);
						}
					}
				}

				if($_REQUEST['act'] == "task_save") {
					$task = $cat->getElementByPath($_REQUEST['id']);

					if(is_array($_REQUEST['t_name'])) {
						foreach($_REQUEST['t_name'] as $key=>$elem) {
							$task->setLang($key,$_REQUEST['t_name'][$key],$_REQUEST['t_tpl'][$key]);
						}
					}
				}

				if($_REQUEST['act'] == "obj_save") {
					$obj = $cat->getElementByPath($_REQUEST['id']);

					if(is_array($_REQUEST['o_name'])) {
						foreach($_REQUEST['o_name'] as $key=>$elem) {
							$obj->setLang($key,$_REQUEST['o_name'][$key]);
						}
					}
				}

				

				$c .= atom_render_category($cat);
			}


			$c .= "</td>
				</tr>
			</table></center>";

		}

		/*
		 * Trigger settings
		 */

		if($_REQUEST['mode'] == "atom" && $_ADMIN->isAdmin()) {
			$c .= "<h1>Trigger Settings</h1>";

			$user = array();
			$user['id'] = 0;
			$user['lang'] = 'en';
			$user['name'] = 'Talvela';
			$user['race'] = "matis";
			$user['civ'] = "neutral";
			$user['cult'] = "neutral";

			$_USER = new RyzomUser($user);

			//menu
			require_once("include/adm_render_atom.php");
			$menu = new AdmMenu($_REQUEST['cat']);

			$c .= "<center><table>
			<tr>
				<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
			

			$c .= adm_render_menu($menu);
				
			$c .= "</div></td>
					<td width='645px' valign='top'>";
			
			$open = $menu->getOpenCat();

			if($open != 0) {
				$cat = new AdmCategory($open,'%','%','%');

				if($_REQUEST['act'] == "insert_atom") {
					$obj = $cat->getElementByPath($_REQUEST['id']);
					
					if($obj != null) {
						$atom = new AdmAtom(array(),$obj);
						$atom->setRuleset($_REQUEST['atom_ruleset']);
						$atom->setMandatory($_REQUEST['atom_mandatory']);
						$atom->setObjective($obj->getID());

						$obj->insertNode($atom);
					}
				}

				if($_REQUEST['act'] == "update_atom") {
					$atom = $cat->getElementByPath($_REQUEST['id']);
					
					if($atom != null) {
						$atom->setRuleset($_REQUEST['atom_ruleset']);
						$atom->setMandatory($_REQUEST['atom_mandatory']);

						$atom->update();
					}
				}

				if($_REQUEST['act'] == "delete") {
					$elem = $cat->getElementByPath($_REQUEST['id']);
					$par = $elem->getParent();
					$par->removeNode($elem->getID());
				}

				$c .= atom_render_category($cat);
			}

			$c .= "</td>
				</tr>
			</table></center>";

		}

		/*
		 * Menu settings
		 */

		if($_REQUEST['mode'] == "menu" && $_ADMIN->isAdmin()) {
			$c .= "<h1>Menu Settings</h1>";

			$user = array();
			$user['id'] = 1;
			$user['lang'] = 'en';
			$user['name'] = 'Talvela';
			$user['race'] = "matis";
			$user['civ'] = "neutral";
			$user['cult'] = "neutral";

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

		/*
		 * Achievement settings
		 */

		if($_REQUEST['mode'] == "ach" && $_ADMIN->isAdmin()) {
			$c .= "<h1>Achievement Settings</h1>";

			$user = array();
			$user['id'] = 0;
			$user['lang'] = 'en';
			$user['name'] = 'Talvela';
			$user['race'] = "matis";
			$user['civ'] = "neutral";
			$user['cult'] = "neutral";

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
				if($_REQUEST['cult']) {
					$cult = $_REQUEST['cult'];
					$_SESSION['cult'] = $cult;
				}
				elseif($_SESSION['cult']) {
					$cult = $_SESSION['cult'];
				}
				else {
					$cult = $_USER->getCult();
				}

				if($_REQUEST['civ']) {
					$civ = $_REQUEST['civ'];
					$_SESSION['civ'] = $civ;
				}
				elseif($_SESSION['civ']) {
					$civ = $_SESSION['civ'];
				}
				else {
					$civ = $_USER->getCiv();
				}

				echo $civ.$cult;

				$cat = new AdmCategory($open,$_USER->getRace(),$cult,$civ);

				$microstop = explode(' ',microtime());
				$stop_time = $microstop[0] + $microstop[1];

				echo "<br>loading: ".round($stop_time - $start_time,3);

				$start_time = $stop_time;

				if($_REQUEST['act'] == "ach_move") {
					$ach = $cat->getChildDataByID($_REQUEST['id']);
					if($ach != null) {
						$ach->setCategory($_REQUEST['new_cat']);
						$ach->update();
						$cat->removeChild($ach->getID());

						$iter = $cat->getOpen();
						while($iter->hasNext()) {
							$item = $iter->getNext();

							if($ach->getID() == $item->getParentID()) {
								$item->setCategory($_REQUEST['new_cat']);
								$item->update();
								$cat->removeChild($item->getID());
							}
						}
					}
				}

				if($_REQUEST['act'] == "ach_insert") {
					$ach = new AdmAchievement(array(),$cat);
					$ach->setCategory($cat->getID());
					$ach->setName($_REQUEST['aal_name']);
					$ach->setTemplate($_REQUEST['aal_template']);
					$ach->setImage($_REQUEST['aa_image']);
					$ach->setParentID($_REQUEST['aa_parent']);
					$ach->setSticky($_REQUEST['aa_sticky']);
					
					$cat->insertNode($ach);

					$task = new AdmTask(array(),$ach);
					$task->setAchievement($ach->getID());
					$task->setName($_REQUEST['atl_name']);
					$task->setTemplate($_REQUEST['atl_template']);
					$task->setValue($_REQUEST['at_value']);
					$task->setCondition($_REQUEST['at_condition']);
					$task->setConditionValue($_REQUEST['at_condition_value']);
					$task->setHeritage(0);

					if(is_array($_REQUEST['at_tie_allegiance'])) {
						$task->setTieAlign($_REQUEST['at_tie_allegiance']);
					}

					$ach->insertNode($task);
				}

				if($_REQUEST['act'] == "ach_update") {
					$ach = $cat->getChildDataByID($_REQUEST['id']);
					
					if($ach != null) {
						$ach->setName($_REQUEST['aal_name']);
						$ach->setTemplate($_REQUEST['aal_template']);
						#$ach->setTieCult($_REQUEST['aa_tie_cult']);
						#$ach->setTieCiv($_REQUEST['aa_tie_civ']);
						$ach->setImage($_REQUEST['aa_image']);
						$ach->setParentID($_REQUEST['aa_parent']);
						$ach->setSticky($_REQUEST['aa_sticky']);

						$ach->update();
					}
				}

				if($_REQUEST['act'] == "task_insert") {
					$ach = $cat->getChildDataByID($_REQUEST['id']);
					if($ach != null) {
						$task = new AdmTask(array(),$ach);
						$task->setAchievement($ach->getID());
						$task->setName($_REQUEST['atl_name']);
						$task->setTemplate($_REQUEST['atl_template']);
						$task->setValue($_REQUEST['at_value']);
						$task->setCondition($_REQUEST['at_condition']);
						$task->setConditionValue($_REQUEST['at_condition_value']);
						$task->setHeritage($_REQUEST['at_inherit']);

						if(is_array($_REQUEST['at_tie_allegiance'])) {
							$task->setTieAlign($_REQUEST['at_tie_allegiance']);
						}

						$ach->insertNode($task);
						$task->setParentID($_REQUEST['at_parent']);
						$ach->orderTasks();
						$task->update();
					}
				}

				if($_REQUEST['act'] == "task_update") {
					$task = $cat->getElementByPath($_REQUEST['id']);
					
					if($task != null) {
						$task->setName($_REQUEST['atl_name']);
						$task->setTemplate($_REQUEST['atl_template']);
						$task->setValue($_REQUEST['at_value']);
						$task->setCondition($_REQUEST['at_condition']);
						$task->setConditionValue($_REQUEST['at_condition_value']);
						$task->setHeritage($_REQUEST['at_inherit']);

						if(is_array($_REQUEST['at_tie_allegiance'])) {
							$task->setTieAlign($_REQUEST['at_tie_allegiance']);
						}

						$task->setParentID($_REQUEST['at_parent']);

						$ach = $task->getParent();
						$ach->orderTasks();

						$task->update();
					}
				}

				if($_REQUEST['act'] == "obj_insert") {
					$task = $cat->getElementByPath($_REQUEST['id']);
					
					if($task != null) {
						$obj = new AdmObjective(array(),$task);
						$obj->setName($_REQUEST['aol_name']);
						$obj->setCondition($_REQUEST['ao_condition']);
						$obj->setValue($_REQUEST['ao_value']);
						$obj->setDisplay($_REQUEST['ao_display']);
						$obj->setMetalink($_REQUEST['ao_metalink']);
						$obj->setTask($task->getID());

						$task->insertNode($obj);
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
					if($elem != null) {
						$par = $elem->getParent();
						$par->removeNode($elem->getID());

						if(get_class($elem) == "AdmAchievement") {
							$iter = $cat->getOpen();
							while($iter->hasNext()) {
								$item = $iter->getNext();

								if($elem->getID() == $item->getParentID()) {
									$item->setParentID(null);
									$item->update();
								}
							}
						}
					}
				}

				if($_REQUEST['act'] == "dev") {
					$curr = $cat->getElementByPath($_REQUEST['id']);
					$curr->setInDev(($_REQUEST['state'] != 1));
				}

				$microstop = explode(' ',microtime());
				$stop_time = $microstop[0] + $microstop[1];

				echo "<br>manipulation: ".round($stop_time - $start_time,3);

				$start_time = $stop_time;
				
				$c .= adm_render_category($cat);

				$microstop = explode(' ',microtime());
				$stop_time = $microstop[0] + $microstop[1];

				echo "<br>rendering: ".round($stop_time - $start_time,3);
			}

			$c .= "</td>
				</tr>
			</table></center>";

		}

		/*
		 * Statistics page
		 */

		if($_REQUEST['mode'] == "stats" && $_ADMIN->isCSR()) {
			require_once("include/adm_render_stats.php");

			$c .= "<h1>Statistics</h1>";

			$c .= stats_render();
		}

		/*
		 * CSR player manager
		 */
		if($_REQUEST['mode'] == "player" && $_ADMIN->isCSR()) {
			$c .= "<h1>Player Administration</h1>";

			#$DBc_char = new mySQL($_CONF['mysql_error']);
			#$DBc_char->connect($_CONF['char_mysql_server'],$_CONF['char_mysql_user'],$_CONF['char_mysql_pass'],$_CONF['char_mysql_database']);

			$DBc_char = ryDB::getInstance("webig");
			//menu
			require_once("include/adm_render_csr.php");

			if(!is_user($_REQUEST['pid'])) { // no user ID
				$c .= csr_render_find_player();
			}
			else {
				$user = array();
				$user['id'] = $_REQUEST['pid'];
				$user['lang'] = 'en';
				$dta = user_get_data($_REQUEST['pid']);
				$user['char_name'] = user_get_name($_REQUEST['pid']);
				$user['race'] = substr($dta['race'],2);
				$user['civ'] = substr($dta['civilisation'],2);
				$user['cult'] = substr($dta['cult'],2);

				$_USER = new RyzomUser($user);

				$menu = new AchMenu($_REQUEST['cat']);

				$open = $menu->getOpenCat();

				if($open != 0) {
					if($_REQUEST['cult']) {
						$cult = $_REQUEST['cult'];
						$_SESSION['cult'] = $cult;
					}
					elseif($_SESSION['cult']) {
						$cult = $_SESSION['cult'];
					}
					else {
						$cult = $_USER->getCult();
					}

					if($_REQUEST['civ']) {
						$civ = $_REQUEST['civ'];
						$_SESSION['civ'] = $civ;
					}
					elseif($_SESSION['civ']) {
						$civ = $_SESSION['civ'];
					}
					else {
						$civ = $_USER->getCiv();
					}

					$cat = new CSRCategory($open,$_USER->getRace(),$cult,$civ);

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


				if($open != 0) {
					$c .= csr_render_category($cat);
				}
				else {
					$cat = new AchSummary($menu,3);
					$c .= ach_render_summary_header();
				}

				if($open == 0) {
					$c .= ach_render_summary_footer($cat);
				}

				$c .= "</td>
					</tr>
				</table></center>";
			}

		}

$c .= "</td>
	</tr>
</table></center>";

#$c = var_export($_USER).$c;

echo ryzom_app_render("achievements admin", $c, $_ADMIN->isIG());

?>
