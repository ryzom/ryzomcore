{block name=content}


	<table width="100%">
		<tr>
			<td width="25%">
				<a data-original-title="{$nrAssignedWaiting} Assigned to you and waiting for support!" data-rel="tooltip" class="well span3 top-block"
				   href="ams?page=show_queue&get=create&userid={$user_id}&groupid=1&what=waiting_for_support&how=assigned&who=user">
					<div style="text-align:center;">Tickets Waiting for Direct Action:
					<font color="red">{$nrAssignedWaiting}</font></div>
				</a>
			</td>
			<td width="25%">
				<a data-original-title="{$nrToDo} Tickets Todo." data-rel="tooltip" class="well span3 top-block" href="ams?page=show_queue&get=todo">
					<div style="text-align:center;">Tickets Todo:
					<font color="red">{$nrToDo}</font></div>
				</a>
			</td>
			<td width="25">
	
				<a data-original-title="By {$newestTicketAuthor}" data-rel="tooltip" class="well span3 top-block" href="ams?page=show_ticket&id={$newestTicketId}">
					<div style="text-align:center;">Newest Ticket:
					<font color="red">{$newestTicketTitle}</font></div>
				</a>
			</td>
			<td width="25%">
		
				<a data-original-title="{$nrTotalTickets} tickets in total" data-rel="tooltip" class="well span3 top-block" href="ams?page=show_queue&get=all">
					<div style="text-align:center;">Total amount of Tickets:
					<font color="red">{$nrTotalTickets}</font></div>
				</a>
			</td>
		</tr>
				
	
	</table>
	
	<div class="row-fluid">
		<div class="box span12">
			<div class="box-header well">
				<h2><i class="icon-info-sign"></i> {$home_title}</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-round" onclick="javascript:show_help('intro');return false;"><i class="icon-info-sign"></i></a>
					<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
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

