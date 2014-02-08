{block name=content}

            <h2><i class="icon-user"></i> Profile of {$target_name}</h2>
         
		<table >
		    <tbody>
			<tr >
			    <td><strong>Email:</strong></td>
			    <td>{$mail}</td>                           
			</tr>
			
			<tr >
			    <td><strong>Role:</strong></td>
			    <td>
			    {if $userPermission eq 1}<font color="green">User</font>{/if}
			    {if $userPermission eq 2}<font color="orange">Moderator</font>{/if}
			    {if $userPermission eq 3}<font color="red">Admin</font>{/if}
			    </td>                           
			</tr>

		    </tbody>
		</table>
	
    
        
            <h2><i class="icon-th"></i>Actions</h2>
                <table width="100%">
		    <tr>
			<td width="25%">
			    <a href="ams?page=settings&id={$target_id}">Edit User</a>
			</td>
			<td width="25%">
			    <a href="ams?page=createticket&user_id={$target_id}">Send Ticket</a>
			</td>
			{if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			    {if $userPermission eq 1}
			    <td width="25%">
				<a href="ams?page=change_permission&user_id={$target_id}&value=2">Make Moderator</a>
			    </td>
			    <td width="25%">
				<a href="ams?page=change_permission&user_id={$target_id}&value=3">Make Admin</a>
			    </td>
			    {else if $userPermission eq 2 }
			    <td width="25%">
				<a href="ams?page=change_permission&user_id={$target_id}&value=1">Demote to User</a>
			    </td>
			    <td width="25%">
				<a href="ams?page=change_permission&user_id={$target_id}&value=3">Make Admin</a>
			    </td>
			    {else if $userPermission eq 3 }
			    <td width="25%">
				<a href="ams?page=change_permission&user_id={$target_id}&value=1">Demote to User</a>
			    </td>
			    <td width="25%">
				<a href="ams?page=change_permission&user_id={$target_id}&value=2">Demote to Moderator</a>
			    </td>
			    {/if}
			   
			{/if}
		    </tr>
                </table>
      
 
  
            <h2><i class="icon-tag"></i> Tickets of {$target_name}</h2>
		<table>
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Title</th>
				    <th>Timestamp</th>
				    <th>Category</th>
				    <th>Status</th>
			    </tr>
		    </thead>   
		    <tbody>
			  {foreach from=$ticketlist item=ticket}
			  <tr>
				<td>{$ticket.tId}</td>
				<td><a href ="ams?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
				<td class="center"><i>{$ticket.timestamp}</i></td>
				<td class="center">{$ticket.category}</td>

				<td class="center"><font color=" {if $ticket.status eq 0}green{else if $ticket.status eq 1}orange{else if $ticket.status eq 3}red{/if}">{$ticket.statusText}</span></td>  
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	

{/block}
	
