<?php
	function ach_render_box_done($content) {
		return "<table bgcolor='#FFFFFF' cellspacing='1' cellpadding='0'>
			<tr>
				<td><table cellspacing='2' cellpadding='0' bgcolor='#B4B4B4'>
					<tr>
						<td><table cellspacing='1' cellpadding='0' bgcolor='#FFFFFF'>
							<tr>
								<td><table cellspacing='1' cellpadding='0' bgcolor='#B4B4B4'>
									<tr>
										<td width='450px' height='50px' align='center' valign='middle'>".$content."</td>
									</tr>
								</table></td>
							</tr>
						</table></td>
					</tr>
				</table></td>
			</tr>
		</table>";
	}
?>