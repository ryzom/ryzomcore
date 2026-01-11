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
		global $menu,$metalist,$DBc,$_USER;

		$html = "";

		$m = $menu->getIterator();
		while($m->hasNext()) {
			$n = $m->getNext();
			$html .= "<option value='null' disabled='disabled'>".$n->getName()."</option>";

			$m2 = $n->getIterator();
			while($m2->hasNext()) {
				$n2 = $m2->getNext();
				$html .= "<option value='null' disabled='disabled'>&nbsp;&nbsp;&nbsp;".$n2->getName()."</option>";

				//db
				$res = $DBc->sqlQuery("SELECT aa_id,aal_name FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$n2->getID()."' ORDER by aa_sticky DESC, aal_name ASC");
				$sz = sizeof($res);
				for($i=0;$i<$sz;$i++) {
					$html .= "<option value='".$res[$i]['aa_id']."'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;".$res[$i]['aal_name']."</option>";
				}
			}

			//db
			$res = $DBc->sqlQuery("SELECT aa_id,aal_name FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$n->getID()."' ORDER by aa_sticky DESC, aal_name ASC");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$html .= "<option value='".$res[$i]['aa_id']."'>&nbsp;&nbsp;&nbsp;".$res[$i]['aal_name']."</option>";
			}
		}

		$metalist = $html;



		$html = "<style>
			.bw {
				background-color:#FFFFFF;
				color:#000000;
			}
		</style>";

		if($_REQUEST['confirm'] == "delete") {
			$tmp = $cat->getElementByPath($_REQUEST['id']);
			if($tmp != null) {
				$html .= "<div style='display:block;padding:3px;margin-bottom:5px;' class='bw'>
				<fieldset>
				<legend>Delete</legend>
					Are you sure you want to delete <b>".$tmp->getName()."</b><p>
					<b>Any nested Tasks/Objective/Atoms will be removed, as well as any player progress connected!</b>
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
									<td class='bw'>name:</td>
									<td><input type='text' name='aal_name' /></td>
								</tr>
								<tr>
									<td class='bw'>naming template:</td>
									<td><input type='text' name='aal_template' /></td>
								</tr>
								<tr>
									<td class='bw'>parent achievement:</td>
									<td>
										<select name='aa_parent'>
											<option value='null' selected='selected'>-- none --</option>";
											$iter = $cat->getOpen();
											while($iter->hasNext()) {
												$item = $iter->getNext();
												$html .= "<option value='".$item->getID()."'>".$item->getName()."</option>";
											}
										$html .= "</select>
									</td>
								</tr>
								<tr>
									<td class='bw'>image:</td>
									<td><input type='text' name='aa_image' /></td>
								</tr>
								<tr>
									<td class='bw'>sticky:</td>
									<td><input type='hidden' value='0' name='aa_sticky' /><input type='checkbox' name='aa_sticky' value='1'/></td>
								</tr>
								<tr>
									<td colspan='2'><hr /></td>
								</tr>
								<tr>
									<td class='bw'>task name:</td>
									<td><input type='text' name='atl_name' /></td>
								</tr>
								<tr>
									<td class='bw'>naming template:</td>
									<td><input type='text' name='atl_template' /></td>
								</tr>
								<tr>
									<td class='bw'>task yubopoints:</td>
									<td><input type='text' name='at_value' /></td>
								</tr>
								<tr>
									<td class='bw'>condition:</td>
									<td>
										<select name='at_condition'>
											<option value='all' selected='selected'>all</option>
											<option value='any'>any</option>
											<option value='value'>by value</option>
										</select>
									</td>
								</tr>
								<tr>
									<td class='bw'>condition value:</td>
									<td><input type='text' name='at_condition_value' /></td>
								</tr>
								<tr>
									<td class='bw'>allegiance:</td>
									<td>
										<select name='at_tie_allegiance[]' multiple='multiple' size='15'>
											<option value='c_neutral|c_neutral'>neutral / neutral</option>
											<option value='c_kami|c_neutral'>Kami / neutral</option>
											<option value='c_karavan|c_neutral'>Karavan / neutral</option>
											<option value='c_neutral|c_fyros'>neutral / Fyros</option>
											<option value='c_kami|c_fyros'>Kami / Fyros</option>
											<option value='c_karavan|c_fyros'>Karavan / Fyros</option>
											<option value='c_neutral|c_matis'>neutral / Matis</option>
											<option value='c_kami|c_matis'>Kami / Matis</option>
											<option value='c_karavan|c_matis'>Karavan / Matis</option>
											<option value='c_neutral|c_tryker'>neutral / Tryker</option>
											<option value='c_kami|c_tryker'>Kami / Tryker</option>
											<option value='c_karavan|c_tryker'>Karavan / Tryker</option>
											<option value='c_neutral|c_zorai'>neutral / Zorai</option>
											<option value='c_kami|c_zorai'>Kami / Zorai</option>
											<option value='c_karavan|c_zorai'>Karavan / Zorai</option>
										</select>
									</td>
								</tr>
								<tr>
									<td colspan='2'><input type='hidden' value='0' name='at_inherit' /><input type='submit' value='create' /></td>
								</tr>
							</table>
						</fieldset>

					</form>
				</div>
			</div>";

		if($cat->hasTieAlignDev()) {
			$html .= ach_render_tiebar($cat->getCurrentCult(),$cat->getCurrentCiv(),$cat);
		}


		$iter = $cat->getOpen();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			if(!$curr->isTiedAlign_open($cat->getCurrentCult(),$cat->getCurrentCiv())) {
				#continue;
			}

			$html .= ach_render_achievement_open($curr,$cat);
		}

		return $html;
	}

	function ach_render_achievement_open(&$ach,&$cat) {
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
									<td width="100%"><center><table><tr><td><span style="font-weight:bold;font-size:24px;color:#FFFFFF;"><a name="ach_'.$ach->getID().'">[ach:]</a>'.$ach->getName().'</span>';
									if($ach->getParentID() != null && $ach->getParentID() != "null") {
										#echo $ach->getParentID();
										$c = $ach->getParent();
										$p = $c->getChildDataByID($ach->getParentID());
										$html .= "<br><span style='font-size:11px;'>child of <b>".$p->getName()."</b></span>";
									}

									$html .= '</td>';
					
					$html .= "<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='?mode=ach&cat=".$_REQUEST['cat']."&act=dev&state=".$ach->getDev()."&id=".$ach->getPathID()."#ach_".$ach->getID()."'><img src='pic/";
					if($ach->inDev()) {
						$html .= "red";
					}
					else {
						$html .= "green";
					}
					$html .= ".gif' /></a>&nbsp;<a href='javascript:hs(\"edit_ach_".$ach->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					$html .= "&nbsp;<a href='javascript:hs(\"new_task_".$ach->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";

					$html .= "&nbsp;<a href='javascript:hs(\"opts_ach_".$ach->getID()."\",\"block\");'><img src='pic/b_tblops.png'></a>";
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=ach&cat=".$_REQUEST['cat']."&confirm=delete&id=".$ach->getPathID()."'><img src='pic/b_drop.png'></a></nobr></td>
									</td></tr></table>";

									$html .= '</center></td><td rowspan="2" valign="top" style="font-weight: bold; text-align: center; font-size: 30px;color:#FFFFFF;padding-right:10px;"><!-- 
										'.$ach->getValueOpen().'<br> --><img src="'.$_CONF['image_url'].'pic/yubo_pending.png">
									</td>
								</tr><tr><td align="center" valign="top">';

							$html .= "<div id='edit_ach_".$ach->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
								<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$ach->getID()."&act=ach_update#ach_".$ach->getID()."'>
									<fieldset>
										<legend>edit achievement</legend>
										<table>
											<tr>
												<td class='bw'>name:</td>
												<td><input type='text' name='aal_name' value='".htmlspecialchars($ach->getName(),ENT_QUOTES)."' /></td>
											</tr>
											<tr>
												<td class='bw'>naming template:</td>
												<td><input type='text' name='aal_template' value='".htmlspecialchars($ach->getTemplate(),ENT_QUOTES)."' /></td>
											</tr>
											<tr>
												<td class='bw'>parent achievement:</td>
												<td>
													<select name='aa_parent'>
														<option value='null' selected='selected'>-- none --</option>";
														$p = $ach->getParent();
														$iter = $p->getOpen();
														while($iter->hasNext()) {
															$item = $iter->getNext();
															if($item->getID() != $ach->getID()) {
																$html .= "<option value='".$item->getID()."'";
																if($item->getID() == $ach->getParentID()) {
																	$html .= " selected='selected'";
																}
																$html .= ">".$item->getName()."</option>";
															}
														}
														$html .= "</select>
												</td>
											</tr>
											<tr>
												<td class='bw'>image:</td>
												<td><input type='text' name='aa_image' value='".htmlspecialchars($ach->getImage())."' /></td>
											</tr>
											<tr>
												<td class='bw'>sticky:</td>
												<td><input type='hidden' value='0' name='aa_sticky' /><input type='checkbox' name='aa_sticky' value='1'";
													if($ach->isSticky()) {
														$html .= " checked='checked'";
													}
													$html .= "/></td>
											</tr>
											<tr>
												<td colspan='2'><input type='submit' value='save' /></td>
											</tr>
										</table>
									</fieldset>
								</form>
							</div>";

							$html .= "<div id='new_task_".$ach->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
								<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$ach->getID()."&act=task_insert#ach_".$ach->getID()."'>
									<fieldset>
										<legend>add new task</legend>
										<table>
											<tr>
												<td class='bw'>name:</td>
												<td><input type='text' name='atl_name' /></td>
											</tr>
											<tr>
												<td class='bw'>naming template:</td>
												<td><input type='text' name='atl_template' /></td>
											</tr>
											<tr>
												<td class='bw'>yubopoints:</td>
												<td><input type='text' name='at_value' /></td>
											</tr>
											<tr>
												<td class='bw'>parent:</td>
												<td>
													<select name='at_parent'>
														<option value='null'>[set as base task]</option>";
														$iter = $ach->getOpen();
														while($iter->hasNext()) {
															$curr = $iter->getNext();
															$html .= "<option value='".$curr->getID()."'";
															if(!$iter->hasNext()) {
																$html .= " selected='selected'";
															}
															$html .= ">".$curr->getDisplayName()."</option>";
														}

													$html .= "</select>
												</td>
											</tr>
											<tr>
												<td class='bw'>inherit objectives:</td>
												<td><input type='hidden' value='0' name='at_inherit' /><input type='checkbox' name='at_inherit' value='1' /></td>
											</tr>
											<tr>
												<td class='bw'>condition:</td>
												<td>
													<select name='at_condition'>
														<option value='all' selected='selected'>all</option>
														<option value='any'>any</option>
														<option value='value'>by value</option>
													</select>
												</td>
											</tr>
											<tr>
												<td class='bw'>condition value:</td>
												<td><input type='text' name='at_condition_value' /></td>
											</tr>
											<tr>
												<td class='bw'>allegiance:</td>
												<td>
													<select name='at_tie_allegiance[]' multiple='multiple' size='15'>
														<option value='c_neutral|c_neutral'>neutral / neutral</option>
														<option value='c_kami|c_neutral'>Kami / neutral</option>
														<option value='c_karavan|c_neutral'>Karavan / neutral</option>
														<option value='c_neutral|c_fyros'>neutral / Fyros</option>
														<option value='c_kami|c_fyros'>Kami / Fyros</option>
														<option value='c_karavan|c_fyros'>Karavan / Fyros</option>
														<option value='c_neutral|c_matis'>neutral / Matis</option>
														<option value='c_kami|c_matis'>Kami / Matis</option>
														<option value='c_karavan|c_matis'>Karavan / Matis</option>
														<option value='c_neutral|c_tryker'>neutral / Tryker</option>
														<option value='c_kami|c_tryker'>Kami / Tryker</option>
														<option value='c_karavan|c_tryker'>Karavan / Tryker</option>
														<option value='c_neutral|c_zorai'>neutral / Zorai</option>
														<option value='c_kami|c_zorai'>Kami / Zorai</option>
														<option value='c_karavan|c_zorai'>Karavan / Zorai</option>
													</select>
												</td>
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
												<td class='bw'>new category:</td>
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

							$html .= ach_render_task_open($ach,$cat);
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

	function ach_render_task_open(&$ach,&$cat) {
		global $metalist;



		$html = "";

		$task_list = $ach->getOpen();
		while($task_list->hasNext()) {

			$task = $task_list->getNext();

			if(!$task->isTiedAlign($cat->getCurrentCult(),$cat->getCurrentCiv())) {
				#continue;
			}


				$html .= "<table><tr><td><span style='color:#999999;font-weight:bold;display:block;'><a name='task_".$task->getID()."'>[task:]</a>".$task->getDisplayName()." (".$task->getValue().")</span></td>";

				$html .= "<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='?mode=ach&cat=".$_REQUEST['cat']."&act=dev&state=".$task->getDev()."&id=".$task->getPathID()."#task_".$task->getID()."'><img src='pic/";
					if($task->inDev()) {
						$html .= "red";
					}
					else {
						$html .= "green";
					}
					$html .= ".gif' /></a>&nbsp;<a href='javascript:hs(\"edit_task_".$task->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					$html .= "&nbsp;<a href='javascript:hs(\"new_obj_".$task->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=ach&cat=".$_REQUEST['cat']."&confirm=delete&id=".$task->getPathID()."'><img src='pic/b_drop.png'></a></nobr></td>
									</td></tr></table>";

					$html .= "<div id='edit_task_".$task->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
						<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$task->getPathID()."&act=task_update#task_".$task->getID()."'>
							<fieldset>
								<legend>edit task</legend>
								<table>
									<tr>
										<td class='bw'>name:</td>
										<td><input type='text' name='atl_name' value='".htmlspecialchars($task->getName(),ENT_QUOTES)."' /></td>
									</tr>
									<tr>
										<td class='bw'>naming template:</td>
										<td><input type='text' name='atl_template' value='".htmlspecialchars($task->getTemplate(),ENT_QUOTES)."' /></td>
									</tr>
									<tr>
										<td class='bw'>yubopoints:</td>
										<td><input type='text' name='at_value' value='".htmlspecialchars($task->getValue(),ENT_QUOTES)."' /></td>
									</tr>
									<tr>
										<td class='bw'>parent:</td>
										<td>
											<select name='at_parent'>
												<option value='null' selected='selected'>[set as base task]</option>";
												$par = $task->getParent();
												$iter = $par->getOpen();
												while($iter->hasNext()) {
													$curr = $iter->getNext();
													if($curr->getID() == $task->getID()) {
														continue;
													}
													
													$html .= "<option value='".$curr->getID()."'";
													if($curr->getID() == $task->getParentID()) {
														$html .= " selected='selected'";
													}
													$html .= ">".$curr->getDisplayName()."</option>";
												}

											$html .= "</select>
										</td>
									</tr>
									<tr>
										<td class='bw'>inherit objectives:</td>
										<td><input type='hidden' value='0' name='at_inherit' /><input type='checkbox' name='at_inherit' value='1'";
										if($task->getHeritage() == 1) {
											$html .= " checked='checked'";
										}
										$html .= "/></td>
									</tr>
									<tr>
										<td class='bw'>condition:</td>
										<td>
											<select name='at_condition'>
												<option value='all'"; if($task->getCondition() == "all") { $html .= " selected='selected'"; } $html .= ">all</option>
												<option value='any'"; if($task->getCondition() == "any") { $html .= " selected='selected'"; } $html .= ">any</option>
												<option value='value'"; if($task->getCondition() == "value") { $html .= " selected='selected'"; } $html .= ">by value</option>
											</select>
										</td>
									</tr>
									<tr>
										<td class='bw'>condition value:</td>
										<td><input type='text' name='at_condition_value' value='".htmlspecialchars($task->getConditionValue(),ENT_QUOTES)."' /></td>
									</tr>
									<tr>
										<td class='bw'>allegiance:</td>
										<td>
											<select name='at_tie_allegiance[]' multiple='multiple' size='15'>
												<option value='c_neutral|c_neutral'"; if($task->isTiedAlign('c_neutral','c_neutral')) { $html .= " selected='selected'"; } $html .= ">neutral / neutral</option>
												<option value='c_kami|c_neutral'"; if($task->isTiedAlign('c_kami','c_neutral')) { $html .= " selected='selected'"; } $html .= ">Kami / neutral</option>
												<option value='c_karavan|c_neutral'"; if($task->isTiedAlign('c_karavan','c_neutral')) { $html .= " selected='selected'"; } $html .= ">Karavan / neutral</option>
												<option value='c_neutral|c_fyros'"; if($task->isTiedAlign('c_neutral','c_fyros')) { $html .= " selected='selected'"; } $html .= ">neutral / Fyros</option>
												<option value='c_kami|c_fyros'"; if($task->isTiedAlign('c_kami','c_fyros')) { $html .= " selected='selected'"; } $html .= ">Kami / Fyros</option>
												<option value='c_karavan|c_fyros'"; if($task->isTiedAlign('c_karavan','c_fyros')) { $html .= " selected='selected'"; } $html .= ">Karavan / Fyros</option>
												<option value='c_neutral|c_matis'"; if($task->isTiedAlign('c_neutral','c_matis')) { $html .= " selected='selected'"; } $html .= ">neutral / Matis</option>
												<option value='c_kami|c_matis'"; if($task->isTiedAlign('c_kami','c_matis')) { $html .= " selected='selected'"; } $html .= ">Kami / Matis</option>
												<option value='c_karavan|c_matis'"; if($task->isTiedAlign('c_karavan','c_matis')) { $html .= " selected='selected'"; } $html .= ">Karavan / Matis</option>
												<option value='c_neutral|c_tryker'"; if($task->isTiedAlign('c_neutral','c_tryker')) { $html .= " selected='selected'"; } $html .= ">neutral / Tryker</option>
												<option value='c_kami|c_tryker'"; if($task->isTiedAlign('c_kami','c_tryker')) { $html .= " selected='selected'"; } $html .= ">Kami / Tryker</option>
												<option value='c_karavan|c_tryker'"; if($task->isTiedAlign('c_karavan','c_tryker')) { $html .= " selected='selected'"; } $html .= ">Karavan / Tryker</option>
												<option value='c_neutral|c_zorai'"; if($task->isTiedAlign('c_neutral','c_zorai')) { $html .= " selected='selected'"; } $html .= ">neutral / Zorai</option>
												<option value='c_kami|c_zorai'"; if($task->isTiedAlign('c_kami','c_zorai')) { $html .= " selected='selected'"; } $html .= ">Kami / Zorai</option>
												<option value='c_karavan|c_zorai'"; if($task->isTiedAlign('c_karavan','c_zorai')) { $html .= " selected='selected'"; } $html .= ">Karavan / Zorai</option>
											</select>
										</td>
									</tr>
									<tr>
										<td colspan='2'><input type='submit' value='save' /></td>
									</tr>
								</table>
							</fieldset>
						</form>
					</div>";

					$html .= "<div id='new_obj_".$task->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
						<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$task->getPathID()."&act=obj_insert#task_".$task->getID()."'>
							<fieldset>
								<legend>add new objective</legend>
								<table>
									<tr>
										<td class='bw'>name:</td>
										<td><input type='text' name='aol_name' /></td>
									</tr>
									<tr>
										<td class='bw'>type:</td>
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
										<td class='bw'>trigger condition:</td>
										<td>
											<select name='ao_condition'>
												<option value='all' selected='selected'>require all</option>
												<option value='any'>require any</option>
												<option value='value'>value / progressbar</option>
											</select>
										</td>
									</tr>
									<tr>
										<td class='bw'>trigger value:</td>
										<td><input type='text' name='ao_value' /></td>
									</tr>
									<tr>
										<td class='bw'>metalink:</td>
										<td>
											<select name='ao_metalink'>
												<option value=''> -- none --</option>";
												/*$m = $menu->getIterator();
												while($m->hasNext()) {
													$n = $m->getNext();
													$html .= "<option value='' disabled='disabled'>".$n->getName()."</option>";

													$m2 = $n->getIterator();
													while($m2->hasNext()) {
														$n2 = $m2->getNext();
														$html .= "<option value='' disabled='disabled'>&nbsp;&nbsp;&nbsp;".$n2->getName()."</option>";

														//db
														$res = $DBc->sqlQuery("SELECT aa_id,aal_name FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$n2->getID()."' ORDER by aa_sticky DESC, aal_name ASC");
														$sz = sizeof($res);
														for($i=0;$i<$sz;$i++) {
															$html .= "<option value='".$res[$i]['aa_id']."'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;".$res[$i]['aal_name']."</option>";
														}
													}

													//db
													$res = $DBc->sqlQuery("SELECT aa_id,aal_name FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$n->getID()."' ORDER by aa_sticky DESC, aal_name ASC");
													$sz = sizeof($res);
													for($i=0;$i<$sz;$i++) {
														$html .= "<option value='".$res[$i]['aa_id']."'>&nbsp;&nbsp;&nbsp;".$res[$i]['aal_name']."</option>";
													}
												}*/

											$html .= $metalist;





											$html .= "</select>
										</td>
									</tr>
									<tr>
										<td colspan='2'><input type='submit' value='add' /></td>
									</tr>
								</table>
							</fieldset>
						</form>
					</div>";

				$html .= ach_render_obj_list($task->getIterator(),$task);
		}

		return $html;
	}

	function ach_render_obj_list($obj,$task) {
		#return null;
		global $metalist;
		$html = "<center><table width='90%'>";

		#$i = 0;
		#$skip = false;
		
		while($obj->hasNext()) {
		#foreach($obj as $elem) {
			$inh = "";
			$elem = $obj->getNext();

			if($task->isInherited($elem->getID())) {
				$inh = "<i>inherited</i>:&nbsp;";
			}

			#if(($i%2) == 0) {
				$html .= "<tr><td><table><tr>";
			#}

			switch($elem->getDisplay()) {
				case "meta":
					$html .= "<td>".$inh.ach_render_obj_meta($elem)."<td>";
					break;
				case "value":
					#if(($i%2) == 1) {
					#	$html .= "</tr><tr>";
					#}
					$html .= "<td>".$inh.ach_render_obj_value($elem)."</td>";
					#$i++;
					break;
				case "simple":
					$html .= "<td>".$inh.ach_render_obj_simple($elem)."</td>";
					break;
				case "hidden":
				default:
					//do nothing
					#$skip = true;
					#if(($i%2) == 1) {
					#	$html .= "</tr><tr>";
					#}
					$html .= "<td>".$inh.ach_render_obj_hidden($elem)."</td>";
					#$i++;
					break;
			}

			$html .= "<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='javascript:hs(\"edit_obj_".$elem->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					#$html .= "&nbsp;<a href='javascript:hs(\"edit_obj_".$elem->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=ach&cat=".$_REQUEST['cat']."&confirm=delete&id=".$elem->getPathID()."'><img src='pic/b_drop.png'></a></nobr></td>
									</td></tr></table>";



			$html .= "<div id='edit_obj_".$elem->getID()."' style='margin-bottom:3px;margin-top:3px;display:none;color:#000000;background-color:#FFFFFF;'>
						<form method='post' action='?mode=ach&cat=".$_REQUEST['cat']."&id=".$elem->getPathID()."&act=obj_update#task_".$elem->getTask()."'>
							<fieldset>
								<legend>edit objective</legend>
								<table>
									<tr>
										<td class='bw'>name:</td>
										<td><input type='text' name='aol_name' value='".htmlspecialchars($elem->getName(),ENT_QUOTES)."' /></td>
									</tr>
									<tr>
										<td class='bw'>type:</td>
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
										<td class='bw'>trigger condition:</td>
										<td>
											<select name='ao_condition'>
												<option value='all'"; if($elem->getCondition() == "all") { $html .= " selected='selected'"; } $html .= ">require all</option>
												<option value='any'"; if($elem->getCondition() == "any") { $html .= " selected='selected'"; } $html .= ">require any</option>
												<option value='value'"; if($elem->getCondition() == "value") { $html .= " selected='selected'"; } $html .= ">value / progressbar</option>
											</select>
										</td>
									</tr>
									<tr>
										<td class='bw'>trigger value:</td>
										<td><input type='text' name='ao_value' value='".htmlspecialchars($elem->getValue(),ENT_QUOTES)."' /></td>
									</tr>
									<tr>
										<td class='bw'>metalink:</td>
										<td><select name='ao_metalink'>
												<option value=''> -- none --</option>";
												/*$m = $menu->getIterator();
												while($m->hasNext()) {
													$n = $m->getNext();
													$html .= "<option value='' disabled='disabled'>".$n->getName()."</option>";

													$m2 = $n->getIterator();
													while($m2->hasNext()) {
														$n2 = $m2->getNext();
														$html .= "<option value='' disabled='disabled'>&nbsp;&nbsp;&nbsp;".$n2->getName()."</option>";

														//db
														$res = $DBc->sqlQuery("SELECT aa_id,aal_name FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$n2->getID()."' ORDER by aa_sticky DESC, aal_name ASC");
														$sz = sizeof($res);
														for($i=0;$i<$sz;$i++) {
															$html .= "<option value='".$res[$i]['aa_id']."'";
															if($res[$i]['aa_id'] == $elem->getMetalink()) {
																$html .= " selected='selected'";
															}
															$html .= ">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;".$res[$i]['aal_name']."</option>";
														}
													}

													//db
													$res = $DBc->sqlQuery("SELECT aa_id,aal_name FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$n->getID()."' ORDER by aa_sticky DESC, aal_name ASC");
													$sz = sizeof($res);
													for($i=0;$i<$sz;$i++) {
														$html .= "<option value='".$res[$i]['aa_id']."'";
														if($res[$i]['aa_id'] == $elem->getMetalink()) {
															$html .= " selected='selected'";
														}
														$html .= ">&nbsp;&nbsp;&nbsp;".$res[$i]['aal_name']."</option>";
													}
												}*/

								$html .= str_replace("value='".$elem->getMetalink()."'","value='".$elem->getMetalink()."' selected='selected'",$metalist);



								$html .= "</select></td>
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
		
		$html .= "<a name='".$obj->getID()."'>[obj:]</a>".$obj->getDisplayName()."</span>";

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
					<td valign='middle'><span style='color:".$col.";'>&nbsp;<a name='".$obj->getID()."'>[obj:]</a>".$obj->getDisplayName()."</span></td>
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
			$html .= "<div style='color:".$col.";display:block;'><a name='".$obj->getID()."'>[obj: hidden]</a></div>";
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
		global $_USER;

		$html = "<style>
			.o {
				color:orange;
			}
		</style>

		<div style='display:block;text-align:center;'><form method='post' action='?mode=ach&cat=".$cat->getID()."' id='cc_form'>
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
					</td>
					<td><a href='?mode=ach&cat=".$cat->getID()."&cult=%&civ=%'>show all</a></td>";
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