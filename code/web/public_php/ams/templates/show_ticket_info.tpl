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
				<td><img src="{$IMAGELOC_WEBPATH}/info/shard.png"/><strong> Shard ID: </strong>{$shard_id}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/user.png"/><strong> User_Id: </strong>{$user_id}</td>
			    </tr>   
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/position.png"/><strong> User Position: </strong>{$user_position}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/view.png"/><strong> View Position: </strong>{$view_position}</td>
			    </tr>
    
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/client.png"/><strong> Client_Version: </strong> {$client_version}</td>              
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/patch.png"/><strong> Patch_Version: </strong>{$patch_version}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/server.png"/><strong> Server_Tick: </strong>{$server_tick}</td>
			    </tr>
			    <tr class="alert alert-info">
				<td><center><strong> Hardware & Software related </strong></center></td>
			    </tr>
			    <tr>
				<td><strong><img src="{$IMAGELOC_WEBPATH}/info/memory.png"/> Memory: </strong>{$memory}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/processor.png"/><strong> Processor: </strong>{$processor}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/cpuid.png"/><strong> Cpu_Id: </strong>{$cpu_id}</td>
			    </tr>			
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/mask.png"/><strong> Cpu_Mask: </strong>{$cpu_mask}</td>
			    </tr>
			    
			     <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/ht.png"/><strong> HT: </strong>{$ht}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/os.png"/><strong> OS: </strong>{$os}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/nel.png"/><strong> NeL3D: </strong>{$nel3d}</td>
			    </tr>
			    
			    <tr class="alert alert-info">
				<td><center><strong> Network related </strong></center></td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/connect.png"/><strong> Connect_State: </strong>{$connect_state}</td>
			    </tr>
			    <tr>
				<td><img src="{$IMAGELOC_WEBPATH}/info/local.png"/><strong> Local_Address: </strong>{$local_address}</td>
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
	
