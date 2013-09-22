{block name=content}

  <tr><td>
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Support Groups</h1></td>
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
	      {if isset($isAdmin) && $isAdmin eq 'TRUE'}
	      <tr><td>
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellpadding="10">
					<tr><td>
						<p><h3>Add a Support group</h3></p>
							    
						<form id="addSGroup" class="form-vertical" method="post" action="{$ingame_webpath}">
						    <table cellpadding="1">
							<tr>
							    <td>
								<table>
								    <tr>
									<td valign="middle">Group name: </td>
									<td><input type="text" maxlength="20" size="180"  id="Name" name="Name"></td>
								    </tr>
								    
								    <tr>
									<td valign="middle">Group Tag: </td>
									<td><input type="text" maxlength="4" size="180" id="Tag" name="Tag"></td>
								    </tr>
								    
								    <tr>
									<td valign="middle">Group EmailAddress: </td>
									<td><input type="text"  id="GroupEmail" size="180" name="GroupEmail"></td>
								    </tr>
								</table>
							    </td>
							    <td width=10></td>
							    <td>
								<table>	    
								    <tr>
									<td valign="middle">IMAP MailServer IP: </td>
									<td><input type="text"  id="IMAP_MailServer" size="180" name="IMAP_MailServer"></td>
								    </tr>
								    
								    <tr>
									<td valign="middle">IMAP Username: </td>
									<td><input type="text"  id="IMAP_Username" size="180" name="IMAP_Username"></td>
								    </tr>
			       
								    <tr>
									<td valign="middle">IMAP Password: </td>
									<td><input type="text" id="IMAP_Password" size="180" name="IMAP_Password"></td>
								    </tr>
								</table>
							    </td>
							</tr>	
						    </table>
						    <input type="hidden" name="function" value="add_sgroup">
						    <p><input type="submit" value="Add Group" /></p>

						    
						    {if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SUCCESS"}
						    <p>
							    <font color="green">{$group_success}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "NAME_TAKEN"}
						    <p>
							    <font color="red">{$group_name_taken}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "TAG_TAKEN"}
						    <p>
							    <font color="red">{$group_tag_taken}</font>
						    </p>
						    {else if isset($RESULT_OF_ADDING) and $RESULT_OF_ADDING eq "SIZE_ERROR"}
						    <p>
							    <font color="red">{$group_size_error}</font>
						    </p>
						    {/if}
						    </form>
					</td></tr>
				</table>
			</td></tr>
		</table>
	      </td></tr>
	      {/if}
	       <tr><td>
		<table width="100%" bgcolor="{$main_tbl_color}" border="2">
			<tr><td>
				<table cellspacing="10">
					<tr><td>
						<p><h3>All groups</h3></p>
						<table cellpadding="4">
						    <tr bgcolor="{$table_header_tr_color}">
							<td>ID</td>
							<td>Name</td>
							<td>Tag</td>
							<td>Email</td>
							{if isset($isAdmin) && $isAdmin eq 'TRUE'}<td>Action</td>{/if}
						    </tr>
		    
						    {foreach from=$grouplist item=group}
						      <tr>
							    <td>{$group.sGroupId}</td>
							    <td><a href ="{$ingame_webpath}?page=show_sgroup&id={$group.sGroupId}">{$group.name}</a></td>
							    <td class="center"><span class="label label-important" >{$group.tag}</span></td>
							    <td class="center">{$group.groupemail}</td>
							    {if isset($isAdmin) && $isAdmin eq 'TRUE'}<td class="center"><a href="{$ingame_webpath}?page=sgroup_list&delete={$group.sGroupId}"><font color="red">Delete</font></a></td>{/if}
						      </tr>
						    {/foreach}
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
	
