{block name=content}
<h1Members</h1>
<table>
  <thead>
	  <tr>
		  <th>Id</th>
		  <th>Username</th>
		  <th>Email</th>
		  <th>Permission</th>
		  <th>Action</th>
	  </tr>
  </thead>   
  <tbody>
	{foreach from=$userlist item=element}
	<tr>
		<td>{$element.id}</td>
		<td class="center"><a href="ams?page=show_user&id={$element.id}">{$element.username}</a></td>
		<td class="center">{$element.email}</td>
		{if $element.permission eq 1}<td class="center"><font color="green">User</font></td>{/if}
		{if $element.permission eq 2}<td class="center"><font color="orange">Moderator</font></td>{/if}
		{if $element.permission eq 3}<td class="center"><font color="red">Admin</font></td>{/if}
		<td class="center">
				<table>
				<tr><td>
				<a class="btn btn-primary" href="ams?page=show_user&id={$element.id}"><i class=" icon-eye-open icon-white"></i> Show User</a>
				</td>
				<td>
				<a href='{$base_webpath}/user/{$element.id}/edit'><i class=" icon-pencil icon-white"></i> Edit User</a>
				</td>
				{if isset($isAdmin) and $isAdmin eq 'TRUE' and $element.id neq 1}
					
						{if $element.permission eq 1}
						<td><a href="ams?page=change_permission&user_id={$element.id}&value=2">Make Moderator</a></td>
						<td><a href="ams?page=change_permission&user_id={$element.id}&value=3">Make Admin</a></td>
						{else if $element.permission eq 2 }
						<td><a href="ams?page=change_permission&user_id={$element.id}&value=1">Demote to User</a></td>
						<td><a href="ams?page=change_permission&user_id={$element.id}&value=3">Make Admin</a></td>
						{else if $element.permission eq 3 }
						<td><a href="ams?page=change_permission&user_id={$element.id}&value=1">Demote to User</a></td>
						<td><a href="ams?page=change_permission&user_id={$element.id}&value=2">Demote to Moderator</a></td>
						{/if}
				{/if}
				<tr>
				</table>
		</td>
	</tr>
	{/foreach}

  </tbody>
</table>
<center>

	      <a href="ams?page=userlist&pagenum=1">&laquo;</a> | 
	      {foreach from=$links item=link}
	      <a href="ams?page=userlist&pagenum={$link}">{if $link == $currentPage}<u>{/if}{$link}{if $link == $currentPage}</u>{/if}</a> |
	      {/foreach}
	      <a href="ams?page=userlist&pagenum={$lastPage}">&raquo;</a>

</center>
				
{/block}

