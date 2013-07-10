{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span12">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-user"></i>Ticket</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Title: {$ticket_title} </legend>
		<table class="table table-striped" >
		    <tbody>
			{foreach from=$ticket_replies item=reply}
			<tr>
			    <td>
				<p><span class="label label-info">[ID{$reply.tReplyId}] {$reply.timestamp}</span></p>
				<p><strong>{$username}</strong>{$reply.replyContent}</p>
			    </td>
			</tr>
			{/foreach}
			<tr>
			    <td>
				<legend>Reply on the ticket:</legend>
				<div class="control-group">
				    <label class="control-label">Description</label>
				    <div class="controls">
					<div class="input-prepend">
					    <textarea rows="8" class="span12" id="Content" name="Content"></textarea>
					</div>
				    </div>
				</div>
			    </td>
			</tr>
		    </tbody>
		</table>
	    </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
