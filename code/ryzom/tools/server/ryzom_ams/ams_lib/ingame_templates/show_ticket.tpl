{block name=content}
<tr><td>
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr bgcolor="#00000040" valign="middle">
	  <td>
	  <table>
	    <tr>
	      <td>
		<table cellspacing="0" cellpadding="4">
		  <tr>		    
			{if isset($isMod) and $isMod eq "TRUE"}<td valign="middle" nowrap><a href="index.php?page=show_ticket_log&id={$ticket_tId}">Show Ticket Log</a></td>{/if}
			<td valign="middle" nowrap><a href="index.php?page=createticket&user_id={$target_id}">Send Other Ticket</a></td>
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
	<table width="100%" bgcolor="#303030" cellspacing="2">
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
		<table width="100%" bgcolor="#00000030" border="2">
			<tr><td>
				<table cellpadding="10" width="100%">
					<tr><td>
					    <table cellpadding="5" width="100%">
						    <tr><td>
							<table cellpadding="1" bgcolor="#00000060" border="2" width="100%">
							    <tr><td>
							    <table cellpadding="3" width="100%">
								<tr>
								    <td><font color="#00CED1">Submitted: </font>{$ticket_timestamp}</td>
								    <td><font color="#00CED1">Last Updated: </font>{$ticket_lastupdate}</td>
								    <td><font color="#00CED1">Status: </font>{if $ticket_status neq 3}<span class="label label-success">Open</span>{/if} <span class="label {if $ticket_status eq 0}label-success{else if $ticket_status eq 1}label-warning{else if $ticket_status eq 2}label-important{/if}"><strong>{$ticket_statustext}</strong></span></td> 
							      </tr>
								<tr>
								    <td><font color="#00CED1">Category: </font>{$ticket_category}</td>
								    <td><font color="#00CED1">Priority </font>{$ticket_prioritytext}</td>
								    <td><font color="#00CED1">Support Group: </font>
									<span class="label label-info">
									    {if $ticket_forwardedGroupName eq "0"}
										{$public_sgroup}
									    {else}
										<a href="index.php?page=show_sgroup&id={$ticket_forwardedGroupId}"><font color="white">{$ticket_forwardedGroupName}</font></a>
									    {/if}
									</span>
								    </td>                  
								</tr>
								<tr>
								    <td><font color="#00CED1">Assigned To:  </font>{if $ticket_assignedTo neq ""} <a href="index.php?page=show_user&id={$ticket_assignedTo}">{$ticket_assignedToText} {else} {$not_assigned} {/if}</td>
								    <td></td>
								    <td></td> 
							      </tr>
							    </table>
							</td></tr>
							</table>
						    </td></tr>
						    <tr><td>
							<table cellpadding="1" bgcolor="#00000060" border="2" width="100%">    
							    {foreach from=$ticket_replies item=reply}
							    <tr>
								<td {if $reply.permission gt '1'} {if $reply.hidden eq 0} bgcolor="#F8C8C140"{else if $reply.hidden eq 1} bgcolor="#CFFEFF40"{/if}{/if}>
								    <p>
									<font color="#00CED1"> {$reply.timestamp}</font>
									{if $reply.permission eq '1'}
									<span class="label label-success"><strong><i class="icon-user icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="white"> {$reply.author}</font>{else} {$reply.author} {/if}</a></strong></span>
									{else if $reply.permission gt '1'}
									<span class="label label-warning"><strong><i class="icon-star icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="red"> {$reply.author}</font>{else} {$reply.author} {/if}</a></strong></span>
									{/if}
								    </p>
								    <p><pre> {if $reply.hidden eq 1}{/if}{$reply.replyContent}{if $reply.hidden eq 1}{/if}</pre></p>
								</td>
							    </tr>
							    {/foreach}
							    
							    {if $ticket_status eq 3}
							    <tr>
								<td bgcolor="#FFE69960">
								    <p><pre>Ticket is closed.</pre></p>
								</td>
							    </tr>
							    {/if}
							    
							    <tr>
								<td>
								    <form id="reply" class="form-vertical" method="post" action="index.php">
								    <p><h4>{$t_reply}:</h4></p>
								    <table>
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
							<table cellpadding="1" bgcolor="#00000060" border="2" width="100%">
							    <tr><td>
							    <table cellpadding="3" width="100%">
								<tr>
								    <td>
									 <p>
									    Ticket Assigning: 
									    {if $ticket_assignedTo eq 0}
									       <form id="assign_ticket" method="post" action="">
										   <input type="hidden" name="ticket_id" value="{$ticket_tId}">
										   <input type="hidden" name="action" value="assignTicket">
										   <input type="submit" value="Assign Ticket"/>
									       </form>
									   {else if $ticket_assignedTo eq $user_id}
									       <form id="assign_ticket" method="post" action="">
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
									    <form id="forward" method="post" action="" >
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
	
