<?php
	function ach_render() {
		global $_USER,$_CONF;

		$c = "<table>
			<tr>
				<td>".ach_render_yubopoints()."</td>
			</tr>
		</table>
			
		<table>
			<tr>
				<td width='230px'>";
				
					$menu = new AchMenu($_REQUEST['cat']);

					$c .= ach_render_menu($menu);
				
				$c .= "</td>
				<td width='455px'>";

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
						$cat = new AchSummary($menu,$_CONF['summary_size']);
						$c .= ach_render_summary_header();
					}

					$c .= ach_render_category($cat);
					if($open == 0) {
						$c .= ach_render_summary_footer($cat);
					}

				$c .= "</td>
					</tr>
				</table>";

		return $c;
	}

	function ach_render_tiebar($cult = "c_neutral", $civ = "c_neutral",&$cat) {
		global $_USER;

		$html = "<form method='post' action='?cat=".$cat->getID()."'>
			<table>
				<tr>";
				if($cat->isAllowedCult()) {
					$html.= "<td>
						<select name='cult'>
							<option value='c_neutral'"; if($cult == "c_neutral") { $html.= " selected='selected'"; } $html .= ">".get_translation('ach_c_neutral',$_USER->getLang())."</option>
							<option value='c_kami'"; if($cult == "c_kami") { $html.= " selected='selected'"; } $html .= ">Kami</option>
							<option value='c_karavan'"; if($cult == "c_karavan") { $html.= " selected='selected'"; } $html .= ">Karavan</option>
						</select>
					</td>";
				}
				if($cat->isAllowedCiv()) {
					$html.= "<td>
						<select name='civ'>
							<option value='c_neutral'"; if($civ == "c_neutral") { $html.= " selected='selected'"; } $html .= ">".get_translation('ach_c_neutral',$_USER->getLang())."</option>
							<option value='c_fyros'"; if($civ == "c_fyros") { $html.= " selected='selected'"; } $html .= ">Fyros</option>
							<option value='c_matis'"; if($civ == "c_matis") { $html.= " selected='selected'"; } $html .= ">Matis</option>
							<option value='c_tryker'"; if($civ == "c_tryker") { $html.= " selected='selected'"; } $html .= ">Tryker</option>
							<option value='c_zorai'"; if($civ == "c_zorai") { $html.= " selected='selected'"; } $html .= ">Zorai</option>
						</select>
					</td>";
				}
				$html.= "<td><input type='submit' value='show' /></td></tr><tr><td>&nbsp;</td></tr>
			</table>
		</form>
		
		<font size='12px' color='#FFFFFF'>";

		/*if($cat->isTiedCult() && !$cat->isTiedCiv() && $cult == "c_neutral") { // neutral / xx
			#While being of neutral allegiance with the higher powers
			$html .= get_translation('ach_allegiance_neutral_cult',$_USER->getLang(),array("<font color='orange'>".get_translation('ach_c_neutral',$_USER->getLang())."</font>"));
		}
		elseif($cat->isTiedCiv() && !$cat->isTiedCult() && $civ == "c_neutral") { // xx / neutral
			#While being of neutral allegiance with the homin civilizations
			$html .= get_translation('ach_allegiance_neutral_civ',$_USER->getLang(),array("<font color='orange'>".get_translation('ach_c_neutral',$_USER->getLang())."</font>"));
		}*/
		if(($cult == "c_neutral" || !$cat->isAllowedCult()) && ($civ == "c_neutral" || !$cat->isAllowedCiv())) { // neutral / neutral
			#While being of neutral allegiance
			$html .= get_translation('ach_allegiance_neutral',$_USER->getLang(),array("<font color='orange'>".get_translation('ach_c_neutral',$_USER->getLang())."</font>"));
		}
		else { //other
			#While being aligned with the
			$html .= get_translation('ach_allegiance_start',$_USER->getLang());
			if($cat->isAllowedCult() && $cult != "c_neutral") {
				#CULT
				$html .= "<font color='orange'>".ach_translate_cc($cult)."</font>";
				if($cat->isAllowedCiv() && $civ != "c_neutral") {
					#and the CIV
					$html .= get_translation('ach_allegiance_and',$_USER->getLang())." <font color='orange'>".ach_translate_cc($civ)."</font>";
				}
			}
			elseif($cat->isAllowedCiv() && $civ != "c_neutral") {
				#CIV
				$html .= "<font color='orange'>".ach_translate_cc($civ)."</font>";
			}
		}
		#, accomplish the following achievements:
		$html .= get_translation('ach_allegiance_end',$_USER->getLang())."</font><br><br>";

		return $html;
	}

	function ach_render_yubopoints() {
		global $DBc,$_USER,$_CONF;

		$res = $DBc->sqlQuery("SELECT sum(at_value) as anz FROM ach_task,ach_player_task WHERE at_id=apt_task AND apt_player='".$_USER->getID()."'");

		$html = "<font size='32px'>".$_USER->getName()."&nbsp;<img src='".$_CONF['image_url']."pic/yubo_done.png?cacheid=".$_CONF['image_cdate']."'>&nbsp;".max(0,$res[0]['anz'])."</font>";

		return $html;
	}

	function ach_render_facebook() {

	}

	function ach_render_twitter() {

	}

	function ach_render_menu(&$menu,$sub = 0) {
		global $_CONF;

		$html = "";
		if($sub == 0) {
			$html = "<table cellpadding='2px'>";
		}

		$iter = $menu->getIterator();
		while($iter->hasNext()) {
			$curr = $iter->getNext();
		#$sz = $menu->getSize();
		#for($i=0;$i<$sz;$i++) {
			#$curr = $menu->getChild($i);
			if($curr->inDev()) {
				continue;
			}
			if($sub == 0) {
				$html .= "<tr><td></td><td bgcolor='#000000'></td></tr>";
			}

			$html .= "<tr><td>";
					if($sub == 0) {
						$html .= "<img src='".$_CONF['image_url']."pic/menu/ig_".$curr->getImage()."?cacheid=".$_CONF['image_cdate']."' />";
					}
					else {
						$html .= "<img src='".$_CONF['image_url']."pic/menu_space.png?cacheid=".$_CONF['image_cdate']."' />";
					}
					$html .= "</td><td><a href='?cat=".$curr->getID()."'><font size='".(16-$sub)."px'";
					if($curr->isOpen()) {
						$html .= " color='orange'";
					}
					$html .= "><b>".$curr->getName()."</b></font></a></td>
				</tr>";

			if($curr->hasOpenCat() != 0) {
				$html .= ach_render_menu($curr,($sub+4));
			}
		}
		
		if($sub == 0) {
			$html .= "<tr><td></td><td bgcolor='#000000'></td></tr></table>";
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

		$html = "
		<table>
			<tr>
				<td  width='450px' bgcolor='#D2CBDC88'>
					<table width='450px' cellpadding='3px'>
						<tr>
							<td width='70px'><img src='".$_CONF['image_url']."pic/icon/".$ach->getImage()."?cacheid=".$_CONF['image_cdate']."'></td>
							<td><center><font size='22px'><b>".$ach->getName()."</b></font></center>
								<table>".ach_render_task_done($ach,$cat)."</table>
							</td>
							<td width='35px'>";
		if((!$ach->isHeroic() && !$ach->isContest()) && $ach->getValueDone() > 0) {
			$html .= "<font size='24px' color='#000000'>".$ach->getValueDone()."</font><br><img src='".$_CONF['image_url']."pic/yubo_done.png?cacheid=".$_CONF['image_cdate']."'>";
		}
		else {
			$html .= '<img src="'.$_CONF['image_url'].'pic/star_done.png?cacheid='.$_CONF['image_cdate'].'"><br>';
		}
		$html .= "</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr><td>&nbsp;</td></tr>
		</table>";

		return $html;
	}

	function ach_render_achievement_open(&$ach,&$cat) {
		global $_CONF;

		$html = "
		<table>
			<tr>
				<td  width='450px' bgcolor='#D2CBDC33'>
					<table width='450px' cellpadding='3px'>
						<tr>
							<td width='70px'><img src='".$_CONF['image_url']."pic/icon/grey/".$ach->getImage()."?cacheid=".$_CONF['image_cdate']."'></td>
							<td><center><font size='22px'><b>".$ach->getName()."</b></font></center>
								<table>".ach_render_task_open($ach)."</table>
							</td>
							<td width='35px'>";
		if(!$ach->isHeroic() && !$ach->isContest()) {
			$html .= "<font size='22px' color='#000000'>".$ach->getValueOpen()."</font><br><img src='".$_CONF['image_url']."pic/yubo_pending.png?cacheid=".$_CONF['image_cdate']."'>";
		}
		$html .= "</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr><td>&nbsp;</td></tr>
		</table>";

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
			$html .= "<tr><td><font color='#999999' size='12px'><b>".$task->getDisplayName()."</b></font></td></tr><tr><td>&nbsp;</td></tr>";
		}
		if($task->objDrawable()) {
			$html .= "<tr><td>".ach_render_obj_list($task->getIterator())."</td></tr>";
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
			$html .= "<tr><td><font color='#66CC00'><b>".$task->getDisplayName()."</b></font><br>( ".date('d.m.Y',$task->getDone())." )";
			if($task->getValue() > 0) {
				$html .= " <img src='".$_CONF['image_url']."pic/yubo_done_small.png?cacheid=".$_CONF['image_cdate']."' /> ".$task->getValue();
			}
			$html .= "</td></tr>";
		}

		return $html;
	}

	function ach_render_obj_list($obj) {
		$html = "<table width='90%'>";

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

		$html .= "</table>";

		return $html;
	}

	function ach_render_obj_simple(&$obj) {
		global $_CONF;

		$html = "";
		if($obj->isdone()) {
			$html .= "<table><tr><td><img src='".$_CONF['image_url']."pic/check.png?cacheid=".$_CONF['image_cdate']."' height='10px' /></td><td>&nbsp;<font color='#71BE02;'>";
		}
		else {
			$html .= "<table><tr><td><img src='".$_CONF['image_url']."pic/pending.png?cacheid=".$_CONF['image_cdate']."' height='10px' />&nbsp;<font color='#999999;'></td><td>";
		}
		
		$html .= $obj->getDisplayName()."</font></td></tr></table>";

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
					<td><img src='".$_CONF['image_url']."pic/icon/".$grey."small/".$obj->getMetaImage()."?cacheid=".$_CONF['image_cdate']."' /></td>
					<td>&nbsp;</td>
					<td><font color='".$col."'>".$obj->getDisplayName()."</font></td>
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
			$html .= "<font color='".$col."'>".$obj->getDisplayName()."</font>";
		}

		$html .= ach_render_progressbar($obj->getProgress(),$obj->getValue(),250);
		
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
		$left = max(1,$left);

		$html = "<table width='".($width+12)."px'>
			<tr>
				<td width='10px'></td>
				<td><table cellpadding='1px' width='".($width+2)."px'><tr><td bgcolor='#FFFFFF'>
		<table width='".$width."px' cellspacing='0' cellpadding='0'>
			<tr>
				<td bgcolor='#66CC00' width='".$left."px'><font color='#000000'>";
				if($hero == false) {
					if(($prog/$val) > 0.5) {
						$html .= "&nbsp;".nf($prog)." / ".nf($val)."&nbsp;";
					}
					$html .= "</font></td>
					<td align='left' bgcolor='#00000066'><font color='#FFFFFF'>";
					if(($prog/$val) <= 0.5) {
						$html .= "&nbsp;".nf($prog)." / ".nf($val)."&nbsp;";
					}
				}
				else {
					$html .= "&nbsp;".nf($prog)."&nbsp;";
				}
				$html .= "</font></td>
			</tr>
		</table></td></tr></table></td>
			</tr>
		</table>";
		
		return $html;
	}

	function ach_render_summary_header() {
		global $_USER;

		return "<font size='30px' color='#FFFFFF'>".get_translation('ach_summary_header',$_USER->getLang())."</font>";
	}

	function ach_render_summary_footer(&$summary) {
		global $_USER;

		$nodes = $summary->getSummary();
		$html = "";

		$sum_done = 0;
		$sum_total = 0;

		$i = 0;
		foreach($nodes as $elem) {
			if(($i%2) == 0) {
				$html .= "<tr>";
			}

			$html .= "<td width='225px'>".$elem[0];
			if($elem[3] == false) {
				$html .= ach_render_progressbar($elem[1],$elem[2],150);
			}
			else {
				$html .= ach_render_progressbar($elem[1],false,150);
			}

			$html .= "<br></td>";

			$sum_done += $elem[1];
			$sum_total += $elem[2];

			if(($i%2) == 2) {
				$html .= "</tr>";
			}

			$i++;
		}

		if(($i%2) == 2) {
			$html .= "</tr>";
		}

		$html = "<p />
		<font size='30px' color='#FFFFFF'>".get_translation('ach_summary_stats',$_USER->getLang())."</font>
		<table width='450px'>
			<tr>
				<td width='450px'>".get_translation('ach_summary_stats_total',$_USER->getLang()).ach_render_progressbar($sum_done,$sum_total,350)."<br></td>
			</tr>
			<tr>
				<td width='450px'><table width='450px'>".$html."</table></td>
			</tr>
		</table>";

		return $html;
	}
?>