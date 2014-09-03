{block name=content}


	<div class="sortable row-fluid ui-sortable">
		<a data-original-title="{$nrAssignedWaiting} Assigned to you and waiting for support!" data-rel="tooltip" class="well span3 top-block"
		   href="index.php?page=show_queue&get=create&userid={$user_id}&groupid=1&what=waiting_for_support&how=assigned&who=user">
			<span class="icon32 icon-blue icon-alert"></span>
			<div>Tickets Waiting for Direct Action</div>
			<span class="notification red">{$nrAssignedWaiting}</span>
		</a>

		<a data-original-title="{$nrToDo} Tickets Todo." data-rel="tooltip" class="well span3 top-block" href="index.php?page=show_queue&get=todo">
			<span class="icon32 icon-blue icon-tag"></span>
			<div>Tickets Todo</div>
			<span class="notification red">{$nrToDo}</span>
		</a>

		<a data-original-title="By {$newestTicketAuthor}" data-rel="tooltip" class="well span3 top-block" href="index.php?page=show_ticket&id={$newestTicketId}">
			<span class="icon32 icon-blue icon-flag"></span>
			<div>Newest Ticket</div>
			<span class="notification blue">{$newestTicketTitle}</span>
		</a>

		<a data-original-title="{$nrTotalTickets} tickets in total" data-rel="tooltip" class="well span3 top-block" href="index.php?page=show_queue&get=all">
			<span class="icon32 icon-blue icon-archive"></span>
			<div>Total amount of Tickets</div>
			<span class="notification blue">{$nrTotalTickets}</span>
		</a>


	</div>

	<div class="row-fluid">
		<div class="box span12">
			<div class="box-header well">
				<h2><i class="icon-info-sign"></i> {$home_title}</h2>
			</div>
			<div class="box-content">
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


{/block}

