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
		    <td valign="middle" nowrap><a href="{$ingame_webpath}?page=settings&id={$target_id}"><h7>Edit User</h7></a></td>
		    <td valign="middle" nowrap><a href="{$ingame_webpath}?page=createticket&user_id={$target_id}"><h7>Send Ticket</h7></a></td>
		    {if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			{if $userPermission eq 1}
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=2"><h7>Make Moderator</h7></a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=3"><h7>Make Admin</h7></a></td>
			{else if $userPermission eq 2 }
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=1"><h7>Demote to User</h7></a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=3"><h7>Make Admin</h7></a></td>
			{else if $userPermission eq 3 }
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=1"><h7>Demote to User</h7></a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=change_permission&user_id={$target_id}&value=2"><h7>Demote to Moderator</h7></a></td>
			{/if}
		    {/if}
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
		<td width="100%" height="12" valign="middle"><h1>Profile of {$target_name}</h1></td>
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
						
					<p><h2>Info</h2></p>
					<table cellpadding="4">
					    
						    <tr>
							<td><strong>Email:</strong></td>
							<td>{$mail}</td>                           
						    </tr>
						    
						    <tr>
							<td><strong>Role:</strong></td>
							<td>
							{if $userPermission eq 1}<font color="{$user_color}">User</font>{/if}
							{if $userPermission eq 2}<font color="{$mod_color}">Moderator</font>{/if}
							{if $userPermission eq 3}<font color="{$admin_color}">Admin</font>{/if}
							</td>                           
						    </tr>
						    {if $firstName neq ""}
						    <tr>
							<td><strong>Firstname:</strong></td>
							<td>{$firstName}</td>                           
						    </tr>
						    {/if}
						    {if $lastName neq ""}
						    <tr>
							<td><strong>LastName:</strong></td>
							<td>{$lastName}</td>                           
						    </tr>
						    {/if}
						    {if $country neq ""}
						    <tr>
							<td><strong>Country:</strong></td>
							<td>{$country}</td>                           
						    </tr>
						    {/if}
						    {if $gender neq 0}
						    <tr>
							<td><strong>Gender:</strong></td>
							{if $gender eq 1}
							<td><strong>♂</strong></td>
							{else if $gender eq 2}
							<td><strong>♀</strong></td>
							{/if}
						    </tr>
						    {/if}
				    
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
						<p><h3>Tickets</h3></p>
						<table cellpadding="3">

							<tr bgcolor="{$table_header_tr_color}">
								<td>ID</td>
								<td>Title</td>
								<td>Timestamp</td>
								<td>Category</td>
								<td>Status</td>
							</tr>

						      {foreach from=$ticketlist item=ticket}
						      <tr>
							    <td>{$ticket.tId}</td>
							    <td><a href ="{$ingame_webpath}?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
							    <td class="center">{$ticket.timestamp}</td>
							    <td class="center">{$ticket.category}</td>
			    
							    <td class="center"><span class="label {if $ticket.status eq 0}label-success{else if $ticket.status eq 1}label-warning{else if $ticket.status eq 2}label-important{/if}">{if $ticket.status eq 0} <i class="icon-exclamation-sign icon-white"></i>{/if} {$ticket.statusText}</span></td>  
						      </tr>
						      {/foreach}

						</table>            
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
	
