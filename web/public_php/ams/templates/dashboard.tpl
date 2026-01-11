{block name=content}


	<div class="sortable row-fluid ui-sortable">
		<a data-original-title="{$nrAssignedWaiting} Assigned to you and waiting for support!" data-rel="tooltip" class="well span3 top-block"
		   href="index.php?page=show_queue&get=create&userid={$user_id}&groupid=1&what=waiting_for_support&how=assigned&who=user">
			<span class="glyphicon glyphicon-exclamation-sign lg-icon red"></span>
			<div>Tickets Waiting for Direct Action</div>
			<span class="notification red">{$nrAssignedWaiting}</span>
		</a>

		<a data-original-title="{$nrToDo} Tickets Todo." data-rel="tooltip" class="well span3 top-block" href="index.php?page=show_queue&get=todo">
			<span class="glyphicon glyphicon-edit lg-icon yellow"></span>
			<div>Tickets Todo</div>
			<span class="notification red">{$nrToDo}</span>
		</a>

		<a data-original-title="By {$newestTicketAuthor}" data-rel="tooltip" class="well span3 top-block" {if $newestTicketId != null}href="index.php?page=show_ticket&id={$newestTicketId}"{/if}>
			<span class="glyphicon glyphicon-flag lg-icon green"></span>
			<div>Newest Ticket</div>
			<span class="notification blue">{if $newestTicketId != null}{$newestTicketTitle}{else}No Tickets!{/if}</span>
		</a>

		<a data-original-title="{$nrTotalTickets} tickets in total" data-rel="tooltip" class="well span3 top-block" href="index.php?page=show_queue&get=all">
			<span class="glyphicon glyphicon-briefcase lg-icon blue"></span>
			<div>Total amount of Tickets</div>
			<span class="notification blue">{$nrTotalTickets}</span>
		</a>


	</div>

	<div class="row-fluid">
		<div class="box col-md-12">
		<div class="panel panel-default">
			<div class="panel-heading">
				<span class="icon-info-sign"></span> {$home_title}
			</div>
			<div class="panel-body">
				<p><strong>{$home_info}</strong></p>
				<p>This is the GSOC project of Daan Janssens mentored by Matthew Lagoe.</p>
				<p>The features as admin covered in this project are:</p>
				<ul>
					<li>Manage user accounts</li>
					<li>Make users moderator or admin</li>
					<li>browse user's tickets</li>
					<li>Create a new ticket for a specific user as admin</li>
					<li>Create a new support group (and link an email to it)</li>
					<li>Add mods to support groups</li>
					<li>Assign ticket to you</li>
					<li>Forward ticket to a support group</li>
					<li>Add hidden messages to a ticket only viewable by other mods</li>
					<li>Browse ticket queues or create one dynamically</li>
					<li>Sync changes after the game server is back up after being down for a while</li>
					<li>Browse the log of a ticket</li>
					<li>Browse additional info sent along when a ticket is created ingame</li>
					<li>All the above can be done while ingame too</li>
				</ul>

				<div class="clearfix"></div>
			</div>
			</div>
		</div>
	</div>


{/block}

