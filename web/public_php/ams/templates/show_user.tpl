{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-9">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-user"></span> Profile of {$target_name}
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <legend>Info</legend>
		<table class="table table-striped" >
		    <tbody>
			<tr >
			    <td><strong>Email:</strong></td>
			    <td>{$mail}</td>
			</tr>

			<tr >
			    <td><strong>Role:</strong></td>
			    <td>
			    {if $userPermission eq 1}<span class="label label-success">User</span>{/if}
			    {if $userPermission eq 2}<span class="label label-warning">Moderator</span>{/if}
			    {if $userPermission eq 3}<span class="label label-important">Admin</span>{/if}
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
		    </tbody>
		</table>
	    </div>
        </div>
		</div>
    </div><!--/span-->

    <div class="box col-md-3">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-th"></span>Actions
        </div>
        <div class="panel-body">
            <div class="row-fluid">
		<div class="btn-group">
                <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
                <ul class="dropdown-menu">
		    <li class="divider"></li>
		    <li><a href="index.php?page=settings&id={$target_id}">Edit User</a></li>
		    <li><a href="index.php?page=createticket&user_id={$target_id}">Send Ticket</a></li>
		    <li class="divider"></li>
		    {if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			{if $userPermission eq 1}
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=2">Make Moderator</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=3">Make Admin</a></li>
			{else if $userPermission eq 2 }
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=1">Demote to User</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=3">Make Admin</a></li>
			{else if $userPermission eq 3 }
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=1">Demote to User</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=2">Demote to Moderator</a></li>
			{/if}
			<li class="divider"></li>
		    {/if}

                </ul>
              </div>
            </div>
        </div>
		</div>
    </div><!--/span-->
</div><!--/row-->

<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-9">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-tag"></span> Tickets of {$target_name}
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <legend>Tickets</legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Title</th>
				    <th>Timestamp</th>
				    <th>Category</th>
				    <th>Status</th>
			    </tr>
		    </thead>
		    <tbody>
			  {foreach from=$ticketlist item=ticket}
			  <tr>
				<td>{$ticket.tId}</td>
				<td><a href ="index.php?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
				<td class="center"><i>{$ticket.timestamp}</i></td>
				<td class="center">{$ticket.category}</td>

				<td class="center"><span class="label {if $ticket.status eq 0}label-success{else if $ticket.status eq 1}label-warning{else if $ticket.status eq 2}label-important{/if}">{if $ticket.status eq 0} <span class="icon-exclamation-sign icon-white"></span>{/if} {$ticket.statusText}</span></td>
			  </tr>
			  {/foreach}

		    </tbody>
	    </table>
	    </div>
	</div>
	</div>
    </div><!--/span-->
</div><!--/row-->
{/block}

