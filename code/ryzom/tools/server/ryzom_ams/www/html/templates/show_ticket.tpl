{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span9">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-tag"></i>{$t_title} #{$ticket_tId} </h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
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
			    <td><strong>Associated: </strong><span class="label label-info">Ticket#33</span></td>                  
			</tr> 
		</table>
		
		
		<table class="table table-bordered" >
		    <tbody>
			{foreach from=$ticket_replies item=reply}
			<tr>
			    <td>
				<p><span class="label label-info"> {$reply.timestamp}</span>
				{if $reply.permission eq '1'}
				<!-- <span class="label label-important"><strong></i>[User]:</strong></span>-->
				{else if $reply.permission gt '1'}
			        <span class="label label-important"><strong><i class="icon-star icon-white"></i>[CSR]</strong></span>
				{/if}
				<span class="label label-warning"><strong><i class="icon-user icon-white"></i>{if isset($isAdmin) and $isAdmin eq "TRUE"} <a href="index.php?page=show_user&id={$reply.authorExtern}"><font color="white">{$reply.author}</font>{else}{$reply.author} {/if}</a></strong></span></p>

				<p><pre{if $reply.permission gt '1'} style="background-color:rgb(248, 200, 200);"{/if}>{$reply.replyContent}</pre></p>
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
				{/if}
				{if isset($isAdmin) and $isAdmin eq "TRUE"}
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
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
		
		<form id="addTag" class="form-vertical" method="post" action="index.php">
		<legend>Tags</legend>
		
		<div class="control-group">
		    <label class="control-label">Current Tags</label>
		    <div class="controls">
			<div class="input-prepend">
			   <div id="checkbox1" class="checker"><span class="checked"><input style="opacity: 0;" id="inlineCheckbox2" value="option2" checked="checked" type="checkbox"></span></div> Hacked
			   <div id="checkbox1" class="checker"><span class="checked"><input style="opacity: 0;" id="inlineCheckbox2" value="option2" checked="checked" type="checkbox"></span></div> Botanic
			   <div id="checkbox1" class="checker"><span class="checked"><input style="opacity: 0;" id="inlineCheckbox2" value="option2" checked="checked" type="checkbox"></span></div> evilwebsite.comz
			   <div id="checkbox1" class="checker"><span class="checked"><input style="opacity: 0;" id="inlineCheckbox2" value="option2" checked="checked" type="checkbox"></span></div> keylogger
			</div>
		    </div>
		</div>
		<div class="control-group">
		    <label class="control-label">New Tag</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" class="span8" id="newTag" name="newTag">
			</div>
		    </div>
		</div>
		<div class="control-group">
		    <label class="control-label"></label>
		    <div class="controls">
			<button type="submit" class="btn btn-primary" >Update</button>
		    </div>
		</div>
		</form>
		
		<form id="addTag" class="form-vertical" method="post" action="index.php">
		<legend>Associations</legend>
		
		<div class="control-group">
		    <label class="control-label">Current Associations</label>
		    <div class="controls">
			<div class="input-prepend">
			   <div id="checkbox1" class="checker"><span class="checked"><input style="opacity: 0;" id="inlineCheckbox2" value="option2" checked="checked" type="checkbox"></span></div> Ticket #33
			</div>
		    </div>
		</div>
		<div class="control-group">
		    <label class="control-label">New Association</label>
		    <div class="controls">
			<div class="input-prepend">
			    <input type="text" class="span8" id="newTag" name="newTag">
			</div>
		    </div>
		</div>
		<div class="control-group">
		    <label class="control-label"></label>
		    <div class="controls">
			<button type="submit" class="btn btn-primary" >Update</button>
		    </div>
		</div>
		</form>
		<legend>Actions</legend>
		<div class="btn-group">
		    <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
		    <ul class="dropdown-menu">
			<li class="divider"></li>
			{if isset($isAdmin) and $isAdmin eq "TRUE"}<li><a href="index.php?page=show_ticket_log&id={$ticket_tId}">Show Ticket Log</a></li>{/if}
			<li><a href="index.php?page=createticket&user_id={$target_id}">Send Other Ticket</a></li>
			<li class="divider"></li>
		    </ul>
		</div>
            </div>                   
        </div>
    </div>
</div><!--/row-->
{/block}
	
