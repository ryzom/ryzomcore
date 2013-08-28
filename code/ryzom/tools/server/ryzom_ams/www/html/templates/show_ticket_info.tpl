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
		<div style=" padding-left:10%;padding-right:10%;">
		    <table class="table table-bordered table-condensed " >
			    <tr class="alert alert-info">
				<td><center><strong> Ingame related </strong></center></td>
			    </tr>
			    <tr>
				<td><img src="img/info/shard.png" height="30" /><strong> Shard ID: </strong>{$shard_id}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/user.png" height="30" /><strong> User_Id: </strong>{$user_id}</td>
			    </tr>   
			    <tr>
				<td><img src="img/info/position.png" height="30" /><strong> User Position: </strong>{$user_position}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/view.png" height="30" /><strong> View Position: </strong>{$view_position}</td>
			    </tr>
    
			    <tr>
				<td><img src="img/info/client.png" height="30" /><strong> Client_Version: </strong> {$client_version}</td>              
			    </tr>
			    <tr>
				<td><img src="img/info/patch.png" height="30" /><strong> Patch_Version: </strong>{$patch_version}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/server.png" height="30" /><strong> Server_Tick: </strong>{$server_tick}</td>
			    </tr>
			    <tr class="alert alert-info">
				<td><center><strong> Hardware & Software related </strong></center></td>
			    </tr>
			    <tr>
				<td><strong><img src="img/info/memory.png" height="30" /> Memory: </strong>{$memory}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/processor.png" height="30" /><strong> Processor: </strong>{$processor}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/cpuid.png" height="30" /><strong> Cpu_Id: </strong>{$cpu_id}</td>
			    </tr>			
			    <tr>
				<td><img src="img/info/mask.png" height="30" /><strong> Cpu_Mask: </strong>{$cpu_mask}</td>
			    </tr>
			    
			     <tr>
				<td><img src="img/info/ht.png" height="30" /><strong> HT: </strong>{$ht}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/os.png" height="30" /><strong> OS: </strong>{$os}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/nel.png" height="30" /><strong> NeL3D: </strong>{$nel3d}</td>
			    </tr>
			    
			    <tr class="alert alert-info">
				<td><center><strong> Network related </strong></center></td>
			    </tr>
			    <tr>
				<td><img src="img/info/connect.png" height="30" /><strong> Connect_State: </strong>{$connect_state}</td>
			    </tr>
			    <tr>
				<td><img src="img/info/local.png" height="30" /><strong> Local_Address: </strong>{$local_address}</td>
			    </tr>
			   
		    </table>
		</div>
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
	
