{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span9">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-tag"></i> <a href="index.php?page=show_ticket&id={$ticket_id}">[#{$ticket_id}] {$ticket_title}</a> </h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Additional Info </legend>
		
		
		<form id="changeTicket" class="form-vertical" method="post" action="index.php">
		<table class="table table-bordered table-condensed " >
			<tr>
			    <td><strong>Shard ID: </strong>{$shard_id}</td>
			</tr>
			<tr>
			    <td><strong>User Position: </strong>{$user_position}</td> 
			<tr>
			    <td><strong>View Position: </strong>{$view_position}</td>
			</tr>
			<tr>
			    <td><strong>client_version: </strong> {$client_version}</td>              
			</tr>
			<tr>
			    <td><strong>patch_version: </strong>{$patch_version}</td>
			</tr>
			<tr>
			    <td><strong>memory: </strong>{$memory}</td>
			</tr>
			
			<tr>
			    <td><strong>server_tick: </strong>{$server_tick}</td>
			</tr>
			<tr>
			    <td><strong>connect_state: </strong>{$connect_state}</td>
			</tr>
			<tr>
			    <td><strong>local_address: </strong>{$local_address}</td>
			</tr>			
			<tr>
			    <td><strong>os: </strong>{$os}</td>
			</tr>
			<tr>
			    <td><strong>processor: </strong>{$processor}</td>
			</tr>
			<tr>
			    <td><strong>cpu_id: </strong>{$cpu_id}</td>
			</tr>			
			<tr>
			    <td><strong>cpu_mask: </strong>{$cpu_mask}</td>
			</tr>
			<tr>
			    <td><strong>ht: </strong>{$ht}</td>
			</tr>
			<tr>
			    <td><strong>nel3d: </strong>{$nel3d}</td>
			</tr>            
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
		
		<legend style="margin-bottom:9px;">Actions</legend>
		<div class="btn-group">
		    <button class="btn btn-primary btn-large dropdown-toggle" data-toggle="dropdown">Actions<span class="caret"></span></button>
		    <ul class="dropdown-menu">
			<li class="divider"></li>
			{if isset($isMod) and $isMod eq "TRUE"}<li><a href="index.php?page=show_ticket_log&id={$ticket_id}">Show Ticket Log</a></li>{/if}
			<li><a href="index.php?page=createticket&user_id={$ticket_author}">Send Other Ticket</a></li>
			<li><a href="index.php?page=show_ticket&id={$ticket_id}">Show Ticket</a></li>
			<li class="divider"></li>
		    </ul>
		</div>
            </div>                   
        </div>
    </div>
</div><!--/row-->
{/block}
	
