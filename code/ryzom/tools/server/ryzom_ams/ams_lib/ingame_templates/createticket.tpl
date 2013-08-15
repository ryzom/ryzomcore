{block name=content}

    <tr><td>
	<table width="100%" bgcolor="{$title_bg_color}" cellspacing="2">
	<tr><td height="7"></td><td></td></tr>
	<tr>
		<td width="3%"></td>
		<td width="100%" height="12" valign="middle"><h1>Create a new ticket</h1></td>
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
				<table cellpadding="10">
					<tr><td>
					    <form id="changePassword" class="form-vertical" method="post" action="index.php?page=createticket&id={$target_id}">
						<table cellspacing="3">
						    <tr>
							<td valign="middle">Title: </td>
							<td>
							    <input type="text" size="300" id="Title" name="Title">
							</td>
						    </tr>
						    <tr>
							<td valign="middle">Category: </td>
							<td>
							        <select name="Category">
								{foreach from=$category key=k item=v}
									<option value="{$k}">{$v}</option>
								{/foreach}
							    </select>	
							</td>
						    </tr>
						    <tr>
							<td valign="middle">Description:</td>
							<td><textarea cols="45" id="Content" name="Content"><br><br><br><br><br></textarea></td>
						    </tr>
						    <tr>
							<td>
							    <input type="hidden" name="function" value="create_ticket">
							    <input type="hidden" name="target_id" value="{$target_id}">
							    <input type="submit" value="Send Ticket"/>
							</td>  
						    </tr>
						</table>
					    </form>				
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
	
	
