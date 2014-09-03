{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span9">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-tag"></i>{$t_title} #{$ticket_tId} </h2>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>{$title}: {$ticket_title} </legend>


		<form id="changeTicket" class="form-vertical" method="post" action="index.php">
		<table class="table table-bordered table-condensed ">
			<tr>
			    <td><strong>Original Submitted: </strong>{$ticket_timestamp}</td>
			    <td><strong>Last Updated: </strong>{$ticket_lastupdate}</td>
			    <td><strong>Status: </strong>{if $ticket_status neq 3}<span class="label label-success">Open</span>{/if} <span class="label {if $ticket_status eq 0}label-success{else if $ticket_status eq 1}label-warning{else if $ticket_status eq 2}label-important{/if}"><strong>{$ticket_statustext}</strong></span></td>
		      </tr>
			<tr>
			    <td><strong>Category: </strong>{$ticket_category}</td>
			    <td><strong>Priority: </strong>{$ticket_prioritytext}</td>
			    <td><strong>Support Group: </strong>
				<span class="label label-info">
				    {if $ticket_forwardedGroupName eq "0"}
					<i>{$public_sgroup}</i>
				    {else}
					<a href="index.php?page=show_sgroup&id={$ticket_forwardedGroupId}"><font color="white">{$ticket_forwardedGroupName}</font></a>
				    {/if}
				</span>
			    </td>
			</tr>
			<tr>
			    <td><strong>Assigned To: </strong>{if $ticket_assignedTo neq ""} <a href="index.php?page=show_user&id={$ticket_assignedTo}">{$ticket_assignedToText}</a> {else}<i> {$not_assigned}</i> {/if}</td>
			    <td></td>
			    <td></td>
		      </tr>
		</table>


		<table class="table table-bordered" >
		    <tbody>
			{foreach from=$ticket_replies item=reply}
			<tr>
			    <td>
				<p>
				    <span class="label label-info"> {$reply.timestamp}</span>
				    {if $reply.permission eq '1'}
				    <span class="label label-success"><strong><i class="icon-user icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="white"> {$reply.author}</font>{else} {$reply.author} {/if}</a></strong></span>
				    {else if $reply.permission gt '1'}
				    <span class="label label-warning"><strong><i class="icon-star icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="white"> {$reply.author}</font>{else} {$reply.author} {/if}</a></strong></span>
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
				<form id="reply" class="form-vertical" method="post" action="index.php">
				{if $ticket_status neq 3}
				    <legend>{$t_reply}:</legend>
				    <div class="control-group">
					<label class="control-label">{$t_fill}</label>
					<div class="controls">
					    <div class="input-prepend">
						<textarea rows="6" class="span12" id="Content" name="Content"></textarea>
					    </div>
					</div>
				    </div>
				    {if isset($isMod) and $isMod eq "TRUE"}
				     <div class="control-group">
					<label class="control-label">Options</label>
					<div class="controls">
					    <div class="input-prepend">
					<input type="checkbox" name="hidden">Hide reply for user.
					 </div>
					</div>
				    </div>
				    {/if}
				{/if}
				{if isset($isMod) and $isMod eq "TRUE"}
				<div class="control-group"  style="display: inline-block;">
				    <label class="control-label">Change status to</label>
				    <div class="controls">
					<select name="ChangeStatus">
					    {foreach from=$statusList key=k item=v}
						    <option value="{$k}">{$v}</option>
					    {/foreach}
					</select>
				    </div>
				</div>
				<div class="control-group"  style="display: inline-block; margin-left:10px;"">
				    <label class="control-label">Change priority to</label>
				    <div class="controls">
					<select name="ChangePriority">
					    {foreach from=$ticket_priorities key=k item=v}
						    <option value="{$k}" {if $k eq $ticket_priority}selected="selected"{/if}>{$v}</option>
					    {/foreach}
					</select>
				    </div>
				</div>
				{/if}
				<input type="hidden" name="function" value="reply_on_ticket">
				<input type="hidden" name="ticket_id" value="{$ticket_id}">
				<div class="control-group">
				    <label class="control-label"></label>
				    <div class="controls">
					<button type="submit" class="btn btn-primary" >{$t_send}</button>
				    </div>
				</div>
				</form>
			    </td>
			</tr>
		    </tbody>
		</table>
	    </div>
        </div>
    </div><!--/span-->



    <div class="box span3">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-th"></i>Actions</h2>
        </div>
        <div class="box-content">
            <div class="row-fluid">

		{if isset($isMod) and $isMod eq "TRUE"}

		 <legend style="margin-bottom:9px;">Ticket Assigning</legend>
		 {if $ticket_assignedTo eq 0}
		    <form id="assign_ticket" class="form-vertical" method="post" action="" style="margin:0px 0px 0px;">
			<input type="hidden" name="ticket_id" value="{$ticket_tId}">
			<input type="hidden" name="action" value="assignTicket">
			<button type="submit" class="btn btn-primary" style="margin-bottom:9px;><i class="icon-flag icon-white"></i> Assign Ticket</button>
		    </form>
		{else if $ticket_assignedTo eq $user_id}
		    <form id="assign_ticket" class="form-vertical" method="post" action="" style="margin:0px 0px 0px;">
			<input type="hidden" name="ticket_id" value="{$ticket_tId}">
			<input type="hidden" name="action" value="unAssignTicket">
			<button type="submit" class="btn btn-warning" style="margin-bottom:9px;><i class="icon-remove icon-white"></i> Remove Assign</button>
		    </form>
		{/if}

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

		<legend style="margin-bottom:9px;">Forward to Group</legend>
		<form id="forward" class="form-vertical" method="post" action="" style="margin-bottom:9px;" >

		<div class="control-group">
		    <div class="controls" >
			<select name="group">
				    <option></option>
			    {foreach from=$sGroups key=k item=v}
				    <option value="{$k}">{$v}</option>
			    {/foreach}
			</select>
		    </div>
		</div>
		<input type="hidden" name="ticket_id" value="{$ticket_tId}">
		<input type="hidden" name="action" value="forward">
		<div class="control-group">
		    <div class="controls">
			<button type="submit" class="btn btn-primary" >Forward</button>
		    </div>
		</div>
		</form>
		{if isset($ACTION_RESULT) and $ACTION_RESULT eq "INVALID_SGROUP"}
		<div class="alert alert-error">
			{$invalid_sgroup}
		</div>
		{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "TICKET_NOT_EXISTING"}
		<div class="alert alert-error">
			{$ticket_not_existing}
		</div>
		{else if isset($ACTION_RESULT) and $ACTION_RESULT eq "SUCCESS_FORWARDED"}
		<div class="alert alert-success">
			{$success_forwarded}
		</div>
		{/if}
		{/if}
		<legend style="margin-bottom:9px;">Other actions</legend>
		<div class="btn-group">
		    <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
		    <ul class="dropdown-menu">
			<li class="divider"></li>
			{if isset($isMod) and $isMod eq "TRUE"}<li><a href="index.php?page=show_ticket_log&id={$ticket_tId}">Show Ticket Log</a></li>{/if}
			<li><a href="index.php?page=createticket&user_id={$ticket_author}">Send Other Ticket</a></li>
			{if $hasInfo}<li><a href="index.php?page=show_ticket_info&id={$ticket_tId}">Show ticket Info</a></li>{/if}
			<li class="divider"></li>
		    </ul>
		</div>
            </div>
        </div>
    </div>
</div><!--/row-->
{/block}

