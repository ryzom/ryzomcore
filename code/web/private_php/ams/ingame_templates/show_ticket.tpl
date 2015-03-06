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
			{if isset($isMod) and $isMod eq "TRUE"}<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_ticket_log&id={$ticket_tId}">Show Ticket Log</a></td>{/if}
			<td valign="middle" nowrap><a href="{$ingame_webpath}?page=createticket&user_id={$ticket_author}">Send Other Ticket</a></td>
			{if $hasInfo}<td valign="middle" nowrap><a href="{$ingame_webpath}?page=show_ticket_info&id={$ticket_tId}">Show Additional Info</a></td>{/if}
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
		<td width="100%" height="12" valign="middle"><h1>[{$t_title}-#{$ticket_tId}] {$ticket_title}</h1></td>
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
				<table cellpadding="10" width="100%">
					<tr><td>
					    <table cellpadding="5" width="100%">
						    <tr><td>
							<table cellpadding="1" bgcolor="{$normal_tbl_color}" border="2" width="100%">
							    <tr><td>
							    <table cellpadding="3" width="100%">
								<tr>
								    <td><font color="{$info_color}">Submitted: </font>{$ticket_timestamp}</td>
								    <td><font color="{$info_color}">Last Updated: </font>{$ticket_lastupdate}</td>
								    <td><font color="{$info_color}">Status: </font>{if $ticket_status neq 3}<font color="green">Open</font>{/if} {if $ticket_status eq 3} <font color="red">{$ticket_statustext}</font>{else}{$ticket_statustext} {/if}</td> 
							      </tr>
								<tr>
								    <td><font color="{$info_color}">Category: </font>{$ticket_category}</td>
								    <td><font color="{$info_color}">Priority </font>{$ticket_prioritytext}</td>
								    <td><font color="{$info_color}">Support Group: </font>
									<span class="label label-info">
									    {if $ticket_forwardedGroupName eq "0"}
										{$public_sgroup}
									    {else}
										<a href="{$ingame_webpath}?page=show_sgroup&id={$ticket_forwardedGroupId}"><font color="white">{$ticket_forwardedGroupName}</font></a>
									    {/if}
									</span>
								    </td>                  
								</tr>
								<tr>
								    <td><font color="{$info_color}">Assigned To:  </font>{if $ticket_assignedTo neq ""} <a href="{$ingame_webpath}?page=show_user&id={$ticket_assignedTo}">{$ticket_assignedToText}</a>{else} {$not_assigned} {/if}</td>
								    <td></td>
								    <td></td> 
							      </tr>
							    </table>
							</td></tr>
							</table>
						    </td></tr>
						    <tr><td>
							<table cellpadding="1" bgcolor="{$normal_tbl_color}" border="2" width="100%">    
							    {foreach from=$ticket_replies item=reply}
							    <tr>
								<td {if $reply.permission gt '1'} {if $reply.hidden eq 0} bgcolor="{$team_reply_tbl_color}"{else if $reply.hidden eq 1} bgcolor="{$hidden_reply_tbl_color}"{/if}{/if}>
								   <table cellpadding="3">
									<tr><td>
									    <p>
										<font color="{$info_color}"> {$reply.timestamp}</font>
										{if $reply.permission eq '1'}
										<span class="label label-success"><strong><i class="icon-user icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="white"> {$reply.author}</font>{else} {$reply.author} {/if}</a></strong></span>
										{else if $reply.permission gt '1'}
										<span class="label label-warning"><strong><i class="icon-star icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="{$team_color}"> {$reply.author}</font>{else} {$reply.author} {/if}</a></strong></span>
										{/if}
									    </p>
									    <p><pre>{$reply.replyContent}</pre></p>
									</td></tr>
								   </table>
								</td>
							    </tr>
							    {/foreach}
							    
							    {if $ticket_status eq 3}
							    <tr>
								<td bgcolor="{$closed_tbl_color}">
								     <table cellpadding="4">
									<tr><td>
									    <p><pre><h4>[Ticket is closed]</h4></pre></p>
									</td></tr>
								     </table>
								</td>
							    </tr>
							    {/if}
							    
							    <tr>
								<td>
								    
								    <form id="reply" class="form-vertical" method="post" action="{$ingame_webpath}"> 
								    <table cellpadding="4">
								    <tr><td height="5"></td></tr>
								    <tr>
									<td>
									    <p><h4>{$t_reply}:</h4></p>
									</td>
								    </tr>
								    {if $ticket_status neq 3}
								    <tr>
									<td><textarea cols="50" id="Content" name="Content"><br><br><br></textarea></td>
								    </tr>
									{if isset($isMod) and $isMod eq "TRUE"}
									<tr>
									    <td><input type="checkbox" name="hidden">Hide reply for user.</td>
									</tr>
									{/if}
								    {/if}
								    <tr>
									<td>
									{if isset($isMod) and $isMod eq "TRUE"}
									    <table>
										<tr>
										    <td>
											Change status to
											<select name="ChangeStatus">
											    {foreach from=$statusList key=k item=v}
												    <option value="{$k}">{$v}</option>
											    {/foreach}
											</select>	
										    </td>
										    <td>
											Change priority to
											<select name="ChangePriority">
											    {foreach from=$ticket_priorities key=k item=v}
												    <option value="{$k}" {if $k eq $ticket_priority}selected="selected"{/if}>{$v}</option>
											    {/foreach}
											</select>
										    </td>
										</tr>
									    </table>
									{/if}
									</td>
								    </tr>
								    <tr>
									<td>
									    <input type="hidden" name="function" value="reply_on_ticket">
									    <input type="hidden" name="ticket_id" value="{$ticket_id}">
									    <input type="submit" value="{$t_send}"/>
									</td>
								    </tr>
								    </table>
								    </form>
								</td>
							    </tr>
							</table>
						    </td></tr>
						    
							{if isset($isMod) and $isMod eq "TRUE"}
						        <tr><td>
							<table cellpadding="1" bgcolor="{$normal_tbl_color}" border="2" width="100%">
							    <tr><td>
							    <table cellpadding="3" width="100%">
								<tr>
								    <td>
									 <p>
									    Ticket Assigning: 
									    {if $ticket_assignedTo eq 0}
									       <form id="assign_ticket" method="post" action="{$ingame_webpath}?page=show_ticket&id={$ticket_tId}">
										   <input type="hidden" name="ticket_id" value="{$ticket_tId}">
										   <input type="hidden" name="action" value="assignTicket">
										   <input type="submit" value="Assign Ticket"/>
									       </form>
									   {else if $ticket_assignedTo eq $user_id}
									       <form id="assign_ticket" method="post" action="{$ingame_webpath}?page=show_ticket&id={$ticket_tId}">
										   <input type="hidden" name="ticket_id" value="{$ticket_tId}">
										   <input type="hidden" name="action" value="unAssignTicket">
										   <input type="submit"value="Remove Assign"/>
									       </form>
									   {/if}
									 </p>		    
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
								    <td>
									<p>
									    Forward to Group:		    
									    <form id="forward" method="post" action="{$ingame_webpath}?page=show_ticket&id={$ticket_tId}" >
										<select name="group">
											    <option></option>
										    {foreach from=$sGroups key=k item=v}
											    <option value="{$k}">{$v}</option>
										    {/foreach}
										</select>	
										
										<input type="hidden" name="ticket_id" value="{$ticket_tId}">
										<input type="hidden" name="action" value="forward">										
										<input type="submit"value="Forward"/>	 
									    </form>
									</p>
									{if isset($ACTION_RESULT) and $ACTION_RESULT eq "INVALID_SGROUP"}
									<p>
										<font color="red">{$invalid_sgroup}</font>
									</p>
									{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
									<p>
										<font color="red">{$ticket_not_existing}</font>
									</p>
									{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_FORWARDED"}
									<p>
										<font color="green">{$success_forwarded}</font>
									</p>
									{/if}
								    </td>
							      </tr>
							    </table>
							</td></tr>
							</table>
						    </td></tr>
						    <tr><td>
						    {/if}
						    
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
	
