{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span9">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-tag"></i> Ticket Queue {$queue_view}</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Tickets</legend>
		
		<div class="alert alert-info">
		    <form id="create_queue" class="form-vertical" method="post" action="index.php?page=show_queue&get=create" style="margin:0px 0px 0px;">
		    Show
		    <select style="width: 136px;" name="what">
			<option value="all" {if isset($prev_created_what) AND $prev_created_what eq "all"}selected="selected"{/if}>all</option>
			<option value="waiting_for_support" {if isset($prev_created_what) AND $prev_created_what eq "waiting_for_support"}selected="selected"{/if}>waiting for support</option>
			<option value="waiting_for_users" {if isset($prev_created_what) AND $prev_created_what eq "waiting_for_users"}selected="selected"{/if}>waiting for user</option>
			<option value="closed" {if isset($prev_created_what) AND $prev_created_what eq "closed"}selected="selected"{/if}>closed</option>
		    </select>
		    tickets
		    <select style="width: 110px;" name="how">
			<option value="assigned" {if isset($prev_created_how) AND $prev_created_how eq "assigned"}selected="selected"{/if}>assigned</option>
			<option value="not_assigned" {if isset($prev_created_how) AND $prev_created_how eq "not_assigned"}selected="selected"{/if}>not assigned</option>
			<option value="both" {if isset($prev_created_how) AND $prev_created_how eq "both"}selected="selected"{/if}>both</option>
		    </select>
		    to
		    <select style="width: 140px;" name="who" onchange="aimedforwhochanged(this.value);">
			<option value="user" {if isset($prev_created_who) AND $prev_created_who eq "user"}selected="selected"{/if}>user</option>
			<option value="support_group" {if isset($prev_created_who) AND $prev_created_who eq "support_group"}selected="selected"{/if}>support group</option>
		    </select>
		    <span id="userList" {if isset($prev_created_who) AND $prev_created_who eq "user"}style="display:inline;"{else if isset($prev_created_who) AND $prev_created_who eq "support_group"}style="display:none;"{else}style="display:inline;"{/if}>
		    <select style="width: 140px;" name="userid">
			{foreach from=$teamlist item=member}
			    <option value="{$member.tUserId}" {if isset($prev_created_userid)} {if $prev_created_userid eq $member.tUserId}selected="selected"{/if}{else if $user_id eq $member.tUserId}selected="selected"{/if}>{$member.name}</option>
			{/foreach}
		    </select>
		    </span>
		    <span id="supportGroupList" {if isset($prev_created_who) AND $prev_created_who eq "user"}style="display:none;"{else if isset($prev_created_who) AND $prev_created_who eq "support_group"}style="display:inline;"{else}style="display:none;"{/if}>
		    <select style="width: 140px;" name="groupid">
			{foreach from=$grouplist item=group}
			    <option value="{$group.sGroupId}" {if $prev_created_groupid eq $group.sGroupId}selected="selected"{/if}>{$group.name}</option>
			{/foreach}
		    </select>
		    </span>
		    <input type="hidden" name="action" value="create_queue">
		    <button type="submit" class="btn btn-primary" style="bottom:4px; position:relative;"  ><i class="icon-tag icon-white"></i> View</button>
		    </form>
		
		    
		</div>

				
		{if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_ASSIGNED"}
		<div class="alert alert-success">
			{$success_assigned}
		</div>
		{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_UNASSIGNED"}
		<div class="alert alert-success">
			{$success_unassigned}
		</div>
		{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
		<div class="alert alert-error">
			{$ticket_not_existing}
		</div>
		{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "ALREADY_ASSIGNED"}
		<div class="alert alert-error">
			{$ticket_already_assigned}
		</div>
		{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "NOT_ASSIGNED"}
		<div class="alert alert-error">
			{$ticket_not_assigned}
		</div>
		{/if}
		
		<table class="table table-striped table-bordered">
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
				<td><a href ="index.php?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
				<td>{if $ticket.assignedText neq ""} <a href="index.php?page=show_user&id={$ticket.assigned}">{$ticket.assignedText} {else}<i> {$not_assigned}</i> {/if}</td>
				<td class="center"><span title="{$ticket.timestamp_elapsed}" data-rel="tooltip"  data-placement="right">{$ticket.timestamp}</span></td>
				<td class="center">{$ticket.category}</td>
				<td class="center"><span class="label {if $ticket.status eq 0}label-success{else if $ticket.status eq 1}label-warning{else if $ticket.status eq 2}label-important{/if}">{if $ticket.status eq 0} <i class="icon-exclamation-sign icon-white"></i>{/if} {$ticket.statusText}</span></td>  
				<td class="center">
				   
				    {if $ticket.forwardedGroupName eq "0"}
					<i>{$public_sgroup}</i>
				    {else}
					 <span class="label label-info"><a href="index.php?page=show_sgroup&id={$ticket.forwardedGroupId}"><font color="white">{$ticket.forwardedGroupName}</font></a></span> 
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
		
	    <div style="width: 300px; margin:0px auto;">
		<ul class="pagination">
		    <li><a href="{$pagination_base_link}&pagenum=1">&laquo;</a></li>
		    {foreach from=$links item=link}
		    <li {if $link == $currentPage}class="active"{/if}><a href="{$pagination_base_link}&pagenum={$link}">{$link}</a></li>
		    {/foreach}
		    <li><a href="{$pagination_base_link}&pagenum={$lastPage}">&raquo;</a></li>
		</ul>
	    </div>
	    </div>
	</div>
    </div><!--/span-->
    
    <div class="box span3">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-th"></i>Actions</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
    		<div class="btn-group">
                <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
                <ul class="dropdown-menu">
		    <li class="divider"></li>
		    <li><a href="index.php?page=show_queue&get=todo">Todo tickets</a></li>
		    <li><a href="index.php?page=show_queue&get=all">All tickets</a></li>
		    <li><a href="index.php?page=show_queue&get=all_open">All open tickets</a></li>
		    <li><a href="index.php?page=show_queue&get=archive">Ticket Archive</a></li>
		    <li><a href="index.php?page=show_queue&get=not_assigned">Not Assigned Tickets</a></li>
		    <li class="divider"></li>
                </ul>
              </div>
            </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->




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
	
