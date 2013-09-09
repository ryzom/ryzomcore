{block name=content}

  <tr><td>
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>{$home_title}</h1></td>
	</tr>
	<tr>
	  <td height="5"></td><td></td>
	</tr>
	</table>
  </td></tr>
  <tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="#000000" valign="middle">
	  <td>
	    <table>
	      <tr><td height="8"></td></tr>
	    </table>
	  </td>
	</tr>
	<tr><td height="2"></td></tr>
	<tr><td height="1" bgcolor="#000000"></td></tr>
	<tr><td height="10"></td></tr>
	<tr valign="middle">
	  <td>
	    <table width="100%" height="100%" cellpadding="10">
	     <tr><td>
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						<table width="100%" cellpadding="3" border="2">
							<tr>
								<td width="25%" align = "center" ><font color="{$info_color}">Tickets Waiting for Direct Action</font></td>
								<td width="25%" align = "center" ><font color="{$info_color}">Tickets Todo</font></td>
								<td width="25%" align = "center" ><font color="{$info_color}">Newest Ticket</font></td>
								<td width="25%" align = "center" ><font color="{$info_color}">Total amount of Tickets</font></td>
							</tr>
							<tr>
								<td width="25%" align = "center" ><a href="{$ingame_webpath}?page=show_queue&get=create&userid={$user_id}&groupid=1&what=waiting_for_support&how=assigned&who=user"><font color="{$notification_color}">{$nrAssignedWaiting}</font></a></td>
								<td width="25%" align = "center" ><a href="{$ingame_webpath}?page=show_queue&get=todo"><font color="{$notification_color}">{$nrToDo}</font></a></td>
								<td width="25%" align = "center" ><a href="{$ingame_webpath}?page=show_ticket&id={$newestTicketId}"><font color="{$notification_color}">{$newestTicketTitle}</font></a></td>
								<td width="25%" align = "center" ><a href="{$ingame_webpath}?page=show_queue&get=all"><font color="{$notification_color}">{$nrTotalTickets}</font></a></td>
							</tr>
						</table>
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
		
	      <tr><td>
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						<p><h3>{$home_info}</h3></p>
						<p> This is the GSOC project of Daan Janssens mentored by Matthew Lagoe.</p>					
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
	    </table>
	  </td>
	</tr>
      </table>
      
  </td></tr>

	

				
			
				
	
{/block}

