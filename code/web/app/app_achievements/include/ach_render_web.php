<?php

	function ach_render_tiebar($cult = "neutral", $civ = "neutral") {

	}

	function ach_render_menu(&$menu,$sub = 0) {
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

		$sz = $menu->getSize();
		for($i=0;$i<$sz;$i++) {
			$curr = $menu->getChild($i);
			$html .= "<span class='ach_mspan'><a href='?lang=en&cat=".$curr->getID()."'><table class='ach_menu'>
				<tr>";
					if($sub == 0) {
						$html .= "<td><img src='pic/menu/test.png' /></td>";
					}
					$html .= "<td style='font-size:".(20-$sub)."px;font-weight:bold;";
					if($curr->isOpen()) {
						$html .= "color:orange;";
					}
					$html .= "'>".$curr->getName()."</td>
				</tr>
			</table></a></span>";
			if($curr->hasOpenCat() != 0) {
				$html .= "<div style='display:block;margin-left:25px;'>".ach_render_menu($curr,($sub+4))."</div>";
			}
		}

		return $html;
	}

	function ach_render_category(&$cat) {
		$html = "";

		$tmp = $cat->getDone();
		$sz = sizeof($tmp);
		for($i=0;$i<$sz;$i++) {
			#echo "A";
			$html .= ach_render_achievement_done($cat->getChild($tmp[$i]));
		}

		$tmp = $cat->getOpen();
		$sz = sizeof($tmp);
		for($i=0;$i<$sz;$i++) {
			#echo "B";
			$html .= ach_render_achievement_open($cat->getChild($tmp[$i]));
		}

		return $html;
	}

	function ach_render_achievement_done(&$ach) {
		$html = "";

		$html .= '<div style="display: block; margin-bottom: 5px;"><table cellpadding="0" cellspacing="0" width="100%">
					<tbody><tr>
						<td width="3px"><img src="pic/bar_done_ul.png"></td>
						<td style="background-image: url(pic/bar_done_u.png);"></td>
						<td width="3px"><img src="pic/bar_done_ur.png"></td>
					</tr>
					<tr>
						<td style="background-image: url(pic/bar_done_l.png);"></td>
						<td style="background-image: url(pic/bar_done_bg.png);">
							<center><table width="100%" cellspacing="0" cellpadding="0">
								<tbody><tr>
									<td rowspan="2" valign="top"><img src="pic/icon/test.png"></td>
									<td width="100%"><center><span style="font-weight:bold;font-size:24px;color:#000000;">'.$ach->getName().'</span></center></td>
									<td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#000000;padding-right:10px;">
										'.$ach->getValueDone().'<br><img src="pic/yubo_done.png">
									</td>
								</tr><tr><td align="center" valign="top">';
							$html .= ach_render_perk_done($ach);
							$html .= '</td></tr></tbody></table></center>
						</td>
						<td style="background-image: url(pic/bar_done_r.png);"></td>
					</tr>
					<tr>
						<td><img src="pic/bar_done_bl.png"></td>
						<td style="background-image: url(pic/bar_done_b.png);"></td>
						<td><img src="pic/bar_done_br.png"></td>
					</tr>
				</tbody></table></div>';

		return $html;
	}

	function ach_render_achievement_open(&$ach) {
		$html = "";

		$html .= '<div style="display: block; margin-bottom: 5px;"><table cellpadding="0" cellspacing="0" width="100%">
					<tbody><tr>
						<td width="3px"><img src="pic/bar_pending_ul.png"></td>
						<td style="background-image: url(pic/bar_pending_u.png);"></td>
						<td width="3px"><img src="pic/bar_pending_ur.png"></td>
					</tr>
					<tr>
						<td style="background-image: url(pic/bar_pending_l.png);"></td>
						<td>
							<center><table width="100%" cellspacing="0" cellpadding="0">
								<tbody><tr>
									<td rowspan="2" valign="top"><img src="pic/icon/test.png"></td>
									<td width="100%"><center><span style="font-weight:bold;font-size:24px;color:#FFFFFF;">'.$ach->getName().'</span></center></td>
									<td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#FFFFFF;padding-right:10px;">
										'.$ach->getValueOpen().'<br><img src="pic/yubo_pending.png">
									</td>
								</tr><tr><td align="center" valign="top">';
							$html .= ach_render_perk_open($ach);
							$html .= '</td></tr></tbody></table></center>
						</td>
						<td style="background-image: url(pic/bar_pending_r.png);"></td>
					</tr>
					<tr>
						<td><img src="pic/bar_pending_bl.png"></td>
						<td style="background-image: url(pic/bar_pending_b.png);"></td>
						<td><img src="pic/bar_pending_br.png"></td>
					</tr>
				</tbody></table></div>';

		return $html;
	}

	function ach_render_perk_open(&$ach) {
		#echo var_export($perk_list,true);
		$html = "";

		$perk_list = $ach->getOpen();

		$perk = $ach->getChild($perk_list[0]);

		$html .= "<span style='color:#999999;font-weight:bold;'>".$perk->getName()."</span>";
		if($perk->objDrawable()) {
			$html .= "<br>".ach_render_obj_list($perk->getChildren());
		}

		return $html;
	}

	function ach_render_perk_done(&$ach) {
		$html = "";

		$perk_list = $ach->getDone();

		foreach($perk_list as $elem) {
			$perk = $ach->getChild($elem);
			$html .= "<div style='display:block;'><span style='color:#66CC00;font-weight:bold;'>".$perk->getName()."</span> ( ".date('d.m.Y',$perk->getDone())." ) <img src='pic/yubo_done.png' width='15px' /> ".$perk->getValue()."</div>";
		}
		/*if($perk->objDrawable()) {
			$html .= "<br>".ach_render_obj_list($perk->getChildren());
		}*/

		return $html;
	}

	function ach_render_obj_list(&$obj) {
		$html = "<center><table width='90%'>";

		$i = 0;
		$skip = false;

		foreach($obj as $elem) {
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
		$html = "";
		if($obj->isdone()) {
			$html .= "<img src='pic/check.png' height='10px' />&nbsp;<span style='color:#71BE02;'>";
		}
		else {
			$html .= "<img src='pic/pending.png' height='10px' />&nbsp;<span style='color:#999999;'>";
		}
		
		$html .= $obj->getName()."</span>";

		return $html;
	}

	function ach_render_obj_meta(&$obj) {
		$html = "";
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
					<td><img src='pic/icon/".$grey."test.png' width='20px' /></td>
					<td valign='middle'><span style='color:".$col.";'>&nbsp;".$obj->getName()."</span></td>
				</tr>
			</table>";
	}

	function ach_render_obj_value(&$obj) {
		$html = "";
		if($obj->isdone()) {
			$col = "#71BE02";
		}
		else {
			$col = "#999999";
		}
		$html .= "<div style='color:".$col.";display:block;'>".$obj->getName()."</div>";

		$html .= ach_render_progressbar($obj->getProgress(),$obj->getValue(),350);
		
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
					$html .= "&nbsp;".$prog." / ".$val."&nbsp;";
				}
				$html .= "</td>
				<td align='left' style='color:#FFFFFF;'>";
				if(($prog/$val) <= 0.85) {
					$html .= "&nbsp;".$prog." / ".$val."&nbsp;";
				}
				$html .= "</td>
			</tr>
		</table>";
		
		return $html;
	}

	function ach_render_summary_header($lang) {
		return "<div style='display:block;font-weight:bold;font-size:30px;color:#FFFFFF;text-align:center;margin-bottom:10px;'>".get_translation('ach_summary_header',$lang)."</div>";
	}

	function ach_render_summary_footer($lang,&$summary,$user) {
		$nodes = $summary->getSummary($lang,$user);
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
		<div style='display:block;font-weight:bold;font-size:30px;color:#FFFFFF;text-align:center;margin-bottom:10px;'>".get_translation('ach_summary_stats',$lang)."</div>
		<table>
			<tr>
				<td colspan='3' align='center'>".get_translation('ach_summary_stats_total',$lang)."<br>".ach_render_progressbar($sum_done,$sum_total,450)."<br></td>
			</tr>
			".$html."
		</table>";

		return $html;
	}
?>