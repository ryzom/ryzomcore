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
		
		$iter = $menu->getIterator();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			$html .= "<span class='ach_mspan'><a href='?mode=atom&cat=".$curr->getID()."'><table class='ach_menu'>
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
		global $_CONF,$menu;

		$open = explode(";",$_REQUEST['id']);

		$o = "none";
		if($open[1] == $ach->getID()) {
			$o = "block";
		}

		$html = "<div style='display: block; margin-bottom: 5px;'>
			<div style='display:block;font-size:22px;' class='bar'><a href='javascript:hs(\"ach_".$ach->getID()."\",\"block\");'>[+]</a> ".$ach->getName()." <span style='font-size:12px;'>(ties= race: ".$ach->getTieRace()."; civ: ".$ach->getTieCiv()."; cult: ".$ach->getTieCult().")</span></div>
			<div style='margin-left:25px;display:".$o.";' id='ach_".$ach->getID()."'>".ach_render_perk_open($ach)."</div>
		</div>";

		return $html;
	}

	function ach_render_perk_open(&$ach) {
		$html = "";

		$open = explode(";",$_REQUEST['id']);

		$perk_list = $ach->getOpen();
		while($perk_list->hasNext()) {

			$perk = $perk_list->getNext();

			$o = "none";
			if($open[2] == $perk->getID()) {
				$o = "block";
			}

			$html .= "<div style='display: block; margin-bottom: 5px;'>
				<div style='display:block;font-size:16px;' class='bar'><a href='javascript:hs(\"perk_".$perk->getID()."\",\"block\");'>[+]</a> ".$perk->getDisplayName()." <span style='font-size:12px;'>(condition= ".$perk->getCondition().": ".$perk->getConditionValue().")</span></div>
				<div style='margin-left:25px;display:".$o.";' id='perk_".$perk->getID()."'>".ach_render_obj_list($perk->getIterator())."</div>
			</div>";
		}

		return $html;
	}

	function ach_render_obj_list($obj) {
		$html = "";

		$open = explode(";",$_REQUEST['id']);
		
		while($obj->hasNext()) {
			$elem = $obj->getNext();
			
			#$o = "none";
			#if($open[3] == $elem->getID()) {
				$o = "block";
			#}
			
			$html .= "<div style='display: block; margin-bottom: 5px;'>
				<div style='display:block;' class='bar'><a href='javascript:hs(\"obj_".$elem->getID()."\",\"block\");' name='obj_".$elem->getID()."'>[+]</a> ".$elem->getDisplayName()." <span style='font-size:12px;'>(condition= ".$elem->getCondition().": ".$elem->getValue().")</span></div>
				<div style='margin-left:25px;display:".$o.";' id='obj_".$elem->getID()."'>
					<div style='display:block;'><a href='javascript:hs(\"add_atom_".$elem->getID()."\",\"block\");'>add</a></div>
					<div style='display:none;' id='add_atom_".$elem->getID()."'>
						<form method='post' action='?mode=atom&cat=".$_REQUEST['cat']."&act=insert_atom&id=".$elem->getPathID()."#obj_".$elem->getID()."'>
						<fieldset>
						<legend>add atom trigger</legend>
						<table>
							<tr>
								<td><textarea name='atom_ruleset' rows='6' cols='80' onkeydown='return catchTab(this,event);'></textarea></td>
							</tr>
							<tr>
								<td><input type='hidden' name='atom_mandatory' value='0' /><input type='checkbox' name='atom_mandatory' value='1' />&nbsp;mandatory</td>
							</tr>
							<tr>
								<td><input type='submit' value='add' /></td>
							</tr>
						</table>
						</fieldset>
						<hr>
					</form>
					</div>
					".ach_render_atom_list($elem->getIterator())."
				</div>
			</div>";
		}

		return $html;
	}

	function ach_render_atom_list($atom) {
		$html = "";
		
		while($atom->hasNext()) {
			$elem = $atom->getNext();
			
			$html .= "<form method='post' action='?mode=atom&cat=".$_REQUEST['cat']."&act=update_atom&id=".$elem->getPathID()."#obj_".$elem->getObjective()."'>
						<table>
							<tr>
								<td><textarea name='atom_ruleset' rows='6' cols='80' onkeydown='return catchTab(this,event);'>".$elem->getRuleset()."</textarea></td>
								<td valign='top' rowspan='3'><a href='?mode=atom&cat=".$_REQUEST['cat']."&act=delete&id=".$elem->getPathID()."#obj_".$elem->getObjective()."'>[X]</a></td>
							</tr>
							<tr>
								<td><div style='width:555px;overflow:scroll;'><pre>".$elem->getRulesetParsed()."</pre></div></td>
							</tr>
							<tr>
								<td><input type='hidden' name='atom_mandatory' value='0' /><input type='checkbox' name='atom_mandatory' value='1'";
								if($elem->getMandatory() == 1) {
									$html .= " checked='checked'";
								}
								$html .= " />&nbsp;mandatory</td>
							</tr>
							<tr>
								<td colspan='2'><input type='submit' value='edit' /></td>
							</tr>
						</table>
					</form><hr>";
		}

		return $html;
	}
?>