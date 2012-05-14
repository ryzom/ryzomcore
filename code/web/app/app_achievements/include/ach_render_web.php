<?php
	function ach_render_box_done($content) {




		return '<div style="display: block; margin-bottom: 5px;"><table cellpadding="0" cellspacing="0" width="100%">
					<tbody><tr>
						<td width="3px"><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_ul.png"></td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_u.png);"></td>
						<td width="3px"><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_ur.png"></td>
					</tr>
					<tr>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_l.png);"></td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_bg.png);">
							<center><table>
								<tbody><tr>
									<td rowspan="2"><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/icon/13.png"></td>
									<td height="35px" width="430px"><center><h2 style="margin: 0px;color:#000000;">'.$content.'</h2></center></td>
									<td rowspan="2" style="font-weight: bold; text-align: center; font-size: 14px;color:#000000;">
										50<br><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png">
									</td>
								</tr>
								<tr id="ach_done_13">
									<td><center><table width="350px"><tbody><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 50 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 100 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 150 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 200 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 250 jewels</center></td><td><b>03.01.2012</b></td></tr></tbody></table></center></td>
								</tr>
							</tbody></table></center>
						</td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_r.png);"></td>
					</tr>
					<tr>
						<td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_bl.png"></td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_b.png);"></td>
						<td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_done_br.png"></td>
					</tr>
				</tbody></table></div>';
	}

	function ach_render_box_pending($name,$content) {
		return '<div style="display: block; margin-bottom: 5px;"><table cellpadding="0" cellspacing="0" width="100%">
					<tbody><tr>
						<td width="3px"><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_ul.png"></td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_u.png);"></td>
						<td width="3px"><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_ur.png"></td>
					</tr>
					<tr>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_l.png);"></td>
						<td>
							<center><table>
								<tbody><tr>
									<td rowspan="2"><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/icon/13.png"></td>
									<td height="35px" width="430px"><center><h2 style="margin: 0px;color:#FFFFFF;">'.$name.'</h2></center></td>
									<td rowspan="2" style="font-weight: bold; text-align: center; font-size: 14px;color:#FFFFFF;">
										50<br><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png">
									</td>
								</tr>
								<tr id="ach_done_13">
									<td><center><table width="350px"><tbody><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 50 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 100 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 150 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 200 jewels</center></td><td><b>03.01.2012</b></td></tr><tr><td><b>10</b></td><td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/yubo_done.png"></td><td width="100%"><center>Equip a full set of at least quality 250 jewels</center></td><td><b>03.01.2012</b></td></tr></tbody></table></center></td>
								</tr>
							</tbody></table></center>
						</td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_r.png);"></td>
					</tr>
					<tr>
						<td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_bl.png"></td>
						<td style="background-image: url(http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_b.png);"></td>
						<td><img src="http://www.3025-game.de/portal/ryzom/resource/pic/achievements/bar_pending_br.png"></td>
					</tr>
				</tbody></table></div>';
	}

	function ach_render_obj_list($list) {
		//group
		//columnize
		//draw
	}

	function ach_render_obj_meta($obj) {

	}

	function ach_render_obj_simple($obj) {
		return $obj->getName();
	}

	function ach_render_obj_value($obj) {

	}

	function ach_render_tiebar($cult = "neutral", $civ = "neutral") {

	}
?>