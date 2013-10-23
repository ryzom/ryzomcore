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
		</style>
		
		<script type='text/javascript'>
function setSelectionRange(input, selectionStart, selectionEnd) {
  if (input.setSelectionRange) {
    input.focus();
    input.setSelectionRange(selectionStart, selectionEnd);
  }
  else if (input.createTextRange) {
    var range = input.createTextRange();
    range.collapse(true);
    range.moveEnd('character', selectionEnd);
    range.moveStart('character', selectionStart);
    range.select();
  }
}

function replaceSelection (input, replaceString) {
	if (input.setSelectionRange) {
		var selectionStart = input.selectionStart;
		var selectionEnd = input.selectionEnd;
		input.value = input.value.substring(0, selectionStart)+ replaceString + input.value.substring(selectionEnd);
    
		if (selectionStart != selectionEnd){ 
			setSelectionRange(input, selectionStart, selectionStart + 	replaceString.length);
		}else{
			setSelectionRange(input, selectionStart + replaceString.length, selectionStart + replaceString.length);
		}

	}else if (document.selection) {
		var range = document.selection.createRange();

		if (range.parentElement() == input) {
			var isCollapsed = range.text == '';
			range.text = replaceString;

			 if (!isCollapsed)  {
				range.moveStart('character', -replaceString.length);
				range.select();
			}
		}
	}
}


// We are going to catch the TAB key so that we can use it, Hooray!
function catchTab(item,e){
	if(navigator.userAgent.match('Gecko')){
		c=e.which;
	}else{
		c=e.keyCode;
	}
	if(c==9){
		replaceSelection(item,String.fromCharCode(9));
		setTimeout('document.getElementById(\"'+item.id+'\").focus();',0);	
		return false;
	}
		    
}
</script>
		
		";
		
		$iter = $cat->getOpen();
		while($iter->hasNext()) {
			$curr = $iter->getNext();

			$html .= ach_render_achievement_open($curr);
		}

		return $html;
	}

	function ach_render_achievement_open(&$ach) {

		$open = explode(";",$_REQUEST['id']);

		$o = "none";
		if($open[1] == $ach->getID()) {
			$o = "block";
		}

		$html = "<div style='display: block; margin-bottom: 5px;'>
			<div style='display:block;font-size:22px;' class='bar'><a href='javascript:hs(\"ach_".$ach->getID()."\",\"block\");'>[+]</a> ".$ach->getName()."</div>
			<div style='margin-left:25px;display:".$o.";' id='ach_".$ach->getID()."'>".ach_render_task_open($ach)."</div>
		</div>";

		return $html;
	}

	function ach_render_task_open(&$ach) {
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
				<div style='display:block;font-size:16px;' class='bar'><a href='javascript:hs(\"task_".$task->getID()."\",\"block\");'>[+]</a> ".$task->getDisplayName()." <span style='font-size:12px;'>(condition= ".$task->getCondition().": ".$task->getConditionValue().")</span></div>
				<div style='margin-left:25px;display:".$o.";' id='task_".$task->getID()."'>".ach_render_obj_list($task->getIterator(),$task)."</div>
			</div>";
		}

		return $html;
	}

	function ach_render_obj_list($obj,$task) {
		$html = "";


		while($obj->hasNext()) {
			$elem = $obj->getNext();

			if($task->isInherited($elem->getID()) || $elem->getDisplay() == "meta") {
				continue;
			}

				$o = "block";

			
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