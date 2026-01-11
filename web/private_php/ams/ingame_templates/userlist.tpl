{block name=content}
<tr><td>
      <table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
      <tr><td height="7"></td><td></td></tr>
      <tr>
	      <td width="3%"></td>
	      <td width="100%" height="12" valign="middle"><h1>Members</h1></td>
      </tr>
      <tr>
	<td height="5"></td><td></td>
      </tr>
      </table>
</td></tr>
<tr><td>
    <table width="100%" cellspacing="0" cellpadding="0" border="0">
      <tr bgcolor="#000000" valign="middle">
	<td>
	  <table>
	    <tr><td height="8"></td></tr>
	  </table>
	</td>
      </tr>
      <tr><td height="2"></td></tr>
      <tr><td height="1" bgcolor="#000000"></td></tr>
      <tr><td height="10"></td></tr>
      <tr valign="middle">
	<td>
	  <table width="100%" height="100%" cellpadding="10">
	    <tr><td>
	      <table width="100%" bgcolor="{$main_tbl_color}" border="2">
		      <tr><td>
			      <table width="100%" cellpadding="10">
				      <tr><td>
					      <p><h3>All Acounts</h3></p>
						<table width="100%" cellpadding="4" cellspacing="2">
							<tr bgcolor="{$table_header_tr_color}">
								<td>Id</td>
								<td>Username</td>
								<td>Email</td>
								<td>Permission</td>
								<td>Action</td>
							</tr>

				                       {foreach from=$userlist item=element}
							<tr>
								<td>{$element.id}</td>
								<td class="center"><a href="{$ingame_webpath}?page=show_user&id={$element.id}">{$element.username}</a></td>
								<td class="center">{$element.email}</td>
								{if $element.permission eq 1}<td class="center"><font color="{$user_color}">User</font></td>{/if}
								{if $element.permission eq 2}<td class="center"><font color="{$mod_color}">Moderator</font></td>{/if}
								{if $element.permission eq 3}<td class="center"><font color="{$admin_color}">Admin</font></td>{/if}
								<td class="center">
								  <a href="{$ingame_webpath}?page=show_user&id={$element.id}">Show User</a>
								  <a href="{$ingame_webpath}?page=settings&id={$element.id}">Edit User</a>
								  {if isset($isAdmin) and $isAdmin eq 'TRUE' and $element.id neq 1}
								     {if $element.permission eq 1}
								         <a href="{$ingame_webpath}?page=change_permission&user_id={$element.id}&value=2">Make Moderator</a>
									 <a href="{$ingame_webpath}?page=change_permission&user_id={$element.id}&value=3">Make Admin</a>
								     {else if $element.permission eq 2 }
								         <a href="{$ingame_webpath}?page=change_permission&user_id={$element.id}&value=1">Demote to User</a>
									 <a href="{$ingame_webpath}?page=change_permission&user_id={$element.id}&value=3">Make Admin</a>
								     {else if $element.permission eq 3 }
									 <a href="{$ingame_webpath}?page=change_permission&user_id={$element.id}&value=1">Demote to User</a>
									 <a href="{$ingame_webpath}?page=change_permission&user_id={$element.id}&value=2">Demote to Moderator</a>
								     {/if}		
								   {/if}
								</td>
								
							</tr>
							{/foreach}
					      </table>            
				      </td></tr>
				      <tr><td align = "center">
					 <table>
				           <tr>
				              <td><a href="{$ingame_webpath}?page=userlist&pagenum=1">&laquo;</a></td>
					      {foreach from=$links item=link}
				                 <td {if $link == $currentPage}bgcolor="{$pagination_current_page_bg}"{/if}><a href="{$ingame_webpath}?page=userlist&pagenum={$link}">{$link}</a></td>
				              {/foreach}
					      <td><a href="{$ingame_webpath}?page=userlist&pagenum={$lastPage}">&raquo;</a></td>
					   </tr>
					 </table>
				      </td></tr>
			      </table>
		      </td></tr>
	      </table>
	    </td></tr>
	  </table>
	</td>
      </tr>
    </table>
    
</td></tr>
				
{/block}

