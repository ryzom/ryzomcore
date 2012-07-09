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
		#$sz = $menu->getSize();
		#for($i=0;$i<$sz;$i++) {
		#	$curr = $menu->getChild($i);
			if($curr->inDev()) {
				#continue;
			}
			$html .= "<span class='ach_mspan'><a href='?mode=ach&cat=".$curr->getID()."'><table class='ach_menu'>
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

	function adm_render_category(&$cat) {
		$html = "";

		if($_REQUEST['confirm'] == "delete") {
			$tmp = $cat->getElementByPath($_REQUEST['id']);
			if($tmp != null) {
				$html .= "<div style='display:block;background-color:#FFFFFF;padding:3px;margin-bottom:5px;color:#000000;'>
				<fieldset>
				<legend>Delete</legend>
					Are you sure you want to delete <b>".$tmp->getName()."</b><p>
					<b>Any nested Perks/Objective/Atoms will be removed, as well as any player progress connected!</b>
					<p>
					<a href='?mode=ach&cat=".$_REQUEST['cat']."&act=delete&id=".$_REQUEST['id']."'><b>delete</b></a>
				</fieldset>
				</div>";
			}
		}

		$html .= "<div style='display:block;background-color:#FFFFFF;padding:3px;margin-bottom:5px;color:#000000;'>
			<div style='display:block;text-align:right;'>
				<a href='javascript:hs(\"new_ach\",\"block\");'>
					<img src='pic/b_insrow.png'>
				</a>
			</div>
				<div id='new_ach' style='display: none;'>
					<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&act=ach_insert'>
						<fieldset>
							<legend>add new achievement</legend>
							<table>
								<tr>
									<td>name:</td>
									<td><input type='text' name='aal_name' /></td>
								</tr>
								<tr>
									<td>naming template:</td>
									<td><input type='text' name='aal_template' /></td>
								</tr>
								<tr>
									<td>cult:</td>
									<td>
										<select name='aa_tie_cult'>
											<option value='null' selected='selected'>any</option>
											<option value='c_neutral'>neutral</option>
											<option value='c_kami'>Kami</option>
											<option value='c_karavan'>Karavan</option>
										</select>
									</td>
								</tr>
								<tr>
									<td>civilization:</td>
									<td>
										<select name='aa_tie_civ'>
											<option value='null' selected='selected'>any</option>
											<option value='c_neutral'>neutral</option>
											<option value='c_fyros'>Fyros</option>
											<option value='c_matis'>Matis</option>
											<option value='c_tryker'>Tryker</option>
											<option value='c_zorai'>Zorai</option>
										</select>
									</td>
								</tr>
								<tr>
									<td>image:</td>
									<td><input type='text' name='aa_image' /></td>
								</tr>
								<tr>
									<td colspan='2'><hr /></td>
								</tr>
								<tr>
									<td>perk name:</td>
									<td><input type='text' name='apl_name' /></td>
								</tr>
								<tr>
									<td>naming template:</td>
									<td><input type='text' name='apl_template' /></td>
								</tr>
								<tr>
									<td>perk yubopoints:</td>
									<td><input type='text' name='ap_value' /></td>
								</tr>
								<tr>
									<td>condition:</td>
									<td>
										<select name='ap_condition'>
											<option value='all' selected='selected'>all</option>
											<option value='any'>any</option>
											<option value='value'>by value</option>
										</select>
									</td>
								</tr>
								<tr>
									<td>condition value:</td>
									<td><input type='text' name='ap_condition_value' /></td>
								</tr>
								<tr>
									<td colspan='2'><input type='submit' value='create' /></td>
								</tr>
							</table>
						</fieldset>

					</form>
				</div>
			</div>";

		if($cat->isTiedCultDev() || $cat->isTiedCivDev()) {
			$html .= ach_render_tiebar($cat->getCurrentCult(),$cat->getCurrentCiv(),$cat);
		}

		/*$iter = $cat->getDone();
		while($iter->hasNext()) {
			$curr = $cat->getChildByIdx($iter->getNext());
		#$sz = sizeof($tmp);
		#for($i=0;$i<$sz;$i++) {
			#echo "A";
			if($curr->inDev()) {
				continue;
			}
			$html .= ach_render_achievement_done($curr);
		}*/

		$iter = $cat->getOpen();
		while($iter->hasNext()) {
			$curr = $iter->getNext();
		#$sz = sizeof($tmp);
		#for($i=0;$i<$sz;$i++) {
			#echo "B";
			if($curr->inDev()) {
				#continue;
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
							$html .= ach_render_perk_done($ach);
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
		global $_CONF,$menu;

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
									<td width="100%"><center><table><tr><td><span style="font-weight:bold;font-size:24px;color:#FFFFFF;">[ach:]'.$ach->getName().'</span></td>';
					
					$html .= "<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='?mode=ach&cat=".$_REQUEST['cat']."&act=dev&state=".$ach->getDev()."&id=".$ach->getPathID()."'><img src='pic/";
					if($ach->inDev()) {
						$html .= "red";
					}
					else {
						$html .= "green";
					}
					$html .= ".gif' /></a>&nbsp;<a href='javascript:hs(\"edit_ach_".$ach->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					$html .= "&nbsp;<a href='javascript:hs(\"new_perk_".$ach->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";

					$html .= "&nbsp;<a href='javascript:hs(\"opts_ach_".$ach->getID()."\",\"block\");'><img src='pic/b_tblops.png'></a>";
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=ach&cat=".$_REQUEST['cat']."&confirm=delete&id=".$ach->getPathID()."'><img src='pic/b_drop.png'></a></nobr></td>
									</td></tr></table>";

									$html .= '</center></td><td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#FFFFFF;padding-right:10px;"><!-- 
										'.$ach->getValueOpen().'<br> --><img src="'.$_CONF['image_url'].'pic/yubo_pending.png">
									</td>
								</tr><tr><td align="center" valign="top">';

							$html .= "<div id='edit_ach_".$ach->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
								<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$ach->getID()."&act=ach_update'>
									<fieldset>
										<legend>edit achievement</legend>
										<table>
											<tr>
												<td>name:</td>
												<td><input type='text' name='aal_name' value=\"".$ach->getName()."\" /></td>
											</tr>
											<tr>
												<td>naming template:</td>
												<td><input type='text' name='aal_template' value=\"".$ach->getTemplate()."\" /></td>
											</tr>
											<tr>
												<td>cult:</td>
												<td>
													<select name='aa_tie_cult'>
														<option value='null'"; if($ach->getTieCult() == null) { $html .= " selected='selected'"; } $html .= ">any</option>
														<option value='c_neutral'"; if($ach->getTieCult() == "c_neutral") { $html .= " selected='selected'"; } $html .= ">neutral</option>
														<option value='c_kami'"; if($ach->getTieCult() == "c_kami") { $html .= " selected='selected'"; } $html .= ">Kami</option>
														<option value='c_karavan'"; if($ach->getTieCult() == "c_karavan") { $html .= " selected='selected'"; } $html .= ">Karavan</option>
													</select>
												</td>
											</tr>
											<tr>
												<td>civilization:</td>
												<td>
													<select name='aa_tie_civ'>
														<option value='null'"; if($ach->getTieCiv() == null) { $html .= " selected='selected'"; } $html .= ">any</option>
														<option value='c_neutral'"; if($ach->getTieCiv() == "c_neutral") { $html .= " selected='selected'"; } $html .= ">neutral</option>
														<option value='c_fyros'"; if($ach->getTieCiv() == "c_fyros") { $html .= " selected='selected'"; } $html .= ">Fyros</option>
														<option value='c_matis'"; if($ach->getTieCiv() == "c_matis") { $html .= " selected='selected'"; } $html .= ">Matis</option>
														<option value='c_tryker'"; if($ach->getTieCiv() == "c_tryker") { $html .= " selected='selected'"; } $html .= ">Tryker</option>
														<option value='c_zorai'"; if($ach->getTieCiv() == "c_zorai") { $html .= " selected='selected'"; } $html .= ">Zorai</option>
													</select>
												</td>
											</tr>
											<tr>
												<td>image:</td>
												<td><input type='text' name='aa_image' value='".$ach->getImage()."' /></td>
											</tr>
											<tr>
												<td colspan='2'><input type='submit' value='save' /></td>
											</tr>
										</table>
									</fieldset>
								</form>
							</div>";

							$html .= "<div id='new_perk_".$ach->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
								<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$ach->getID()."&act=perk_insert'>
									<fieldset>
										<legend>add new perk</legend>
										<table>
											<tr>
												<td>name:</td>
												<td><input type='text' name='apl_name' /></td>
											</tr>
											<tr>
												<td>naming template:</td>
												<td><input type='text' name='apl_template' /></td>
											</tr>
											<tr>
												<td>yubopoints:</td>
												<td><input type='text' name='ap_value' /></td>
											</tr>
											<tr>
												<td>parent:</td>
												<td>
													<select name='ap_parent'>
														<option value='null' selected='selected'>[set as main perk]</option>";
														$iter = $ach->getOpen();
														while($iter->hasNext()) {
															$curr = $iter->getNext();
															$html .= "<option value='".$curr->getID()."'>".$curr->getName()."</option>";
														}

													$html .= "</select>
												</td>
											</tr>
											<tr>
												<td>condition:</td>
												<td>
													<select name='ap_condition'>
														<option value='all' selected='selected'>all</option>
														<option value='any'>any</option>
														<option value='value'>by value</option>
													</select>
												</td>
											</tr>
											<tr>
												<td>condition value:</td>
												<td><input type='text' name='ap_condition_value' /></td>
											</tr>
											<tr>
												<td colspan='2'><input type='submit' value='add' /></td>
											</tr>
										</table>
									</fieldset>
								</form>
							</div>";

							$html .= "<div id='opts_ach_".$ach->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
								<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$ach->getID()."&act=ach_move'>
									<fieldset>
										<legend>move achievement</legend>
										<table>
											<tr>
												<td>new category:</td>
												<td>
													<select name='new_cat'>";
														$iter = $menu->getIterator();
														while($iter->hasNext()) {
															$curr = $iter->getNext();
															$html .= "<option value='".$curr->getID()."'>".$curr->getName()."</option>";
															
															$iter2 = $curr->getIterator();
															while($iter2->hasNext()) {
																$curr2 = $iter2->getNext();
																$html .= "<option value='".$curr2->getID()."'>&nbsp;&nbsp;&nbsp;&nbsp;".$curr2->getName()."</option>";
															}
														}

													$html .= "</select>
												</td>
											</tr>
											<tr>
												<td colspan='2'><input type='submit' value='move' /></td>
											</tr>
										</table>
									</fieldset>
								</form>
							</div>";

							$html .= ach_render_perk_open($ach);
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

	function ach_render_perk_open(&$ach) {
		#echo var_export($perk_list,true);
		$html = "";

		$perk_list = $ach->getOpen();
		while($perk_list->hasNext()) {

			$perk = $perk_list->getNext();

			#$perk = $ach->getChild($perk_list[0]);

			if($perk->inDev()) {
				#return $html;
			}
			
			#if($perk->getName() != null) {
				$html .= "<table><tr><td><span style='color:#999999;font-weight:bold;display:block;'>[perk:]".$perk->getDisplayName()."</span></td>";

				$html .= "<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='?mode=ach&cat=".$_REQUEST['cat']."&act=dev&state=".$perk->getDev()."&id=".$perk->getPathID()."'><img src='pic/";
					if($perk->inDev()) {
						$html .= "red";
					}
					else {
						$html .= "green";
					}
					$html .= ".gif' /></a>&nbsp;<a href='javascript:hs(\"edit_perk_".$perk->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					$html .= "&nbsp;<a href='javascript:hs(\"new_obj_".$perk->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=ach&cat=".$_REQUEST['cat']."&confirm=delete&id=".$perk->getPathID()."'><img src='pic/b_drop.png'></a></nobr></td>
									</td></tr></table>";

					$html .= "<div id='edit_perk_".$perk->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
						<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$perk->getPathID()."&act=perk_update'>
							<fieldset>
								<legend>edit perk</legend>
								<table>
									<tr>
										<td>name:</td>
										<td><input type='text' name='apl_name' value='".$perk->getName()."' /></td>
									</tr>
									<tr>
										<td>naming template:</td>
										<td><input type='text' name='apl_template' value=\"".$perk->getTemplate()."\" /></td>
									</tr>
									<tr>
										<td>yubopoints:</td>
										<td><input type='text' name='ap_value' value='".$perk->getValue()."' /></td>
									</tr>
									<tr>
										<td>parent:</td>
										<td>
											<select name='ap_parent'>
												<option value='null' selected='selected'>[set as main perk]</option>";
												$par = $perk->getParent();
												$iter = $par->getOpen();
												while($iter->hasNext()) {
													$curr = $iter->getNext();
													if($curr->getID() == $perk->getID()) {
														continue;
													}
													
													$html .= "<option value='".$curr->getID()."'";
													if($curr->getID() == $perk->getParentID()) {
														$html .= " selected='selected'";
													}
													$html .= ">".$curr->getName()."</option>";
												}

											$html .= "</select>
										</td>
									</tr>
									<tr>
										<td>condition:</td>
										<td>
											<select name='ap_condition'>
												<option value='all'"; if($perk->getCondition() == "all") { $html .= " selected='selected'"; } $html .= ">all</option>
												<option value='any'"; if($perk->getCondition() == "any") { $html .= " selected='selected'"; } $html .= ">any</option>
												<option value='value'"; if($perk->getCondition() == "value") { $html .= " selected='selected'"; } $html .= ">by value</option>
											</select>
										</td>
									</tr>
									<tr>
										<td>condition value:</td>
										<td><input type='text' name='ap_condition_value' value='".$perk->getConditionValue()."' /></td>
									</tr>
									<tr>
										<td colspan='2'><input type='submit' value='save' /></td>
									</tr>
								</table>
							</fieldset>
						</form>
					</div>";

					$html .= "<div id='new_obj_".$perk->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
						<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$perk->getPathID()."&act=obj_insert'>
							<fieldset>
								<legend>add new objective</legend>
								<table>
									<tr>
										<td>name:</td>
										<td><input type='text' name='aol_name' /></td>
									</tr>
									<tr>
										<td>type:</td>
										<td>
											<select name='ao_display'>
												<option value='simple' selected='selected'>simple</option>
												<option value='hidden'>hidden</option>
												<option value='value'>value / progressbar</option>
												<option value='meta'>meta</option>
											</select>
										</td>
									</tr>
									<tr>
										<td>trigger condition:</td>
										<td>
											<select name='ao_condition'>
												<option value='simple' selected='selected'>require all</option>
												<option value='hidden'>require any</option>
												<option value='value'>value / progressbar</option>
											</select>
										</td>
									</tr>
									<tr>
										<td>trigger value:</td>
										<td><input type='text' name='ao_value' /></td>
									</tr>
									<tr>
										<td>metalink:</td>
										<td></td>
									</tr>
									<tr>
										<td colspan='2'><input type='submit' value='add' /></td>
									</tr>
								</table>
							</fieldset>
						</form>
					</div>";
			#}
			#if($perk->objDrawable()) {
				$html .= ach_render_obj_list($perk->getIterator());
			#}
		}

		return $html;
	}

	function ach_render_perk_done(&$ach) {
		global $_CONF;
		$html = "";

		$perk_list = $ach->getDone();
		while($perk_list->hasNext()) {
			$perk = $perk_list->getNext();
		#foreach($perk_list as $elem) {
			#$perk = $ach->getChild($elem);
			if($perk->inDev()) {
				continue;
			}
			$html .= "<div style='display:block;'><span style='color:#66CC00;font-weight:bold;'>".$perk->getName()."</span> ( ".date('d.m.Y',$perk->getDone())." ) <img src='".$_CONF['image_url']."pic/yubo_done.png' width='15px' /> ".$perk->getValue()."</div>";
		}

		return $html;
	}

	function ach_render_obj_list($obj) {
		$html = "<center><table width='90%'>";

		#$i = 0;
		#$skip = false;
		
		while($obj->hasNext()) {
		#foreach($obj as $elem) {
			$elem = $obj->getNext();
			#if(($i%2) == 0) {
				$html .= "<tr><td><table><tr>";
			#}

			switch($elem->getDisplay()) {
				case "meta":
					$html .= "<td>".ach_render_obj_meta($elem)."<td>";
					break;
				case "value":
					#if(($i%2) == 1) {
					#	$html .= "</tr><tr>";
					#}
					$html .= "<td>".ach_render_obj_value($elem)."</td>";
					#$i++;
					break;
				case "simple":
					$html .= "<td>".ach_render_obj_simple($elem)."</td>";
					break;
				case "hidden":
				default:
					//do nothing
					#$skip = true;
					#if(($i%2) == 1) {
					#	$html .= "</tr><tr>";
					#}
					$html .= "<td>".ach_render_obj_hidden($elem)."</td>";
					#$i++;
					break;
			}

			$html .= "<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='javascript:hs(\"edit_obj_".$elem->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					#$html .= "&nbsp;<a href='javascript:hs(\"edit_obj_".$elem->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=ach&cat=".$_REQUEST['cat']."&confirm=delete&id=".$elem->getPathID()."'><img src='pic/b_drop.png'></a></nobr></td>
									</td></tr></table>";

									#$perk = $elem->getParent();

			$html .= "<div id='edit_obj_".$elem->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
						<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$elem->getPathID()."&act=obj_update'>
							<fieldset>
								<legend>edit objective</legend>
								<table>
									<tr>
										<td>name:</td>
										<td><input type='text' name='aol_name' value='".$elem->getName()."' /></td>
									</tr>
									<tr>
										<td>type:</td>
										<td>
											<select name='ao_display'>
												<option value='simple'"; if($elem->getDisplay() == "simple") { $html .= " selected='selected'"; } $html .= ">simple</option>
												<option value='hidden'"; if($elem->getDisplay() == "hidden") { $html .= " selected='selected'"; } $html .= ">hidden</option>
												<option value='value'"; if($elem->getDisplay() == "value") { $html .= " selected='selected'"; } $html .= ">value / progressbar</option>
												<option value='meta'"; if($elem->getDisplay() == "meta") { $html .= " selected='selected'"; } $html .= ">meta</option>
											</select>
										</td>
									</tr>
									<tr>
										<td>trigger condition:</td>
										<td>
											<select name='ao_condition'>
												<option value='simple'"; if($elem->getCondition() == "simple") { $html .= " selected='selected'"; } $html .= ">require all</option>
												<option value='hidden'"; if($elem->getCondition() == "hidden") { $html .= " selected='selected'"; } $html .= ">require any</option>
												<option value='value'"; if($elem->getCondition() == "value") { $html .= " selected='selected'"; } $html .= ">value / progressbar</option>
											</select>
										</td>
									</tr>
									<tr>
										<td>trigger value:</td>
										<td><input type='text' name='ao_value' value='".$elem->getValue()."' /></td>
									</tr>
									<tr>
										<td>metalink:</td>
										<td></td>
									</tr>
									<tr>
										<td colspan='2'><input type='submit' value='save' /></td>
									</tr>
								</table>
							</fieldset>
						</form>
					</div>";

			#if(($i%2) == 1) {
				$html .= "</td></tr>";
			#}
			
			
			#if(!$skip) {
			#	$i++;
			#}
			#$skip = false;
		}

		#if(($i%2) == 1) {
		#	$html .= "</tr>";
		#}

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
		
		$html .= "[obj:]".$obj->getDisplayName()."</span>";

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

		return "<table cellspacing='0' cellpadding='0'>
				<tr>
					<td><img src='".$_CONF['image_url']."pic/icon/".$grey.$obj->getMetaImage()."' width='20px' /></td>
					<td valign='middle'><span style='color:".$col.";'>&nbsp;[obj:]".$obj->getDisplayName()."</span></td>
				</tr>
			</table>";
	}

	function ach_render_obj_value(&$obj) {
		$html = "";
		#if($obj->getName() != null) {
			if($obj->isdone()) {
				$col = "#71BE02";
			}
			else {
				$col = "#999999";
			}
			$html .= "<div style='color:".$col.";display:block;'>[obj:]".$obj->getDisplayName()."</div>";
		#}

		$html .= ach_render_progressbar($obj->getProgress(),$obj->getValue(),350);
		
		return $html;
	}

	function ach_render_obj_hidden(&$obj) {
		$html = "";
		#if($obj->getName() != null) {
			if($obj->isdone()) {
				$col = "#71BE02";
			}
			else {
				$col = "#999999";
			}
			$html .= "<div style='color:".$col.";display:block;'>[obj: untitled]</div>";
		#}

		#$html .= ach_render_progressbar($obj->getProgress(),$obj->getValue(),350);
		
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


	function ach_render_tiebar($cult = "c_neutral", $civ = "c_neutral",&$cat) {
		global $_USER,$_CONF;

		$html = "<style>
			.o {
				color:orange;
			}
		</style>

		<div style='display:block;text-align:center;'><form method='post' action='?cat=".$cat->getID()."' id='cc_form'>
			<table>
				<tr>";
				if($cat->isTiedCult()) {
					$html.= "<td>
						<select name='cult' onchange='document.getElementById(\"cc_form\").submit();'>
							<option value='c_neutral'"; if($cult == "c_neutral") { $html.= " selected='selected'"; } $html .= ">".get_translation('ach_c_neutral',$_USER->getLang())."</option>
							<option value='c_kami'"; if($cult == "c_kami") { $html.= " selected='selected'"; } $html .= ">Kami</option>
							<option value='c_karavan'"; if($cult == "c_karavan") { $html.= " selected='selected'"; } $html .= ">Karavan</option>
						</select>
					</td>";
				}
				if($cat->isTiedCiv()) {
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

		if($cat->isTiedCult() && !$cat->isTiedCiv() && $cult == "c_neutral") { // neutral / xx
			#While being of neutral allegiance with the higher powers
			$html .= get_translation('ach_allegiance_neutral_cult',$_USER->getLang(),array("<span class='o'>".get_translation('ach_c_neutral',$_USER->getLang())."</span>"));
		}
		elseif($cat->isTiedCiv() && !$cat->isTiedCult() && $civ == "c_neutral") { // xx / neutral
			#While being of neutral allegiance with the homin civilizations
			$html .= get_translation('ach_allegiance_neutral_civ',$_USER->getLang(),array("<span class='o'>".get_translation('ach_c_neutral',$_USER->getLang())."</span>"));
		}
		elseif($cat->isTiedCiv() && $cat->isTiedCult() && $cult == "c_neutral" && $civ == "c_neutral") { // neutral / neutral
			#While being of neutral allegiance
			$html .= get_translation('ach_allegiance_neutral',$_USER->getLang(),array("<span class='o'>".get_translation('ach_c_neutral',$_USER->getLang())."</span>"));
		}
		else { //other
			#While being aligned with the
			$html .= get_translation('ach_allegiance_start',$_USER->getLang());
			if($cat->isTiedCult() && $cult != "c_neutral") {
				#CULT
				$html .= "<span class='o'>".ach_translate_cc($cult)."</span>";
				if($cat->isTiedCiv() && $civ != "c_neutral") {
					#and the CIV
					$html .= get_translation('ach_allegiance_and',$_USER->getLang())." <span class='o'>".ach_translate_cc($civ)."</span>";
				}
			}
			elseif($cat->isTiedCiv() && $civ != "c_neutral") {
				#CIV
				$html .= "<span class='o'>".ach_translate_cc($civ)."</span>";
			}
		}
		#, accomplish the following achievements:
		$html .= get_translation('ach_allegiance_end',$_USER->getLang())."</div>";

		return $html;
	}
?>