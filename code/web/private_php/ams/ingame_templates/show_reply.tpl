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
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_ticket_log&id={$ticket_id}">Show Ticket Log</a></td>
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
		<td width="100%" height="12" valign="middle"><h1>Reply ID#{$reply_id} of Ticket <a href="{$ingame_webpath}?page=show_ticket&id={$ticket_id}">#{$ticket_id}</a></h1></td>
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
						<p><h3>Reply:</h3></p>
						<p>
						    <table cellspacing="5">
							<tr>
							    <td>
								<p>
								<font color="{$info_color}"> {$reply_timestamp}</font>
								{if $author_permission eq '1'}
								<span class="label label-success"><strong><i class="icon-user icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="white"> {$authorName}</font>{else} {$authorName} {/if}</a></strong></span>
								{else if $reply.permission gt '1'}
								<span class="label label-warning"><strong><i class="icon-star icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="{$team_color}"> {$authorName}</font>{else} {$authorName} {/if}</a></strong></span>
								{/if}</p>
								<p><pre>{$reply_content}</pre></p>
							    </td>
							</tr>
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
	

