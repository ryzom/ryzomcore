{block name=content}

    <h2>Create a new Ticket</h2>
    <form id="changePassword" method="post" action="ams?page=createticket&id={$target_id}">
    <table>
	<tr>
	    <td>
		<label>Title</label>
		<input type="text" size="60" id="Title" name="Title">
	    </td>
	</tr>
	<tr>
	    <td>	       
		<label>Category</label>
		<select name="Category">
		    {foreach from=$category key=k item=v}
			    <option value="{$k}">{$v}</option>
		    {/foreach}
		</select>
	    </td>
	</tr>
	<tr>
	    <td>			  
		<label>Description</label>
		<textarea rows="12" id="Content" style="width: 90%;" name="Content"></textarea>
	    </td>
	</tr>
	<tr>
	    <td>
		<input type="hidden" name="function" value="create_ticket">
		<input type="hidden" name="target_id" value="{$target_id}">		    
		<button type="submit">Send Ticket</button>
	    </td>
	</tr>
    </table>
    </form>
{/block}
	
