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
		
		$html .= "<div style='display:block;background-color:#FFFFFF;padding:3px;margin-bottom:5px;color:#000000;'>
		<div style='display:block;text-align:right;'><a href='javascript:hs(\"new_main\",\"block\");'><img src='pic/b_insrow.png'></a></div>

		<div style='display:none;' id='new_main'>
			<form method='post' action='?mode=menu&act=insert'>
				<fieldset>
				<legend>create new category</legend>
				<input type='hidden' name='ac_parent' value='NULL' />
				<table>
					<tr>
						<td>name</td>
						<td><input type='text' name='acl_name' /></td>
					</tr>
					<tr>
						<td>image</td>
						<td><input type='text' name='ac_image' /></td>
					</tr>
					<tr>
						<td>&nbsp;</td>
						<td><input type='submit' value='create' /></td>
					</tr>
				</table>
			</fieldset>
			</form>
		</div>";

		if($_REQUEST['ac_id'] > 0 && $_REQUEST['confirm'] == "delete") {
			$curr = $menu->getNode($_REQUEST['ac_id']);
			$html .= "<div style='display:block;'>
			<fieldset>
				<legend>Are you sure you want to delete this category?</legend>";
				if($curr->hasAchievements()) {
					$html .= "<b>You may NOT DELETE this category since there are still achievements tied to it or one of its sub-categories!</b>";
				}
				else {
					$html .= "<b style='font-size:16px;'>".$curr->getName()."</b><p>";
				
					if($curr->getParentID() == null) {
						$html .= "<b>WARNING:</b> Deleting this category will also delete ALL sub-categories!<br>";
					}
					$html .= "<a href='?mode=menu&act=delete&ac_id=".$_REQUEST['ac_id']."'><b>delete</b></a>";
				}
			$html .= "</fieldset>
			</div>";
		}
		
		$html .= "</div>";

		return $html.ach_render_mnode($menu,$sub);
	}

	function ach_render_mnode(&$menu,$sub) {
		global $_CONF;

	#	echo "1";

		$iter = $menu->getIterator();
		while($iter->hasNext()) {
			$curr = $iter->getNext();
		#$sz = $menu->getSize();
		#for($i=0;$i<$sz;$i++) {
		#	$curr = $menu->getChild($i);

			$html .= "<span class='ach_mspan'><table class='ach_menu'>
				<tr>";
					if($sub == 0) {
						$html .= "<td><img src='".$_CONF['image_url']."pic/menu/".$curr->getImage()."' /></td>";
					}
					$html .= "<td style='font-size:".(20-$sub)."px;font-weight:bold;' width='100%'>";
					if($curr->inDev()) {
						$html .= "<s>";
					}
					
					$html .= $curr->getName();
					
					if($curr->inDev()) {
						$html .= "</s>";
					}
					$html .= "<a name='cat_".$curr->getID()."'></td>
					<td style='background-color:#FFFFFF;padding:3px;'><nobr><a href='?mode=menu&act=dev&state=".$curr->getDev()."&ac_id=".$curr->getID()."#cat_".$curr->getID()."'><img src='pic/";
					if($curr->inDev()) {
						$html .= "red";
					}
					else {
						$html .= "green";
					}
					$html .= ".gif' /></a>&nbsp;<a href='javascript:hs(\"edit_m".$curr->getID()."\",\"block\");'><img src='pic/icon_edit.gif'></a>";

					if($sub == 0) {
						$html .= "&nbsp;<a href='javascript:hs(\"ins_m".$curr->getID()."\",\"block\");'><img src='pic/b_insrow.png'></a>";
					}
						
					$html .= "&nbsp;&nbsp;&nbsp;<a href='?mode=menu&confirm=delete&ac_id=".$curr->getID()."'><img src='pic/b_drop.png'></a></nobr></td>
				</tr>
			</table></span>";
			if($sub == 0) {
				$html .= "<div style='display:none;color:#000000;background-color:#FFFFFF;' id='ins_m".$curr->getID()."'>
					<form method='post' action='?mode=menu&act=insert#cat_".$curr->getID()."'>
						<fieldset>
						<legend>create new sub-category</legend>
						<input type='hidden' name='ac_parent' value='".$curr->getID()."' />
						<input type='hidden' name='ac_image' value='NULL' />
						<table>
							<tr>
								<td>name</td>
								<td><input type='text' name='acl_name' /></td>
							</tr>
							<tr>
								<td>&nbsp;</td>
								<td><input type='submit' value='create' /></td>
							</tr>
						</table>
					</fieldset>
					</form>
				</div>";
			}

			$html .= "<div style='display:none;color:#000000;background-color:#FFFFFF;' id='edit_m".$curr->getID()."'>
					<form method='post' action='?mode=menu&act=update&ac_id=".$curr->getID()."#cat_".$curr->getID()."'>
						<fieldset>
						<legend>edit category</legend>";
						if($sub != 0) {
							$html .= "<input type='hidden' name='ac_image' value='NULL' />";
						}
						$html .= "<table>
							<tr>
								<td>name</td>
								<td><input type='text' name='acl_name' value='".htmlspecialchars($curr->getName(),ENT_QUOTES)."' /></td>
							</tr>";
							if($sub == 0) {
								$html .= "<tr>
									<td>image</td>
									<td><input type='text' name='ac_image' value='".htmlspecialchars($curr->getImage(),ENT_QUOTES)."' /></td>
								</tr>";
							}
							$html .= "<tr>
								<td>&nbsp;</td>
								<td><input type='submit' value='save' /></td>
							</tr>
						</table>
					</fieldset>
					</form>
				</div>";

			if(!$curr->isEmpty()) {
				$html .= "<div style='display:block;margin-left:25px;'>".ach_render_mnode($curr,($sub+4))."</div>";
			}
		}

		return $html;
	}
?>