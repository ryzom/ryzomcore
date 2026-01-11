<?php
	function is_user($id) {
		global $DBc_char;

		$res = $DBc_char->sqlQuery("SELECT count(*) as anz FROM players WHERE id='".$DBc_char->sqlEscape($id)."'");

		if($res[0]['anz'] > 0) {
			return true;
		}
		return false;
	}

	function user_get_name($id) {
		global $DBc_char;

		$res = $DBc_char->sqlQuery("SELECT name FROM players WHERE id='".$DBc_char->sqlEscape($id)."'");

		return $res[0]['name'];
	}

	function user_get_data($id) {
		global $_CONF,$DBc_char;
		$DBc_align = new mySQL($_CONF['mysql_error']);
		$DBc_align->connect($_CONF['char_mysql_server'],$_CONF['char_mysql_user'],$_CONF['char_mysql_pass'],$_CONF['char_mysql_database']);

		$res = $DBc_char->sqlQuery("SELECT cid FROM players WHERE id='".$DBc_char->sqlEscape($id)."'");

		$res = $DBc_align->sendSQL("SELECT race,civilisation,cult FROM characters WHERE char_id='".$res[0]['cid']."'","ARRAY");

		return $res[0];
	}

	function csr_render_yubopoints() {
		global $DBc,$_USER,$_CONF;

		$res = $DBc->sqlQuery("SELECT sum(at_value) as anz FROM ach_task,ach_player_task WHERE at_id=apt_task AND apt_player='".$_USER->getID()."'");

		$html = "<div style='display:block;border-bottom:1px solid #000000;'><span style='font-size:32px;'>".$_USER->getName()."&nbsp;<img src='".$_CONF['image_url']."pic/yubo_done.png'>&nbsp;".max(0,$res[0]['anz'])."</span></div>";

		return $html;
	}

	function csr_render_find_player() {
		global $DBc_char;

		$html = "<form method='post' action='?mode=player'>
			<fieldset>
			<legend>Search for a player</legend>
				<table>
					<tr>
						<td>Name:&nbsp;</td>
						<td><input type='text' name='pname' value=\"".$_REQUEST['pname']."\" /></td>
					</tr>
					<tr>
						<td colspan='2'><input type='submit' value='search' /></td>
					</tr>
				</table>
			</fieldset>
		</form>";

		if($_REQUEST['pname'] != "") {
			$html .= "<div style='display:block;color:#000000;background-color:#FFFFFF;margin-top:5px;'>";
			
			$res = $DBc_char->sqlQuery("SELECT * FROM players WHERE (name LIKE '".$DBc_char->sqlEscape(strtolower($_REQUEST['pname']))."%' OR id='".$DBc_char->sqlEscape(strtolower($_REQUEST['pname']))."') AND deleted='0' LIMIT 0,100");
			$sz = sizeof($res);

			if($sz == 0) {
				$html .= "<b>no characters found</b>";
			}
			
			$cols = 1;
			if($sz > 8) {
				$cols = 3;
			}

			$html .= "<table><tr>";

			for($i=0;$i<$sz;$i++) {
				if($cols != 1 && ($i%$cols) == 0) {
					$html .= "</tr><tr>";
				}

				$html .= "<td><a href='?mode=player&pid=".$res[$i]['id']."'><b>".$res[$i]['name']."</b></a></td>";
			}

			$html .= "</tr></table>";

			$html .= "</div>";
		}

		return $html;
	}

	function csr_render_menu(&$menu,$sub = 0) {
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
		#$sz = $menu->getSize();
		#for($i=0;$i<$sz;$i++) {
		#	$curr = $menu->getChild($i);
			if($curr->inDev()) {
				#continue;
			}
			$html .= "<span class='ach_mspan'><a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$curr->getID()."'><table class='ach_menu'>
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

	function csr_render_category(&$cat) {
		$html = "";

		if($cat->hasTieAlign_done() || $cat->hasTieAlign_open()) {
			$html .= ach_render_tiebar($cat->getCurrentCult(),$cat->getCurrentCiv(),$cat);
		}

		$iter = $cat->getDone();

		while($iter->hasNext()) {
			$curr = $iter->getNext();
			
		#$sz = sizeof($tmp);
		#for($i=0;$i<$sz;$i++) {
			#echo "A";
			if($curr->inDev()) {
				continue;
			}
			$html .= ach_render_achievement_done($curr);
		}

		$iter = $cat->getOpen();

		while($iter->hasNext()) {
			$curr = $iter->getNext();

		#$sz = sizeof($tmp);
		#for($i=0;$i<$sz;$i++) {
			#echo "B";
			if($curr->inDev()) {
				continue;
			}
			$html .= ach_render_achievement_open($curr);
		}

		return $html;
	}

	function ach_render_achievement_done(&$ach) {
		global $_CONF;

		$html = '<div style="display: block; margin-bottom: 5px;"><table cellpadding="0" cellspacing="0" width="100%">
					<tbody><tr>
						<td width="3px"><img src="'.$_CONF['image_url'].'pic/bar_done_ul.png"></td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_done_u.png);"></td>
						<td width="3px"><img src="'.$_CONF['image_url'].'pic/bar_done_ur.png"></td>
					</tr>
					<tr>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_done_l.png);"></td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_done_bg.png);">
							<center><table width="100%" cellspacing="0" cellpadding="0">
								<tbody><tr>
									<td rowspan="2" valign="top"><img src="'.$_CONF['image_url'].'pic/icon/'.$ach->getImage().'"></td>
									<td width="100%"><center><span style="font-weight:bold;font-size:24px;color:#000000;">'.$ach->getName().'</span></center></td>
									<td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#000000;padding-right:10px;">
										'.$ach->getValueDone().'<br><img src="'.$_CONF['image_url'].'pic/yubo_done.png">
									</td>
								</tr><tr><td align="center" valign="top">';
							$html .= ach_render_task_done($ach);
							$html .= '</td></tr></tbody></table></center>
						</td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_done_r.png);"></td>
					</tr>
					<tr>
						<td><img src="'.$_CONF['image_url'].'pic/bar_done_bl.png"></td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_done_b.png);"></td>
						<td><img src="'.$_CONF['image_url'].'pic/bar_done_br.png"></td>
					</tr>
				</tbody></table></div>';

		return $html;
	}

	function ach_render_achievement_open(&$ach) {
		global $_CONF;

		$html = '<div style="display: block; margin-bottom: 5px;"><table cellpadding="0" cellspacing="0" width="100%">
					<tbody><tr>
						<td width="3px"><img src="'.$_CONF['image_url'].'pic/bar_pending_ul.png"></td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_pending_u.png);"></td>
						<td width="3px"><img src="'.$_CONF['image_url'].'pic/bar_pending_ur.png"></td>
					</tr>
					<tr>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_pending_l.png);"></td>
						<td>
							<center><table width="100%" cellspacing="0" cellpadding="0">
								<tbody><tr>
									<td rowspan="2" valign="top"><img src="'.$_CONF['image_url'].'pic/icon/grey/'.$ach->getImage().'"></td>
									<td width="100%"><center><span style="font-weight:bold;font-size:24px;color:#FFFFFF;">'.$ach->getName().'</span></center></td>
									<td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#FFFFFF;padding-right:10px;">
										'.$ach->getValueOpen().'<br><img src="'.$_CONF['image_url'].'pic/yubo_pending.png">
									</td>
								</tr><tr><td align="center" valign="top">';
							$html .= ach_render_task_open($ach);
							$html .= '</td></tr></tbody></table></center>
						</td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_pending_r.png);"></td>
					</tr>
					<tr>
						<td><img src="'.$_CONF['image_url'].'pic/bar_pending_bl.png"></td>
						<td style="background-image: url('.$_CONF['image_url'].'pic/bar_pending_b.png);"></td>
						<td><img src="'.$_CONF['image_url'].'pic/bar_pending_br.png"></td>
					</tr>
				</tbody></table></div>';

		return $html;
	}

	function ach_render_task_open(&$ach) {
		$html = "";

		$task_list = $ach->getOpen();
		$task = $task_list->getNext();
		

		if($task->inDev()) {
			return $html;
		}
		
		$html .= "<span style='color:#999999;font-weight:bold;display:block;'>";
		
		if($task->getName() != null) {
			$html .= $task->getDisplayName();
		}
		else {
			$html .= "[untitled]";
		}
		$html .=  " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&grant=".$task->getPath()."'><b>Task:</b> grant</a></span>";

		if($task->objDrawable()) {
			$html .= ach_render_obj_list($task->getIterator());
		}

		return $html;
	}

	function ach_render_task_done(&$ach) {
		global $_CONF;
		$html = "";

		$task_list = $ach->getDone();
		while($task_list->hasNext()) {
			$task = $task_list->getNext();

			if($task->inDev()) {
				continue;
			}
			$html .= "<div style='display:block;'><span style='color:#66CC00;font-weight:bold;'>".$task->getDisplayName()."</span> ( ".date('d.m.Y',$task->getDone())." ) <img src='".$_CONF['image_url']."pic/yubo_done.png' width='15px' /> ".$task->getValue()." <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&deny=".$task->getPath()."'>Task: deny</a></div>";
		}

		return $html;
	}

	function ach_render_obj_list($obj) {
		$html = "<center><table width='90%'>";

		$i = 0;
		$skip = false;
		
		while($obj->hasNext()) {
		#foreach($obj as $elem) {
			$elem = $obj->getNext();
			if(($i%2) == 0) {
				$html .= "<tr>";
			}

			switch($elem->getDisplay()) {
				case "meta":
					$html .= "<td width='50%'>".ach_render_obj_meta($elem)."</td>";
					break;
				case "value":
					if(($i%2) == 1) {
						$html .= "</tr><tr>";
					}
					$html .= "<td colspan='2' width='100%'><center>".ach_render_obj_value($elem)."</center></td>";
					$i++;
					break;
				case "simple":
					$html .= "<td width='50%'>".ach_render_obj_simple($elem)."</td>";
					break;
				case "hidden":
				default:
					//do nothing
					$skip = true;
					break;
			}

			if(($i%2) == 1) {
				$html .= "</tr>";
			}
			
			
			if(!$skip) {
				$i++;
			}
			$skip = false;
		}

		if(($i%2) == 1) {
			$html .= "</tr>";
		}

		$html .= "</table></center>";

		return $html;
	}

	function ach_render_obj_simple(&$obj) {
		global $_CONF;
		$html = "";
		if($obj->isdone()) {
			$html .= "<img src='".$_CONF['image_url']."pic/check.png' height='10px' />&nbsp;<span style='color:#71BE02;'>";
		}
		else {
			$html .= "<img src='".$_CONF['image_url']."pic/pending.png' height='10px' />&nbsp;<span style='color:#999999;'>";
		}
		
		$html .= $obj->getDisplayName();
		if($obj->isdone()) {
			$html .= " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&deny=".$obj->getPath()."'>Obj: deny</a>";
		}
		else {
			$html .= " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&grant=".$obj->getPath()."'>Obj: grant</a>";
		}
		$html .= "</span>";

		return $html;
	}

	function ach_render_obj_meta(&$obj) {
		global $_CONF;
		$html = "";
		if($obj->isdone()) {
			$col = "#71BE02";
			$grey = "";
		}
		else {
			$col = "#999999";
			$grey = "grey/";
		}

		$html .= "<table cellspacing='0' cellpadding='0'>
				<tr>
					<td><img src='".$_CONF['image_url']."pic/icon/".$grey.$obj->getMetaImage()."' width='20px' /></td>
					<td valign='middle'><span style='color:".$col.";'>&nbsp;".$obj->getDisplayName();
					if($obj->isdone()) {
						$html .= " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&deny=".$obj->getPath()."'>Obj: deny</a>";
					}
					else {
						$html .= " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&grant=".$obj->getPath()."'>Obj: grant</a>";
					}
					$html .= "</span></td>
				</tr>
			</table>";

		return $html;
	}

	function ach_render_obj_value(&$obj) {
		$html = "";
		if($obj->getName() != null) {
			if($obj->isdone()) {
				$col = "#71BE02";
			}
			else {
				$col = "#999999";
			}
			$html .= "<div style='color:".$col.";display:block;'>".$obj->getDisplayName()."</div>";
		}

		$html .= "<table>
			<tr>
				<td>".ach_render_progressbar($obj->getProgress(),$obj->getValue(),350)."</td>
				<td>";
				if($obj->isdone()) {
					$html .= " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&deny=".$obj->getPath()."'>Obj: deny</a>";
				}
				else {
					$html .= " <a href='?mode=player&pid=".$_REQUEST['pid']."&cat=".$_REQUEST['cat']."&grant=".$obj->getPath()."'>Obj: grant</a>";
				}
				$html .= "</td>
			</tr>
		</table>";
		
		return $html;
	}

	function ach_render_progressbar($prog,$val,$width) {
		$val = max(1,$val);
		$left = floor($width*(100*($prog/$val))/100);

		$html = "
		<table width='".$width."px' cellspacing='0' cellpadding='0' style='border:1px solid #FFFFFF;color:#000000;'>
			<tr>
				<td bgcolor='#66CC00' width='".$left."px' align='right'>";
				if(($prog/$val) > 0.85) {
					$html .= "&nbsp;".nf($prog)." / ".nf($val)."&nbsp;";
				}
				$html .= "</td>
				<td align='left' style='color:#FFFFFF;'>";
				if(($prog/$val) <= 0.85) {
					$html .= "&nbsp;".nf($prog)." / ".nf($val)."&nbsp;";
				}
				$html .= "</td>
			</tr>
		</table>";
		
		return $html;
	}

	function ach_render_summary_header() {
		global $_USER;
		
		return "<div style='display:block;font-weight:bold;font-size:30px;color:#FFFFFF;text-align:center;margin-bottom:10px;'>".get_translation('ach_summary_header',$_USER->getLang())."</div>";
	}

	function ach_render_summary_footer(&$summary) {
		global $_USER;

		$nodes = $summary->getSummary();
		$html = "";

		$sum_done = 0;
		$sum_total = 0;

		$i = 0;
		foreach($nodes as $elem) {
			if(($i%3) == 0) {
				$html .= "<tr>";
			}

			$html .= "<td width='50%' align='center'>".$elem[0]."<br>".ach_render_progressbar($elem[1],$elem[2],200)."</td>";
			$sum_done += $elem[1];
			$sum_total += $elem[2];

			if(($i%3) == 2) {
				$html .= "</tr>";
			}

			$i++;
		}

		if(($i%3) == 2) {
			$html .= "</tr>";
		}

		$html = "<p />
		<div style='display:block;font-weight:bold;font-size:30px;color:#FFFFFF;text-align:center;margin-bottom:10px;'>".get_translation('ach_summary_stats',$_USER->getLang())."</div>
		<table>
			<tr>
				<td colspan='3' align='center'>".get_translation('ach_summary_stats_total',$_USER->getLang())."<br>".ach_render_progressbar($sum_done,$sum_total,450)."<br></td>
			</tr>
			".$html."
		</table>";

		return $html;
	}

	function ach_render_tiebar($cult = "c_neutral", $civ = "c_neutral",&$cat) {
		global $_USER;

		$html = "<style>
			.o {
				color:orange;
			}
		</style>

		<div style='display:block;text-align:center;'><form method='post' action='?cat=".$cat->getID()."&mode=player&pid=".$_REQUEST['pid']."' id='cc_form'>
			<table>
				<tr>";
				if($cat->isAllowedCult()) {
					$html.= "<td>
						<select name='cult' onchange='document.getElementById(\"cc_form\").submit();'>
							<option value='c_neutral'"; if($cult == "c_neutral") { $html.= " selected='selected'"; } $html .= ">".get_translation('ach_c_neutral',$_USER->getLang())."</option>
							<option value='c_kami'"; if($cult == "c_kami") { $html.= " selected='selected'"; } $html .= ">Kami</option>
							<option value='c_karavan'"; if($cult == "c_karavan") { $html.= " selected='selected'"; } $html .= ">Karavan</option>
						</select>
					</td>";
				}
				if($cat->isAllowedCiv()) {
					$html.= "<td>
						<select name='civ' onchange='document.getElementById(\"cc_form\").submit();'>
							<option value='c_neutral'"; if($civ == "c_neutral") { $html.= " selected='selected'"; } $html .= ">".get_translation('ach_c_neutral',$_USER->getLang())."</option>
							<option value='c_fyros'"; if($civ == "c_fyros") { $html.= " selected='selected'"; } $html .= ">Fyros</option>
							<option value='c_matis'"; if($civ == "c_matis") { $html.= " selected='selected'"; } $html .= ">Matis</option>
							<option value='c_tryker'"; if($civ == "c_tryker") { $html.= " selected='selected'"; } $html .= ">Tryker</option>
							<option value='c_zorai'"; if($civ == "c_zorai") { $html.= " selected='selected'"; } $html .= ">Zorai</option>
						</select>
					</td>";
				}
				$html.= "</tr>
			</table>
		</form></div>
		
		<div style='display:block;font-weight:bold;font-size:20px;color:#FFFFFF;text-align:center;margin-bottom:5px;'>";

		/*if($cat->isTiedCult() && !$cat->isTiedCiv() && $cult == "c_neutral") { // neutral / xx
			#While being of neutral allegiance with the higher powers
			$html .= get_translation('ach_allegiance_neutral_cult',$_USER->getLang(),array("<span class='o'>".get_translation('ach_c_neutral',$_USER->getLang())."</span>"));
		}
		elseif($cat->isTiedCiv() && !$cat->isTiedCult() && $civ == "c_neutral") { // xx / neutral
			#While being of neutral allegiance with the homin civilizations
			$html .= get_translation('ach_allegiance_neutral_civ',$_USER->getLang(),array("<span class='o'>".get_translation('ach_c_neutral',$_USER->getLang())."</span>"));
		}*/
		if(($cult == "c_neutral" || !$cat->isAllowedCult()) && ($civ == "c_neutral" || !$cat->isAllowedCiv())) { // neutral / neutral
			#While being of neutral allegiance
			$html .= get_translation('ach_allegiance_neutral',$_USER->getLang(),array("<span class='o'>".get_translation('ach_c_neutral',$_USER->getLang())."</span>"));
		}
		else { //other
			#While being aligned with the
			$html .= get_translation('ach_allegiance_start',$_USER->getLang());
			if($cat->isAllowedCult() && $cult != "c_neutral") {
				#CULT
				$html .= "<span class='o'>".ach_translate_cc($cult)."</span>";
				if($cat->isAllowedCiv() && $civ != "c_neutral") {
					#and the CIV
					$html .= get_translation('ach_allegiance_and',$_USER->getLang())." <span class='o'>".ach_translate_cc($civ)."</span>";
				}
			}
			elseif($cat->isAllowedCiv() && $civ != "c_neutral") {
				#CIV
				$html .= "<span class='o'>".ach_translate_cc($civ)."</span>";
			}
		}
		#, accomplish the following achievements:
		$html .= get_translation('ach_allegiance_end',$_USER->getLang())."</div>";

		return $html;
	}
?>