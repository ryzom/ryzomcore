
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
			<th colspan="10">Shards</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Name</b></td>
			<td><b>Shard ID</b></td>
			<td><b>Domain</b></td>
			<td><b>Language</b></td>
		</tr>
{section name=shard loop=$tool_shard_list}
{cycle assign="trclass" values="row1,row0"}
		<tr class="{$trclass}">
			<td>{$tool_shard_list[shard].shard_id}</td>
			<td><a href="tool_administration.php?toolmode=shards&toolaction=edit&shard_id={$tool_shard_list[shard].shard_id}">{$tool_shard_list[shard].shard_name}</a></td>
			<td>{$tool_shard_list[shard].shard_as_id}</td>
			<td>{$tool_shard_list[shard].domain_name}</td>
			<td>{$tool_shard_list[shard].shard_lang}</td>
		</tr>
{/section}
		</table>
	</td>


	<td align="right" valign="top" width="30%">
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=shards" method="post">
		<tr>
			<th colspan="10">Shard Details</th>
		</tr>
{if $tool_shard_edit_data.shard_id}
		<tr>
			<td align="right">Id :</td>
			<td><input type="text" name="tool_form_shard_id" value="{$tool_shard_edit_data.shard_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td align="right">Name :</td>
			<td><input type="text" name="tool_form_shard_name" value="{$tool_shard_edit_data.shard_name}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">Shard ID :</td>
			<td><input type="text" name="tool_form_shard_as_id" value="{$tool_shard_edit_data.shard_as_id}" maxlength="255" size="30"></td>
		</tr>
		<tr>
			<td align="right">Domain :</td>
			<td>
				<select name="tool_form_shard_domain_id">
{section name=domain loop=$tool_domain_list}
					<option value="{$tool_domain_list[domain].domain_id}" {if $tool_shard_edit_data.shard_domain_id == $tool_domain_list[domain].domain_id}selected{/if}>{$tool_domain_list[domain].domain_name}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td align="right">Language :</td>
			<td>
				<select name="tool_form_shard_language">
{section name=lang loop=$tool_language_list}
					<option value="{$tool_language_list[lang].lang_id}" {if $tool_shard_edit_data.shard_lang == $tool_language_list[lang].lang_id}selected{/if}>{$tool_language_list[lang].lang_name}</option>
{/section}
				</select>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td>

{if $tool_shard_edit_data.shard_id}
				<input type="submit" name="toolaction" value="update">
				<input type="submit" name="toolaction" value="delete" onclick="if (confirm('Are you sure you want to DELETE the shard &lt; {$tool_shard_edit_data.shard_name} &gt; ?')) return true; return false;">
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

{include file="page_footer.tpl"}
