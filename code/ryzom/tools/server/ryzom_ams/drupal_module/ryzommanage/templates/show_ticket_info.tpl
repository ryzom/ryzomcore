{block name=content}
<h1><a href="ams?page=show_ticket&id={$ticket_id}">[#{$ticket_id}] {$ticket_title}</a> </h1>

<h2>Actions</h2>
<table width=100%>
    <tr>
	{if isset($isMod) and $isMod eq "TRUE"}<td width=33%><a href="ams?page=show_ticket_log&id={$ticket_id}">Show Ticket Log</a></td>{/if}
	<td width=33%><a href="ams?page=createticket&user_id={$ticket_author}">Send Other Ticket</a></td>
	<td width=33%><a href="ams?page=show_ticket&id={$ticket_id}">Show Ticket</a></td>
    </tr>
</table>

<h2>Additional Info </h2>
<table>
	<tr>
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
	
{/block}
	
