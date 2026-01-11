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
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_queue&get=todo">Todo tickets</a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_queue&get=all">All tickets</a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_queue&get=all_open">All open tickets</a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_queue&get=archive">Ticket Archive</a></td>
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_queue&get=not_assigned">Not Assigned Tickets</a></td>
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
	      <td width="100%" height="12" valign="middle"><h1>Ticket Queue: {$queue_view}</h1></td>
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
			      <table width="100%" cellpadding="10">
				      <tr><td>
					      <p><h3>Tickets</h3></p>
					      <p>
						<form id="create_queue"  method="post" action="{$ingame_webpath}?page=show_queue&get=create">
						<table width="100%" bgcolor="#00000060" border="1">
						    <tr>
							<td width="5"></td>
							<td valign="middle"> Show </td>
							<td>
							    <select size ="100" name="what">
								<option value="all" {if $prev_created_what eq "all"}selected="selected"{/if}>all</option>
								<option value="waiting_for_support" {if $prev_created_what eq "waiting_for_support"}selected="selected"{/if}>waiting for support</option>
								<option value="waiting_for_users" {if $prev_created_what eq "waiting_for_users"}selected="selected"{/if}>waiting for user</option>
								<option value="closed" {if $prev_created_what eq "closed"}selected="selected"{/if}>closed</option>
							    </select>
							</td>
							<td valign="middle"> tickets </td>
							<td>
							    <select style="width: 110px;" name="how">
								<option value="assigned" {if $prev_created_how eq "assigned"}selected="selected"{/if}>assigned</option>
								<option value="not_assigned" {if $prev_created_how eq "not_assigned"}selected="selected"{/if}>not assigned</option>
							    </select>
							</td>
							<td valign="middle"> to </td>
							<td>
							    <select style="width: 140px;" name="who" onchange="aimedforwhochanged(this.value);">
								<option value="user" {if $prev_created_who eq "user"}selected="selected"{/if}>user</option>
								<option value="support_group" {if $prev_created_who eq "support_group"}selected="selected"{/if}>Group</option>
							    </select>
							</td>
							<td valign="middle">:</td>
							<td>
							    <select style="width: 140px;" name="userid">
								    <option>select user</option>
								{foreach from=$teamlist item=member}
								    <option value="{$member.tUserId}" {if $prev_created_userid eq $member.tUserId}selected="selected"{/if}>{$member.name}</option>
								{/foreach}
							    </select>
							</td>
							<td valign="middle"> or </td>
							<td>
							<select style="width: 140px;" name="groupid">
							    <option>select group</option>
							    {foreach from=$grouplist item=group}
								<option value="{$group.sGroupId}" {if $prev_created_groupid eq $group.sGroupId}selected="selected"{/if}>{$group.name}</option>
							    {/foreach}
							</select>
							</td>
							<td valign="middle">
							<input type="hidden" name="action" value="create_queue">
							<input type="submit" value="View"/>
    							</td>
							<td width="30%"></td>
						    </tr>
						</table>
						</form>
					      </p>
					      <p>
						<table width="100%" cellpadding="4" cellspacing="2">
							<tr bgcolor="{$table_header_tr_color}">								
							    <td>ID</td>
							    <td>Title</td>
							    <td>Assigned</td>
							    <td>Timestamp</td>
							    <td>Category</td>
							    <td>Status</td>
							    <td>SupportGroup</td>
							    <td>Actions</td>			  
							</tr>
							{foreach from=$tickets item=ticket}
							  <tr>
								<td>{$ticket.tId}</td>
								<td><a href ="{$ingame_webpath}?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
								<td>{if $ticket.assignedText neq ""} <a href="{$ingame_webpath}?page=show_user&id={$ticket.assigned}">{$ticket.assignedText}</a>{else} {$not_assigned} {/if}</td>
								<td class="center"><span title="{$ticket.timestamp_elapsed}" data-rel="tooltip"  data-placement="right">{$ticket.timestamp}</span></td>
								<td class="center">{$ticket.category}</td>
								<td class="center">{if $ticket.status eq 0}<font color="green">{else if $ticket.status eq 1}<font color="orange">{else if $ticket.status eq 2}<font color="red">{/if}{$ticket.statusText}</font></td>  
								<td class="center">
								   
								    {if $ticket.forwardedGroupName eq "0"}
									{$public_sgroup}
								    {else}
									<a href="{$ingame_webpath}?page=show_sgroup&id={$ticket.forwardedGroupId}">{$ticket.forwardedGroupName}</a>
								    {/if}
								      
								</td>  
								<td>
								    {if $ticket.assigned eq 0}
									<form id="assign_ticket" class="form-vertical" method="post" action="{$ingame_webpath}?page=show_queue&get=todo" style="margin:0px 0px 0px;">
									    <input type="hidden" name="ticket_id" value="{$ticket.tId}">
									    <input type="hidden" name="action" value="assignTicket">
									    <input type="submit" value="Assign Ticket"/>
									</form>
								    {else if $ticket.assigned eq $user_id}
									<form id="assign_ticket" class="form-vertical" method="post" action="{$ingame_webpath}?page=show_queue&get=todo" style="margin:0px 0px 0px;">
									    <input type="hidden" name="ticket_id" value="{$ticket.tId}">
									    <input type="hidden" name="action" value="unAssignTicket">
									    <input type="submit" value="Remove Assign"/>
									</form>
								    {/if}
								</td>
							  </tr>
							{/foreach}         
					      </table>
					      </p>
				      </td></tr>
				      <tr><td align = "center">
					 <table>
				           <tr>
				              <td><a href="{$pagination_base_link}&pagenum=1">&laquo;</a></td>
					      {foreach from=$links item=link}
				                 <td {if $link == $currentPage}bgcolor="{$pagination_current_page_bg}"{/if}><a href="{$pagination_base_link}&pagenum={$link}">{$link}</a></td>
				              {/foreach}
					      <td><a href="{$pagination_base_link}&pagenum={$lastPage}">&raquo;</a></td>
					   </tr>
					 </table>
				      </td></tr>
			      </table>
		      </td></tr>
		      {if isset($ACTION_RESULT)}
		      <tr><td>
			<table width="100%" bgcolor="{$main_tbl_color}" border="1">
			    <tr><td height="4"></td></tr>
			    <tr>
				<td valign="middle">
				    {if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_ASSIGNED"}
				    <p>
					    <font color="green">{$success_assigned}</font>
				    </p>
				    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_UNASSIGNED"}
				    <p>
					    <font color="green">{$success_unassigned}</font>
				    </p>
				    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
				    <p>
					    <font color="red">{$ticket_not_existing}</font>
				    </p>
				    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "ALREADY_ASSIGNED"}
				    <p>
					    <font color="red">{$ticket_already_assigned}</font>
				    </p>
				    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "NOT_ASSIGNED"}
				    <p>
					    <font color="red">{$ticket_not_assigned}</font>
				    </p>
				    {/if}
				</td>
			    </tr>
			    <tr><td height="4"></td></tr>
			<table>
		      </td></tr>
		      {/if}
	      </table>
	    </td></tr>
	  </table>
	</td>
      </tr>
    </table>
    
</td></tr>

{/block}
	
