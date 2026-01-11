{block name=content}
<tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="{$second_menu_bg_color}" valign="middle">
	  <td>
	  <table>
	    <tr>
	      <td>
		<table cellspacing="0" cellpadding="4">
		  <tr>		    
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_ticket&id={$ticket_id}">Show Ticket</a></td>
		  </tr>
		</table>
	      </td>
	    </tr>
	  </table>
	  </td>
	</tr>
	<tr>
		<td height="3" bgcolor="#000000"></td>
	</tr>
      </table>
  </td></tr>
<tr><td>
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Log of Ticket #{$ticket_id}</h1></td>
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
						<p><h3>Title: <a href="{$ingame_webpath}?page=show_ticket&id={$ticket_id}">{$ticket_title}</a></h3></p>
						<p>
						    <table cellspacing="5">
							<tr bgcolor="{$table_header_tr_color}">
								<td>ID</td>
								<td>Timestamp</td>
								<td>Query</td>
							</tr>
						
							{foreach from=$ticket_logs item=log}
							<tr>
							      <td>{$log.tLogId}</td>
							      <td><span title="{$log.timestamp_elapsed}" data-rel="tooltip"  data-placement="right">{$log.timestamp}</span></td>
							      <td>{$log.query}</td>
							</tr>
							{/foreach}
	  
						    </table>
						</p>
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
	