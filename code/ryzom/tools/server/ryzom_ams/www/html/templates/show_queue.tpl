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
		    Show
		    <select style="width: 136px;" name="what">
			<option value="all">All</option>
			<option value="wfs">'waiting for support'</option>
			<option value="wfu">'waiting for user'</option>
			<option value="closed">'closed'</option>
		    </select>
		    tickets
		    <select style="width: 110px;" name="how">
			<option value="all">assigned</option>
			<option value="wfs">not assigned</option>
		    </select>
		    to
		    <select style="width: 140px;" name="who" onchange="aimedforwhochanged(this.value);">
			<option value="user">user</option>
			<option value="support_group">support group</option>
		    </select>
		    <span id="userList" style="display:inline;">
		    <select style="width: 140px;" name="name">
			<option value="all">Quitta</option>
			<option value="wfs">Botanic</option>
		    </select>
		    </span>
		    <span id="supportGroupList" style="display:none;">
		    <select style="width: 140px;" name="name">
			<option value="all">Developers</option>
			<option value="wfs">Webteam</option>
		    </select>
		    </span>
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
		
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
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
					<form id="assign_ticket" class="form-vertical" method="post" action="" style="margin:0px 0px 0px;">
					    <input type="hidden" name="ticket_id" value="{$ticket.tId}">
					    <input type="hidden" name="action" value="assignTicket">
					    <button type="submit" class="btn btn-primary" ><i class="icon-flag icon-white"></i> Assign Ticket</button>
					</form>
				    {else if $ticket.assigned eq $user_id}
					<form id="assign_ticket" class="form-vertical" method="post" action="" style="margin:0px 0px 0px;">
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
	
