{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-12">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-tag"></span> Log of Ticket #{$ticket_id}
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <legend>Title: <a href="index.php?page=show_ticket&id={$ticket_id}">{$ticket_title}</a></legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Timestamp</th>
				    <th>Query</th>
			    </tr>
		    </thead>
		    <tbody>
			  {foreach from=$ticket_logs item=log}
			  <tr>
				<td>{$log.tLogId}</td>
				<td><span title="{$log.timestamp_elapsed}" data-rel="tooltip"  data-placement="right">{$log.timestamp}</span></td>
				<td>{$log.query}</td>
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
