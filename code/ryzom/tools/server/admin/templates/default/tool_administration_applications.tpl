
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
			<th colspan="10">Applications</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Name</b></td>
			<td><b>URI</b></td>
			<td><b>Restriction</b></td>
			<td><b>Icon</b></td>
			<td><b>Order</b></td>
			<td><b>Visible</b></td>
		</tr>
{section name=application loop=$tool_application_list}
{cycle assign="trclass" values="row1,row0"}
		<tr class="{$trclass}">
			<td>{$tool_application_list[application].application_id}</td>
			<td><a href="tool_administration.php?toolmode=applications&toolaction=edit&application_id={$tool_application_list[application].application_id}">{$tool_application_list[application].application_name}</a></td>
			<td>{$tool_application_list[application].application_uri}</td>
			<td>{$tool_application_list[application].application_restriction}</td>
			<td>{$tool_application_list[application].application_icon}</td>
			<td>{$tool_application_list[application].application_order}</td>
			<td>{if $tool_application_list[application].application_visible  == 1}Yes{else}No{/if}</td>
		</tr>
{/section}
		</table>
	</td>


	<td align="right" valign="top" width="30%">
		<table width="90%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_administration.php?toolmode=applications" method="post">
		<tr>
			<th colspan="10">Applications Details</th>
		</tr>
{if $tool_application_edit_data.application_id}
		<tr>
			<td align="right">Id :</td>
			<td><input type="text" name="tool_form_application_id" value="{$tool_application_edit_data.application_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td align="right">Name :</td>
			<td><input type="text" name="tool_form_application_name" value="{$tool_application_edit_data.application_name}" maxlength="64" size="30"></td>
		</tr>
		<tr>
			<td align="right">URI :</td>
			<td><input type="text" name="tool_form_application_uri" value="{$tool_application_edit_data.application_uri}" maxlength="255" size="30"></td>
		</tr>
		<tr>
			<td align="right">Restriction :</td>
			<td><input type="text" name="tool_form_application_restriction" value="{$tool_application_edit_data.application_restriction}" maxlength="64" size="30"></td>
		</tr>
		<tr>
			<td align="right">Icon :</td>
			<td><input type="text" name="tool_form_application_icon" value="{$tool_application_edit_data.application_icon}" maxlength="128" size="30"></td>
		</tr>
		<tr>
			<td align="right">Order :</td>
			<td><input type="text" name="tool_form_application_order" value="{$tool_application_edit_data.application_order}" maxlength="6" size="30"></td>
		</tr>
		<tr>
			<td align="right">Visible :</td>
			<td>
				<select name="tool_form_application_visible">
					<option value="1" {if $tool_application_edit_data.application_visible == 1}selected{/if}>Yes</option>
					<option value="0" {if $tool_application_edit_data.application_visible == 0}selected{/if}>No</option>
				</select>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td>

{if $tool_application_edit_data.application_id}
				<input type="submit" name="toolaction" value="update">
				<input type="submit" name="toolaction" value="delete" onclick="if (confirm('Are you sure you want to DELETE the application &lt; {$tool_application_edit_data.application_name} &gt; ?')) return true; return false;">
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
