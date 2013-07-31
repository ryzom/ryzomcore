{block name=content}

    <tr><td>
	<table width="100%" bgcolor="#303030" cellspacing="2">
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
		<table width="100%" bgcolor="#00000030" border="2">
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
							<td><textarea cols="50" height="400" id="Content" name="Content"></textarea></td>
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
	




{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span8">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-th"></i> Create a new Ticket</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <form id="changePassword" class="form-vertical" method="post" action="index.php?page=createticket&id={$target_id}">
                    <legend>New ticket</legend>
                    
                    <div class="control-group">
                        <label class="control-label">Title</label>
                        <div class="controls">
                            <div class="input-prepend">
                                <input type="text" class="span8" id="Title" name="Title">
                            </div>
                        </div>
                    </div>
                    
                    <div class="control-group">
                        <label class="control-label">Category</label>
                        <div class="controls">
                            <select name="Category">
                                {foreach from=$category key=k item=v}
                                        <option value="{$k}">{$v}</option>
                                {/foreach}
                            </select>	
                        </div>
                    </div> 
                    
                    <div class="control-group">
                        <label class="control-label">Description</label>
                        <div class="controls">
                            <div class="input-prepend">
				    <textarea rows="12" class="span12" id="Content" name="Content"></textarea>
                            </div>
                        </div>
                    </div>

                    <input type="hidden" name="function" value="create_ticket">
                    <input type="hidden" name="target_id" value="{$target_id}">
                    <div class="control-group">
                        <label class="control-label"></label>
                        <div class="controls">
                            <button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Send Ticket</button>
                        </div>
                    </div>
                </form>		
            </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
