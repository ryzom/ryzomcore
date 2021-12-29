<?php
	function ach_render() {
		global $_USER,$_CONF;

		$c = "<center><table>
			<tr>
				<td colspan='2' align='left'>
					<div style='display:block;border-bottom:1px solid #000000;'>
						<div style='float:left;width:420px;'>".ach_render_yubopoints()."</div>
						<div style='float:right;width:420px;text-align:right;'>".ach_render_facebook()."</div>
						<div style='clear:both;'></div>
					</div>
					
				</td>
			</tr>
			<tr>
				<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
				#$_REQUEST['mid'] = 1;
				
				$menu = new AchMenu($_REQUEST['cat']);

				$c .= ach_render_menu($menu);
				
		$c .= "</div></td>
				<td width='645px' valign='top'>";

		/*for($i=0;$i<15;$i++) {
			$c .= ach_render_box_done("Bejeweled");
		}*/

		$open = $menu->getOpenCat();

		if($open != 0) {
			if($_REQUEST['cult']) {
				$cult = $_REQUEST['cult'];
			}
			else {
				$cult = $_USER->getCult();
			}

			if($_REQUEST['civ']) {
				$civ = $_REQUEST['civ'];
			}
			else {
				$civ = $_USER->getCiv();
			}

			$cat = new AchCategory($open,$_USER->getRace(),$cult,$civ);
		}
		else {
			#die($_CONF['summary_size']);
			$cat = new AchSummary($menu,$_CONF['summary_size']);
			$c .= ach_render_summary_header();
		}

		$c .= ach_render_category($cat);
		if($open == 0) {
			$c .= ach_render_summary_footer($cat);
		}

		$c .= "</td>
			</tr>
		</table></center>";

		return $c;
	}

	function ach_render_tiebar($cult = "c_neutral", $civ = "c_neutral", &$cat) {
		global $_USER;

		$html = "<style>
			.o {
				color:orange;
			}
		</style>

		<div style='display:block;text-align:center;'><form method='post' action='?cat=".$cat->getID()."' id='cc_form'>
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

	function ach_render_yubopoints() {
		global $DBc,$_USER,$_CONF;

		$res = $DBc->sqlQuery("SELECT sum(at_value) as anz FROM ach_task,ach_player_task WHERE at_id=apt_task AND apt_player='".$_USER->getID()."'");

		$html = "<div style='display:block;'><span style='font-size:32px;'>".$_USER->getName()."&nbsp;<img src='".$_CONF['image_url']."pic/yubo_done.png'>&nbsp;".max(0,$res[0]['anz'])."</span></div>";

		return $html;
	}

	function ach_render_facebook() {
		return "";

		return "<div id='auth-status'>
			<div id='auth-loggedout'>
				<a href='#' id='auth-loginlink'><img src='pic/f-connect.png' height='30px'></a>
			</div>
			<div id='auth-loggedin' style='display:none'>
				<table cellpadding='0' align='right'>
					<tr>
						<td rowspan='2'><img src='pic/facebook-logo.png' height='30px'></td>
						<td style='text-align:left;font-size:10px;'>connected</td>
					</tr>
					<tr>
						<td><span id='auth-displayname' style='font-size:16px;'></span></td>
					</tr>
				</table>
			
			</div>
		</div>";
	}

	function ach_render_twitter() {

	}

	function ach_render_menu(&$menu,$sub = 0) {
		$html = "<style>
				.ach_menu {
					display:block;
					padding:2px;
					border:1px solid #000000;
					margin-bottom:2px;
					color:#FFFFFF;
					width:100%;
				}
				.ach_menu:hover {
					color:orange;
				}

				.ach_mspan a {
					text-decoration:none;
				}
				</style>";

		return $html.ach_render_mnode($menu,$sub);
	}

	function ach_render_mnode(&$menu,$sub) {
		global $_CONF;

		$html = "";
		
		$iter = $menu->getIterator();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			if($curr->inDev()) {
				continue;
			}
			$html .= "<span class='ach_mspan'><a href='?cat=".$curr->getID()."'><table class='ach_menu'>
				<tr>";
					if($sub == 0) {
						$html .= "<td style='width:32px;'><img src='".$_CONF['image_url']."pic/menu/".$curr->getImage()."' /></td>";
					}
					$html .= "<td style='font-size:".(20-$sub)."px;font-weight:bold;";
					if($curr->isOpen()) {
						$html .= "color:orange;";
					}
					$html .= "'>".$curr->getName()."</td>
				</tr>
			</table></a></span>";
			if($curr->hasOpenCat() != 0) {
				$html .= "<div style='display:block;margin-left:25px;'>".ach_render_mnode($curr,($sub+4))."</div>";
			}
		}

		return $html;
	}

	function ach_render_category(&$cat) {
		global $_USER;
		$html = "";

		if($cat->isHeroic() && !$cat->hasDone()) {
			return "<center style='font-size:24px;'>".get_translation('ach_no_heroic_deeds',$_USER->getLang())."</center>";
		}

		if($cat->hasTieAlign_done() || $cat->hasTieAlign_open()) {
			$html .= ach_render_tiebar($cat->getCurrentCult(),$cat->getCurrentCiv(),$cat);
		}

		$iter = $cat->getDone();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			if($curr->inDev() || !$curr->parentDone() || !$curr->isTiedRace_done($cat->getCurrentRace()) || !$curr->isTiedAlign_done($cat->getCurrentCult(),$cat->getCurrentCiv())) {
				continue;
			}
			$html .= ach_render_achievement_done($curr,$cat);
		}

		if($cat->isHeroic()) {
			return $html;
		}

		$iter = $cat->getOpen();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			if($curr->inDev() || !$curr->parentDone() || !$curr->isTiedRace_open($cat->getCurrentRace()) || !$curr->isTiedAlign_open($cat->getCurrentCult(),$cat->getCurrentCiv())) {
				continue;
			}
			$html .= ach_render_achievement_open($curr,$cat);
		}

		return $html;
	}

	function ach_render_achievement_done(&$ach,&$cat) {
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
									<td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#000000;padding-right:10px;">';
									if((!$ach->isHeroic() && !$ach->isContest()) && $ach->getValueDone() > 0) {
										$html .= $ach->getValueDone().'<br><img src="'.$_CONF['image_url'].'pic/yubo_done.png">';
									}
									else {
										$html .= '<img src="'.$_CONF['image_url'].'pic/star_done.png"><br>&nbsp;';
									}
									$html .= '</td>
								</tr><tr><td align="center" valign="top"><table>';
							$html .= ach_render_task_done($ach,$cat);
							$html .= '</table></td></tr></tbody></table></center>
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

	function ach_render_achievement_open(&$ach,&$cat) {
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
									<td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#FFFFFF;padding-right:10px;">';
									if(!$ach->isHeroic() && !$ach->isContest()) {
										$html .= $ach->getValueOpen().'<br><img src="'.$_CONF['image_url'].'pic/yubo_pending.png">';
									}
									else {
										$html .= '<img src="pic/star_pending.png">';
									}
								$html .= '</td>
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
		
		if($task->getName() != null) {
			$html .= "<span style='color:#999999;font-weight:bold;display:block;'>".$task->getDisplayName()."</span>";
		}
		if($task->objDrawable()) {
			$html .= ach_render_obj_list($task->getIterator());
		}

		return $html;
	}

	function ach_render_task_done(&$ach,&$cat) {
		global $_CONF;
		$html = "";

		$task_list = $ach->getDone();
		while($task_list->hasNext()) {
			$task = $task_list->getNext();

			if($task->inDev() || !$task->isTiedRace_open($cat->getCurrentRace()) || !$task->isTiedAlign_open($cat->getCurrentCult(),$cat->getCurrentCiv())) {
				continue;
			}
			$html .= "<tr><td><span style='color:#66CC00;font-weight:bold;'>".$task->getDisplayName()."</span></td><td>( ".date('d.m.Y',$task->getDone())." )</td>";
			if($task->getValue() > 0) {
				$html .= "<td><img src='".$_CONF['image_url']."pic/yubo_done.png' width='15px' /> ".$task->getValue()."</td>";
			}
			$html .= "</tr>";
		}

		return $html;
	}

	function ach_render_obj_list($obj) {
		$html = "<center><table width='90%'>";

		$i = 0;
		$skip = false;
		
		while($obj->hasNext()) {

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
		
		$html .= $obj->getDisplayName()."</span>";

		return $html;
	}

	function ach_render_obj_meta(&$obj) {
		global $_CONF;

		if($obj->isdone()) {
			$col = "#71BE02";
			$grey = "";
		}
		else {
			$col = "#999999";
			$grey = "grey/";
		}

		return "<table cellspacing='0' cellpadding='0'>
				<tr>
					<td><img src='".$_CONF['image_url']."pic/icon/".$grey.$obj->getMetaImage()."' width='20px' /></td>
					<td valign='middle'><span style='color:".$col.";'>&nbsp;".$obj->getDisplayName()."</span></td>
				</tr>
			</table>";
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

		$html .= ach_render_progressbar($obj->getProgress(),$obj->getValue(),350);
		
		return $html;
	}

	function ach_render_progressbar($prog,$val,$width) {
		$hero = false;
		if($val == false) {
			$hero = true;
			$val = $prog;
		}
		$val = max(1,$val);
		$left = floor($width*(100*($prog/$val))/100);

		$html = "
		<table width='".$width."px' cellspacing='0' cellpadding='0' style='border:1px solid #FFFFFF;color:#000000;'>
			<tr>
				<td bgcolor='#66CC00' width='".$left."px' align='right'>";
				if($hero == false) {
					if(($prog/$val) > 0.5) {
						$html .= "&nbsp;".nf($prog)." / ".nf($val)."&nbsp;";
					}
					$html .= "</td>
					<td align='left' style='color:#FFFFFF;'>";
					if(($prog/$val) <= 0.5) {
						$html .= "&nbsp;".nf($prog)." / ".nf($val)."&nbsp;";
					}
				}
				else {
					$html .= "&nbsp;".nf($prog)."&nbsp;";
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

			$html .= "<td width='50%' align='center'>".$elem[0]."<br>";
			if($elem[3] == 0) {
				$html .= ach_render_progressbar($elem[1],$elem[2],200);
			}
			else {
				$html .= ach_render_progressbar($elem[1],false,200);
			}
			$html .= "</td>";
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
?>