
{include file="page_header.tpl"}

<table width="100%" cellpadding="2" cellspacing="0" border="0">
	<tr>
		<td align="left" valign="center"><span class="alert">{$tool_alert_message}</span></td>
		<td align="right">
			<table cellpadding="1" cellspacing="5" border="0">
				<tr>
					{section name=onemenu loop=$tool_menu}
					<td height="22" class="boxed"><a href="{$tool_menu[onemenu].uri}">{$tool_menu[onemenu].title}</a></td>
					{/section}
				</tr>
			</table>
		</td>
	</tr>
</table>

<br>

<table width="100%" border="0" cellpadding="0">
<tr>
	<td align="left" valign="top" width="70%">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Restart Groups</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Name</b></td>
			<td><b>List</b></td>
			<td><b>Order</b></td>
		</tr>
{section name=restart loop=$tool_restart_list}
{cycle assign="trclass" values="row1,row0"}
		<tr class="{$trclass}">
			<td>{$tool_restart_list[restart].restart_group_id}</td>
			<td><a href="tool_administration.php?toolmode=restarts&toolaction=edit&restart_id={$tool_restart_list[restart].restart_group_id}">{$tool_restart_list[restart].restart_group_name}</a></td>
			<td>{$tool_restart_list[restart].restart_group_list}</td>
			<td>{$tool_restart_list[restart].restart_group_order}</td>
		</tr>
{/section}
		</table>
	</td>


	<td align="right" valign="top" width="30%">
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=restarts" method="post">
		<tr>
			<th colspan="10">Restart Group Details</th>
		</tr>
{if $tool_restart_edit_data.restart_group_id}
		<tr>
			<td align="right">Id :</td>
			<td><input type="text" name="tool_form_restart_id" value="{$tool_restart_edit_data.restart_group_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td align="right">Name :</td>
			<td><input type="text" name="tool_form_restart_name" value="{$tool_restart_edit_data.restart_group_name}" maxlength="128" size="40"></td>
		</tr>
		<tr>
			<td align="right">Services :</td>
			<td><input type="text" name="tool_form_restart_services" value="{$tool_restart_edit_data.restart_group_list}" maxlength="255" size="40"></td>
		</tr>
		<tr>
			<td align="right">Order :</td>
			<td><input type="text" name="tool_form_restart_order" value="{$tool_restart_edit_data.restart_group_order}" maxlength="3" size="10"></td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td>

{if $tool_restart_edit_data.restart_group_id}
				<input type="submit" name="toolaction" value="update">
				<input type="submit" name="toolaction" value="delete" onclick="if (confirm('Are you sure you want to DELETE the restart group &lt; {$tool_restart_edit_data.restart_group_name} &gt; ?')) return true; return false;">
{else}
				<input type="submit" name="toolaction" value="create">

{/if}

			</td>
		</tr>
		</form>
		</table>
	</td>

</tr>
</table>

<br>

<table width="100%" border="0" cellpadding="0">
<tr>
	<td align="left" valign="top" width="70%">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Restart Messages</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Language</b></td>
			<td><b>Name</b></td>
			<td><b>Value</b></td>
		</tr>
{section name=msg loop=$tool_message_list}
{cycle assign="trclass" values="row1,row0"}
		<tr class="{$trclass}">
			<td>{$tool_message_list[msg].restart_message_id}</td>
			<td>{$tool_message_list[msg].restart_message_lang}</td>
			<td><a href="tool_administration.php?toolmode=restarts&toolaction=editmsg&msg_id={$tool_message_list[msg].restart_message_id}">{$tool_message_list[msg].restart_message_name}</a></td>
			<td>{$tool_message_list[msg].restart_message_value}</td>
		</tr>
{/section}
		</table>
	</td>


	<td align="right" valign="top" width="30%">
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=restarts" method="post">
		<tr>
			<th colspan="10">Restart Message Details</th>
		</tr>
{if $tool_message_edit_data.restart_message_id}
		<tr>
			<td align="right">Id :</td>
			<td><input type="text" name="tool_form_message_id" value="{$tool_message_edit_data.restart_message_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td align="right">Name :</td>
			<td><input type="text" name="tool_form_message_name" value="{$tool_message_edit_data.restart_message_name}" maxlength="32" size="40"></td>
		</tr>
		<tr>
			<td align="right">Value :</td>
			<td><input type="text" name="tool_form_message_value" value="{$tool_message_edit_data.restart_message_value}" maxlength="255" size="40"></td>
		</tr>
		<tr>
			<td align="right">Language :</td>
			<td>
				<select name="tool_form_message_lang">
{section name=lang loop=$tool_language_list}
					<option value="{$tool_language_list[lang].lang_id}" {if $tool_message_edit_data.restart_message_lang == $tool_language_list[lang].lang_id}selected{/if}>{$tool_language_list[lang].lang_name}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td>

{if $tool_message_edit_data.restart_message_id }
				<input type="submit" name="toolaction" value="update message">
				<input type="submit" name="toolaction" value="delete message" onclick="if (confirm('Are you sure you want to DELETE the restart message &lt; {$tool_message_edit_data.restart_message_name} &gt; ?')) return true; return false;">
{else}
				<input type="submit" name="toolaction" value="create message">

{/if}

			</td>
		</tr>
		</form>
		</table>
	</td>

</tr>
</table>


{include file="page_footer.tpl"}
