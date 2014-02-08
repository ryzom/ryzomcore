{block name=content}
        <h1><u>Title</u>: {$ticket_title} [ID#{$ticket_tId}] </h1>
	
	<table>
	    <tr>
		{if isset($isMod) and $isMod eq "TRUE"}
		<td>
		    Ticket Assigning:
		    {if $ticket_assignedTo eq 0}
		       <form id="assign_ticket" class="form-vertical" method="post" action="" >
			   <input type="hidden" name="ticket_id" value="{$ticket_tId}">
			   <input type="hidden" name="action" value="assignTicket">
			   <button type="submit">Assign Ticket</button>
		       </form>
		    {else if $ticket_assignedTo eq $user_id}
		       <form id="assign_ticket" class="form-vertical" method="post" action="" >
			   <input type="hidden" name="ticket_id" value="{$ticket_tId}">
			   <input type="hidden" name="action" value="unAssignTicket">
			   <button type="submit">Remove Assign</button>
		       </form>
		    {/if}
		</td>
		<td>
		    Forward to Group:	    
		    <form id="forward" class="form-vertical" method="post" action="">
		    <select name="group">
				<option></option>
			{foreach from=$sGroups key=k item=v}
				<option value="{$k}">{$v}</option>
			{/foreach}
		    </select>		
		    <input type="hidden" name="ticket_id" value="{$ticket_tId}">
		    <input type="hidden" name="action" value="forward">
		    <button type="submit">Forward</button>
		    </form>
		</td>
		{/if}		
		{if isset($isMod) and $isMod eq "TRUE"}<td><a href="ams?page=show_ticket_log&id={$ticket_tId}">Show Ticket Log</a></td>{/if}
		
		 <td><a href="ams?page=createticket&user_id={$ticket_author}">Send Other Ticket</a></td>
		 {if $hasInfo}<td><a href="ams?page=show_ticket_info&id={$ticket_tId}">Show ticket Info</a></td>{/if}
		
	     </tr>
	</table>
		
		
			     
	 {if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_ASSIGNED"}
	 <font color="green">
		 <p>{$success_assigned}</p>
	 </font>
	 {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_UNASSIGNED"}
	 <font color="green">
		 <p>{$success_unassigned}</p>
	 </font>
	 {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
	 <font color="red">
		 <p>{$ticket_not_existing}</p>
	 </font>
	 {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "ALREADY_ASSIGNED"}
	 <font color="red">
		 <p>{$ticket_already_assigned}</p>
	 </font>
	 {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "NOT_ASSIGNED"}
	 <font color="red">
		 <p>{$ticket_not_assigned}</p>
	 </font>
	 {/if}
	 
	 {if isset($ACTION_RESULT) and $ACTION_RESULT eq "INVALID_SGROUP"}
	 <font color="red">
		 <p>{$invalid_sgroup}</p>
	 </font>
	 {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
	 <font color="red">
		 <p>{$ticket_not_existing}</p>
	 </font>
	 {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_FORWARDED"}
	 <font color="green">
		 <p>{$success_forwarded}</p>
	 </font>
	 {/if}
	 
	<form id="changeTicket"  method="post" action="ams?page=show_ticket&id={$ticket_tId}">
	<table width=100%>
		<tr>
		    <td width=33%><strong>Original Submitted: </strong>{$ticket_timestamp}</td>
		    <td width=33%><strong>Last Updated: </strong>{$ticket_lastupdate}</td>
		    <td width=33%><strong>Status: </strong>{if $ticket_status neq 3}<font color="green">Open</font>{/if} <font color="{if $ticket_status eq 0}green{else if $ticket_status eq 1}orange{else if $ticket_status eq 3}red{/if}"><strong>{$ticket_statustext}</strong></font></td> 
	      </tr>
		<tr>
		    <td width=33%><strong>Category: </strong>{$ticket_category}</td>
		    <td width=33%><strong>Priority: </strong>{$ticket_prioritytext}</td>
		    <td width=33%><strong>Support Group: </strong>
			<span class="label label-info">
			    {if $ticket_forwardedGroupName eq "0"}
				<i>{$public_sgroup}</i>
			    {else}
				<a href="ams?page=show_sgroup&id={$ticket_forwardedGroupId}">{$ticket_forwardedGroupName}</a>
			    {/if}
			</span>
		    </td>                  
		</tr>
		<tr>
		    <td width=33%><strong>Assigned To: </strong>{if $ticket_assignedTo neq ""} <a href="ams?page=show_user&id={$ticket_assignedTo}">{$ticket_assignedToText}</a> {else}<i> {$not_assigned}</i> {/if}</td>
		    <td width=33%></td>
		    <td width=33%></td> 
	      </tr>
	</table>
		
	
	<table class="table table-bordered" >
	    <tbody>
		{foreach from=$ticket_replies item=reply}
		<tr>
		    <td>
			<p>
			    <font color="blue"> {$reply.timestamp}</font>
			    {if $reply.permission eq '1'}
			    <span class="label label-success"><strong>{if isset($isMod) and $isMod eq "TRUE"} <a href="ams?page=show_user&id={$reply.authorExtern}"> {$reply.author}{else} {$reply.author} {/if}</a></strong></span>
			    {else if $reply.permission gt '1'}
			    <span class="label label-warning"><strong>{if isset($isMod) and $isMod eq "TRUE"} <a href="ams?page=show_user&id={$reply.authorExtern}"><font color="red">{$reply.author}</font>{else}<font color="red"> {$reply.author} </font>{/if}</a></strong></span>
			    {/if}
			</p>
			<p><pre{if $reply.permission gt '1'} {if $reply.hidden eq 0} style="background-color:rgb(248, 200, 200);"{else if $reply.hidden eq 1}style="background-color:rgb(207, 254, 255);"{/if}{/if}> {if $reply.hidden eq 1}<i>{/if}{$reply.replyContent}{if $reply.hidden eq 1}</i>{/if}</pre></p>
		    </td>
		</tr>
		{/foreach}
		
		{if $ticket_status eq 3}
		<tr>
		    <td>
			<p><pre style="background-color:rgb(255, 230, 153);">Ticket is closed.</pre></p>
		    </td>
		</tr>
		{/if}
		
		<tr>
		    <td>
			<form id="reply" class="form-vertical" method="post" action="ams?page=show_ticket&id={$ticket_tId}">
			{if $ticket_status neq 3}
			    <legend>{$t_reply}:</legend>
			    <label>{$t_fill}</label>
			    <textarea rows="6" class="span12" style="width: 90%;" id="Content" name="Content"></textarea>
			    {if isset($isMod) and $isMod eq "TRUE"}	     
				<label>Options</label>
				<input type="checkbox" name="hidden">Hide reply for user.
			    {/if}
			{/if}
		    </td>
		</tr>
		<tr>
		    <td>
			{if isset($isMod) and $isMod eq "TRUE"}
			<div style="display: inline-block;">
			    <label>Change status to</label>
			    <select name="ChangeStatus">
				{foreach from=$statusList key=k item=v}
					<option value="{$k}">{$v}</option>
				{/foreach}
			    </select>	
			</div>
			<div style="display: inline-block; margin-left:10px;"">
			    <label>Change priority to</label>		    
			    <select name="ChangePriority">
			        {foreach from=$ticket_priorities key=k item=v}
				        <option value="{$k}" {if $k eq $ticket_priority}selected="selected"{/if}>{$v}</option>
			        {/foreach}
			    </select>	
			</div>
			{/if}
		    </td>
		</tr>
		<tr>
		    <td>
			<input type="hidden" name="function" value="reply_on_ticket">
			<input type="hidden" name="ticket_id" value="{$ticket_id}">
			<button type="submit" class="btn btn-primary" >{$t_send}</button>
			</form>
		    </td>
		</tr>
	    </tbody>
	</table>
	  
       
   
{/block}
	
