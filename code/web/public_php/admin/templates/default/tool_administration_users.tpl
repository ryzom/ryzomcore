
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
			<th colspan="10">Accounts</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Login</b></td>
			<td><b>Group</b></td>
			<td><b>Created</b></td>
			<td><b>Last&nbsp;Logged</b></td>
			<td><b>Num&nbsp;Logs</b></td>
			<td><b>Active</b></td>
		</tr>
{section name=user loop=$tool_user_list}
{cycle assign="trclass" values="row1,row0"}
		<tr class="{$trclass}">
			<td>{$tool_user_list[user].user_id}</td>
			<td><a href="tool_administration.php?toolmode=users&toolaction=edit&user_id={$tool_user_list[user].user_id}">{$tool_user_list[user].user_name}</a></td>
			<td><a href="tool_administration.php?toolmode=groups&toolaction=edit&group_id={$tool_user_list[user].user_group_id}">{$tool_user_list[user].user_group_name}</a></td>
			<td>{$tool_user_list[user].user_created|date_format:"%Y/%m/%d %H:%M:%S"}</td>
{if $tool_user_list[user].user_logged_last > 0}
			<td>{$tool_user_list[user].user_logged_last|date_format:"%Y/%m/%d %H:%M:%S"}</td>
{else}
			<td>never</td>
{/if}
			<td>{$tool_user_list[user].user_logged_count}</td>
			<td>{if $tool_user_list[user].user_active == 1}Yes{else}No{/if}</td>
		</tr>
{/section}
		</table>
	</td>


	<td align="right" valign="top" width="30%">
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=users" method="post">
		<tr>
			<th colspan="10">Account Details</th>
		</tr>
{if $tool_user_edit_data.user_id}
		<tr>
			<td width="40%" align="right">Id :</td>
			<td><input type="text" name="tool_form_user_id" value="{$tool_user_edit_data.user_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td width="40%" align="right">Name :</td>
			<td><input type="text" name="tool_form_user_name" value="{$tool_user_edit_data.user_name}" maxlength="32" size="30"></td>
		</tr>
		<tr>
			<td width="40%" align="right">Password :</td>
			<td><input type="password" name="tool_form_user_password" value="" maxlength="16" size="30"></td>
		</tr>
		<tr>
			<td width="40%" align="right">Group :</td>
			<td>
				<select name="tool_form_user_group">
{section name=group loop=$tool_group_list}
					<option value="{$tool_group_list[group].group_id}" {if $tool_user_edit_data.user_group_id == $tool_group_list[group].group_id}selected{/if}>{$tool_group_list[group].group_name}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td width="40%" align="right">Active :</td>
			<td>
				<select name="tool_form_user_active">
					<option value="1" {if $tool_user_edit_data.user_active == 1}selected{/if}>Yes</option>
					<option value="0" {if $tool_user_edit_data.user_active == 0}selected{/if}>No</option>
				</select>
			</td>
		</tr>
		<tr>
			<td width="40%">&nbsp;</td>
			<td>

{if $tool_user_edit_data.user_id}
				<input type="submit" name="toolaction" value="update">
				<input type="submit" name="toolaction" value="delete" onclick="if (confirm('Are you sure you want to DELETE the user &lt; {$tool_user_edit_data.user_name} &gt; ?')) return true; return false;">
{else}
				<input type="submit" name="toolaction" value="create">

{/if}

			</td>
		</tr>
		</form>
		</table>

{if $tool_user_edit_data.user_id}

		<br>
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=users" method="post">
		<tr>
			<th colspan="10">Application Access</th>
		</tr>
		<tr>
			<td align="right" width="40%">Applications :</td>
			<td>
				<select name="tool_form_application_ids[]" multiple size="10" style="width:150px;">
{section name=appl loop=$tool_application_list}
					<option value="{$tool_application_list[appl].application_id}" {if $tool_application_list[appl].application_restriction == "" || $tool_application_list[appl].application_disabled}disabled{elseif $tool_application_list[appl].application_selected}selected{/if}>{$tool_application_list[appl].application_name}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td width="40%">&nbsp;</td>
			<td>
				<input type="hidden" name="tool_form_user_id" value="{$tool_user_edit_data.user_id}">
				<input type="submit" name="toolaction" value="update applications">
			</td>
		</tr>
		</form>
		</table>

		<br>
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=users" method="post">
		<tr>
			<th colspan="10">Domain Access</th>
		</tr>
		<tr>
			<td width="40%" align="right">Domains :</td>
			<td>
				<select name="tool_form_domain_ids[]" multiple size="5" style="width:150px;">
{section name=domain loop=$tool_domain_list}
					<option value="{$tool_domain_list[domain].domain_id}" {if $tool_domain_list[domain].domain_disabled}disabled{elseif $tool_domain_list[domain].domain_selected}selected{/if}>{$tool_domain_list[domain].domain_name}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td width="40%">&nbsp;</td>
			<td>
				<input type="hidden" name="tool_form_user_id" value="{$tool_user_edit_data.user_id}">
				<input type="submit" name="toolaction" value="update domains">
			</td>
		</tr>
		</form>
		</table>

		<br>
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=users" method="post">
		<tr>
			<th colspan="10">Shard Access {if $tool_domain_list[domain].domain_disabled}(group){/if}</th>
		</tr>
		<tr>
			<td width="40%" align="right">Shards :</td>
			<td>
				<select name="tool_form_shard_ids[]" multiple size="20" style="width:150px;">
{section name=domain loop=$tool_shard_list}
{if $tool_shard_list[domain].domain_visible}
					<option value="" disabled >Domain : {$tool_shard_list[domain].domain_name}</option>
{section name=shard loop=$tool_shard_list[domain].shard_list}
					<option value="{$tool_shard_list[domain].domain_id}_{$tool_shard_list[domain].shard_list[shard].shard_id}" {if $tool_shard_list[domain].shard_list[shard].shard_disabled}disabled{elseif $tool_shard_list[domain].shard_list[shard].shard_selected}selected{/if}> +-- {$tool_shard_list[domain].shard_list[shard].shard_name}</option>
{/section}
					<option disabled></option>
{/if}
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td width="40%">&nbsp;</td>
			<td>
				<input type="hidden" name="tool_form_user_id" value="{$tool_user_edit_data.user_id}">
				{* <input type="hidden" name="tool_form_domain_id" value="{$tool_shard_list[domain].domain_id}"> *}
				<input type="submit" name="toolaction" value="update shards">
			</td>
		</tr>
		</form>
		</table>

{/if}

	</td>

</tr>
</table>

{include file="page_footer.tpl"}
