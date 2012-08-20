<?php
	function adm_render_menu(&$menu,$sub = 0) {
		$html = "<style>
				.ach_menu {
					display:block;
					padding:2px;
					border:1px solid #000000;
					margin-bottom:2px;
					color:#FFFFFF;
				}
				.ach_menu:hover {
					color:orange;
				}

				.ach_mspan a {
					text-decoration:none;
				}
				</style>";

		return $html.adm_render_mnode($menu,$sub);
	}

	function adm_render_mnode(&$menu,$sub) {
		global $_CONF;

		$html = "";
		
		$iter = $menu->getIterator();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			$html .= "<span class='ach_mspan'><a href='?mode=lang&cat=".$curr->getID()."'><table class='ach_menu'>
				<tr>";
					if($sub == 0) {
						$html .= "<td><img src='".$_CONF['image_url']."pic/menu/".$curr->getImage()."' /></td>";
					}
					$html .= "<td style='font-size:".(20-$sub)."px;font-weight:bold;";
					if($curr->isOpen()) {
						$html .= "color:orange;";
					}
					$html .= "'>".$curr->getName()."</td>
				</tr>
			</table></a></span>";
			if($curr->hasOpenCat() != 0) {
				$html .= "<div style='display:block;margin-left:25px;'>".adm_render_mnode($curr,($sub+4))."</div>";
			}
		}

		return $html;
	}

	function atom_render_category(&$cat) {
		$html = "<style>
			.bar {
				background-color:#FFFFFF;
				color:#000000;
				padding:2px;
				margin-bottom:2px;
				font-weight:bold;
			}

			.bar a {
				color:#000000;
				text-decoration:none;
			}
		</style>";
		
		$iter = $cat->getOpen();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			$html .= ach_render_achievement_open($curr);
		}

		return $html;
	}

	function ach_render_achievement_open(&$ach) {
		global $_CONF;

		$open = explode(";",$_REQUEST['id']);

		$o = "none";
		if($open[1] == $ach->getID()) {
			$o = "block";
		}

		$html = "<div style='display: block; margin-bottom: 5px;'>
			<div style='display:block;font-size:22px;' class='bar'><a href='javascript:hs(\"ach_".$ach->getID()."\",\"block\");'>[+]</a> ".$ach->getName()." <span style='font-size:12px;'>(ties= race: ".$ach->getTieRace()."; civ: ".$ach->getTieCiv()."; cult: ".$ach->getTieCult().")</span>
			
			<div style='margin-left:35px;'>
				<form method='post' action='?mode=lang&cat=".$_REQUEST['cat']."&act=ach_save&id=".$ach->getPathID()."'>
				<table>
					<tr>
						<td>&nbsp;</td>
						<td style='color:#454545;'>name</td>
						<td style='color:#454545;'>template</td>
					</tr>";

					foreach($_CONF['langs'] as $elem) {
						$tmp = $ach->getLang($elem);

						$html .= "<tr>
							<td style='color:#454545;'>".$elem."</td>
							<td><input type='text' name='a_name[".$elem."]' style='width:270px;' value='".htmlspecialchars($tmp[0],ENT_QUOTES)."' /></td>
							<td><input type='text' name='a_tpl[".$elem."]' style='width:270px;' value='".htmlspecialchars($tmp[1],ENT_QUOTES)."' /></td>
						</tr>";
					}

				$html .= "<tr>
						<td>&nbsp;</td>
						<td colspan='2'><input type='submit' value='save' /></td>
					</tr>
					</table>
					</form>
			</div>

			</div>
			<div style='margin-left:25px;display:".$o.";' id='ach_".$ach->getID()."'>".ach_render_task_open($ach)."</div>
		</div>";

		return $html;
	}

	function ach_render_task_open(&$ach) {
		global $_CONF;

		$html = "";

		$open = explode(";",$_REQUEST['id']);

		$task_list = $ach->getOpen();
		while($task_list->hasNext()) {

			$task = $task_list->getNext();

			$o = "none";
			if($open[2] == $task->getID()) {
				$o = "block";
			}

			$html .= "<div style='display: block; margin-bottom: 5px;'>
				<div style='display:block;font-size:16px;' class='bar'><a href='javascript:hs(\"task_".$task->getID()."\",\"block\");'>[+]</a> ".$task->getDisplayName()."
				
				<div style='margin-left:35px;'>
				<form method='post' action='?mode=lang&cat=".$_REQUEST['cat']."&act=task_save&id=".$task->getPathID()."'>
				<table>
					<tr>
						<td>&nbsp;</td>
						<td style='color:#454545;'>name</td>
						<td style='color:#454545;'>template</td>
					</tr>";

					foreach($_CONF['langs'] as $elem) {
						$tmp = $task->getLang($elem);

						$html .= "<tr>
							<td style='color:#454545;'>".$elem."</td>
							<td><input type='text' name='t_name[".$elem."]' style='width:258px;' value='".htmlspecialchars($tmp[0],ENT_QUOTES)."' /></td>
							<td><input type='text' name='t_tpl[".$elem."]' style='width:258px;' value='".htmlspecialchars($tmp[1],ENT_QUOTES)."' /></td>
						</tr>";
					}

				$html .= "<tr>
						<td>&nbsp;</td>
						<td colspan='2'><input type='submit' value='save' /></td>
					</tr>
					</table>
					</form>
			</div>
				
				</div>
				<div style='margin-left:25px;display:".$o.";' id='task_".$task->getID()."'>".ach_render_obj_list($task->getIterator())."</div>
			</div>";
		}

		return $html;
	}

	function ach_render_obj_list($obj) {
		global $_CONF;

		$html = "";


		while($obj->hasNext()) {
			$elem = $obj->getNext();
			



			
			$html .= "<div style='display: block; margin-bottom: 5px;'>
				<div style='display:block;' class='bar'>&nbsp;&nbsp;".$elem->getDisplayName()."</span>
				
				<div style='margin-left:35px;'>
				<form method='post' action='?mode=lang&cat=".$_REQUEST['cat']."&act=obj_save&id=".$elem->getPathID()."'>
						<table>
					<tr>
						<td>&nbsp;</td>
						<td style='color:#454545;'>name</td>
					</tr>";

					foreach($_CONF['langs'] as $lang) {
						$tmp = $elem->getLang($lang);

						$html .= "<tr>
							<td style='color:#454545;'>".$lang."</td>
							<td><input type='text' name='' style='width:246px;' value='".htmlspecialchars($tmp,ENT_QUOTES)."' /></td>
						</tr>";
					}

				$html .= "<tr>
						<td>&nbsp;</td>
						<td><input type='submit' value='save' /></td>
					</tr>
					</table>
					</form>
			</div>
				
				</div>
				

			</div>";
		}

		return $html;
	}
?>