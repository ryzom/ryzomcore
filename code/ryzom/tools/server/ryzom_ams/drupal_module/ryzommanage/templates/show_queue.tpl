{block name=content}
<h1>Ticket Queue {$queue_view}</h1>




<table>
    <tr>
	<td>
	    <table>
		<tr>
		    <td width=20%><a href="ams?page=show_queue&get=todo">Todo tickets</a></td>
		    <td width=20%><a href="ams?page=show_queue&get=all">All tickets</a></td>
		    <td width=20%><a href="ams?page=show_queue&get=all_open">All open tickets</a></td>
		    <td width=20%><a href="ams?page=show_queue&get=archive">Ticket Archive</a></td>
		    <td width=20%><a href="ams?page=show_queue&get=not_assigned">Not Assigned Tickets</a></td>
		</tr>
	    </table>
	</td>	
    </tr>
    <tr>
	<td>
	    <form id="create_queue" class="form-vertical" method="post" action="ams?page=show_queue&get=create" style="margin:0px 0px 0px;">
	    <table>
		<tr>
		    <td>
			<table width=100%>
			    <tr>
				<td>
				    Show
				</td>
				<td>
				    <select name="what">
					<option value="all" {if isset($prev_created_what) and $prev_created_what eq "all"}selected="selected"{/if}>all</option>
					<option value="waiting_for_support" {if isset($prev_created_what) and $prev_created_what eq "waiting_for_support"}selected="selected"{/if}>waiting for support</option>
					<option value="waiting_for_users" {if isset($prev_created_what) and $prev_created_what eq "waiting_for_users"}selected="selected"{/if}>waiting for user</option>
					<option value="closed" {if isset($prev_created_what) and $prev_created_what eq "closed"}selected="selected"{/if}>closed</option>
				    </select>
				</td>
				<td>
				    tickets
				</td>
				<td>
				    <select name="how">
					<option value="assigned" {if isset($prev_created_how) and $prev_created_how eq "assigned"}selected="selected"{/if}>assigned</option>
					<option value="not_assigned" {if isset($prev_created_how) and $prev_created_how eq "not_assigned"}selected="selected"{/if}>not assigned</option>
				    </select>
				</td>
				<td>
				    to
				</td>
				<td>
				    <select name="who" onchange="aimedforwhochanged(this.value);">
					<option value="user" {if isset($prev_created_who) and $prev_created_who eq "user"}selected="selected"{/if}>user</option>
					<option value="support_group" {if isset($prev_created_who) and $prev_created_who eq "support_group"}selected="selected"{/if}>support group</option>
				    </select>
				</td>
				<td>
				    <span id="userList" {if isset($prev_created_who) and $prev_created_who eq "user"}style="display:inline;"{else if isset($prev_created_who) and $prev_created_who eq "support_group"}style="display:none;"{else}style="display:inline;"{/if}>
					<select  name="userid">
					    {foreach from=$teamlist item=member}
						<option value="{$member.tUserId}" {if isset($prev_created_userid)} {if $prev_created_userid eq $member.tUserId}selected="selected"{/if}{else if $user_id eq $member.tUserId}selected="selected"{/if}>{$member.name}</option>
					    {/foreach}
					</select>
				    </span>
				</td>
				<td>
				    <span id="supportGroupList" {if isset($prev_created_who) and $prev_created_who eq "user"}style="display:none;"{else if isset($prev_created_who) and $prev_created_who eq "support_group"}style="display:inline;"{else}style="display:none;"{/if}>
				    <select name="groupid">
					{foreach from=$grouplist item=group}
					    <option value="{$group.sGroupId}" {if isset($prev_created_groupid) and  $prev_created_groupid eq $group.sGroupId}selected="selected"{/if}>{$group.name}</option>
					{/foreach}
				    </select>
				    </span>
				</td>
			    </tr>
			</table>
		    </td>
		</tr>
		<tr>
		    <td>
			<input type="hidden" name="action" value="create_queue">
			<button type="submit" style="bottom:4px; position:relative;">View</button>
		    </td>
		</tr>
	    </table>
	    </form>
	</td>
    </tr>
    <tr>
	<td>
	    {if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_ASSIGNED"}
	    <font color="green">
		    {$success_assigned}
	    </font>
	    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_UNASSIGNED"}
	    <font color="green">
		    {$success_unassigned}
	    </font>
	    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
	    <font color="red">
		    {$ticket_not_existing}
	    </font>
	    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "ALREADY_ASSIGNED"}
	    <font color="red">
		    {$ticket_already_assigned}
	    </font>
	    {else if isset($ACTION_RESULT) and $ACTION_RESULT eq "NOT_ASSIGNED"}
	    <font color="red">
		    {$ticket_not_assigned}
	    </font>
	    {/if}
	</td>
    </tr>
    <tr>
	<td>
	    <table width=100%>
		<thead>
			<tr>
				<th>ID</th>
				<th>Title</th>
				<th>Assigned</th>
				<th>Timestamp</th>
				<th>Category</th>
				<th>Status</th>
				<th>SupportGroup</th>
				<th>Actions</th>
			</tr>
		</thead>   
		<tbody>
		      {foreach from=$tickets item=ticket}
		      <tr>
			    <td>{$ticket.tId}</td>
			    <td><a href ="ams?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
			    <td>{if $ticket.assignedText neq ""} <a href="ams?page=show_user&id={$ticket.assigned}">{$ticket.assignedText} {else}<i> {$not_assigned}</i> {/if}</td>
			    <td class="center"><span title="{$ticket.timestamp_elapsed}" data-rel="tooltip"  data-placement="right">{$ticket.timestamp}</span></td>
			    <td class="center">{$ticket.category}</td>
			    <td class="center"><font color=" {if $ticket.status eq 0}green{else if $ticket.status eq 1}orange{else if $ticket.status eq 3}red{/if}"> {$ticket.statusText}</font></td>  
			    <td class="center">
			       
				{if $ticket.forwardedGroupName eq "0"}
				    <i>{$public_sgroup}</i>
				{else}
				     <span class="label label-info"><a href="ams?page=show_sgroup&id={$ticket.forwardedGroupId}">{$ticket.forwardedGroupName}</a></span> 
				{/if}
				  
			    </td>  
			    <td>
				{if $ticket.assigned eq 0}
				    <form id="assign_ticket" class="form-vertical" method="post" action="{$getURL}" style="margin:0px 0px 0px;">
					<input type="hidden" name="ticket_id" value="{$ticket.tId}">
					<input type="hidden" name="action" value="assignTicket">
					<button type="submit" class="btn btn-primary" ><i class="icon-flag icon-white"></i> Assign Ticket</button>
				    </form>
				{else if $ticket.assigned eq $user_id}
				    <form id="assign_ticket" class="form-vertical" method="post" action="{$getURL}" style="margin:0px 0px 0px;">
					<input type="hidden" name="ticket_id" value="{$ticket.tId}">
					<input type="hidden" name="action" value="unAssignTicket">
					<button type="submit" class="btn btn-warning" ><i class="icon-remove icon-white"></i> Remove Assign</button>
				    </form>
				{/if}
			    </td>
		      </tr>
		      {/foreach}
      
		</tbody>
	</table>
		
	    <center>
		    <a href="{$pagination_base_link}&pagenum=1">&laquo;</a> |
		    {foreach from=$links item=link}
		    <a href="{$pagination_base_link}&pagenum={$link}">{if $link == $currentPage}<u>{/if}{$link}{if $link == $currentPage}</u>{/if}</a> |
		    {/foreach}
		    <a href="{$pagination_base_link}&pagenum={$lastPage}">&raquo;</a>
	    </center>
	</td>
    </tr>
</table>

<!----- /javascript for this page -->
<script type="text/javascript">
    function aimedforwhochanged(value) 
{
	
if (value == "user") 
    {
    //hide the supportGroupList span
    var elem = document.getElementById("supportGroupList");
    elem.style.display="none";
    var elem2 = document.getElementById("userList");
    elem2.style.display="inline";
    }
else if(value == "support_group")
    {
    //hide the userList span
    var elem = document.getElementById("supportGroupList");
    elem.style.display= "inline";
    var elem2 = document.getElementById("userList");
    elem2.style.display="none";
    }
}
</script>
{/block}
	
