
{include file="page_header.tpl"}

<br>

<table width="30%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
<form action="tool_preferences.php" method="post">
<tr>
	<th colspan="2">My Preferences</th>
</tr>
<tr>
	<td align="right">Login:</td>
	<td><b>{$tool_v_login}</b></td>
</tr>
<tr>
	<td align="right">Old Password:</td>
	<td><input type="password" name="tool_form_password_old" maxlength="8"></td>
</tr>
<tr>
	<td align="right">New Password:</td>
	<td><input type="password" name="tool_form_password_new" maxlength="8"></td>
</tr>
<tr>
	<td align="right">Menu Style:</td>
	<td><select name="tool_form_menu_style">
		<option value="0" {if $tool_v_menu == 0}selected{/if}>Text only</option>
		<option value="1" {if $tool_v_menu == 1}selected{/if}>Icon only</option>
		<option value="2" {if $tool_v_menu == 2}selected{/if}>Text and Icon</option>
		</select>
		</td>
</tr>
<tr>
	<td>&nbsp;</td>
	<td><input type="submit" name="toolaction" value="update"></td>
</tr>
{if $tool_error != null}
<tr>
	<td colspan="2" align="center"><b>{$tool_error}</b></td>
</tr>
{/if}
<input type="hidden" name="tool_form_user_id" value="{$tool_v_user_id}">
</form>
</table>

<br>
<table width="30%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
<form action="tool_preferences.php" method="post">
<tr>
	<th colspan="2">Default Application</th>
</tr>
<tr>
	<td align="right">Application :</td>
	<td><select name="tool_form_application_default" style="width:150px;">
		<option value="0"><i>Use Group Default</i></option>
		<option disabled > --------------- </option>
{section name=menu loop=$nel_menu}
{if $nel_menu[menu].application_order != 999999}
		<option value="{$nel_menu[menu].application_id}" {if $tool_v_application == $nel_menu[menu].application_id}selected{/if}>{$nel_menu[menu].application_name}</option>
{/if}
{/section}
	</select></td>
</tr>
<tr>
	<td>&nbsp;</td>
	<td><input type="submit" name="toolaction" value="update default application"></td>
</tr>
<input type="hidden" name="tool_form_user_id" value="{$tool_v_user_id}">
</form>
</table>


{include file="page_footer.tpl"}
