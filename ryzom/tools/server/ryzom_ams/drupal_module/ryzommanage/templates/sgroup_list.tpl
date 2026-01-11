{block name=content}

    <h2>List of all Support Groups</h2>

		<table>
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Name</th>
				    <th>Tag</th>
				    <th>Email</th>
				    {if isset($isAdmin) && $isAdmin eq 'TRUE'}<th>Action</th>{/if}
			    </tr>
		    </thead>   
		    <tbody>
			{foreach from=$grouplist item=group}
			  <tr>
				<td>{$group.sGroupId}</td>
				<td><a href ="ams?page=show_sgroup&id={$group.sGroupId}">{$group.name}</a></td>
				<td class="center">{$group.tag}</td>
				<td class="center">{$group.groupemail}</td>
				{if isset($isAdmin) && $isAdmin eq 'TRUE'}<td class="center"><a href="ams?page=sgroup_list&delete={$group.sGroupId}"><font color="red">Delete</font></a></td>{/if}
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	   
    {if isset($isAdmin) && $isAdmin eq 'TRUE'}
    
            <h2>Add a support group</h2>
		
		<form id="addSGroup" class="form-vertical" method="post" action="ams?page=sgroup_list">
		    <table>
			<tr>
			    <td>
				<table>
				    <tr>
					<td>
					    <label>Group name</label>
					    <input type="text" maxlength="20"   id="Name" name="Name">
					</td>
					<td>
					    <label>Group Tag</label>
					    <input type="text" maxlength="4"  id="Tag" name="Tag">
					</td>
					<td>			
					    <label>Group EmailAddress</label>
					    <input type="text"  id="GroupEmail" name="GroupEmail">
					</td>
				    </tr>
				</table>
			    </td>
			</tr>
			<tr>
			    <td>
				<table>
				    <tr>
					<td>
					    <label>IMAP MailServer IP</label>
					    <input type="text"  id="IMAP_MailServer" name="IMAP_MailServer">
					</td>
					<td>		
					    <label class="control-label">IMAP Username</label>
					    <input type="text"  id="IMAP_Username" name="IMAP_Username">
					</td>
					<td>
					    <label class="control-label">IMAP Password</label>
					    <input type="password"   id="IMAP_Password" name="IMAP_Password">
					</td>
				    </tr>
				</table>
			    </td>
			</tr>
			<tr>
			    <td>
				<input type="hidden" name="function" value="add_sgroup">		
				<button type="submit" class="btn btn-primary" >Add</button>
			    </td>
			</tr>
		    </table>

		    
		</form>
		{if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SUCCESS"}
		<font color="green">
			<p>{$group_success}</p>
		</font>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "NAME_TAKEN"}
		<font color="red">
			<p>{$group_name_taken}</p>
		</font>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "TAG_TAKEN"}
		<font color="red">
			<p>{$group_tag_taken}</p>
		</font>
		{else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SIZE_ERROR"}
		<font color="red">
			<p>{$group_size_error}</p>
		</font>
		{/if}
	
    {/if}
{/block}
	
