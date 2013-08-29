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
				<div class="box-icon">
					<a href="#" class="btn btn-round" onclick="javascript:show_help('intro');return false;"><i class="icon-info-sign"></i></a>
					<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<p><strong>{$home_info}</strong></p>
	
				<div class="clearfix"></div>
			</div>
		</div>
	</div>
{/block}

