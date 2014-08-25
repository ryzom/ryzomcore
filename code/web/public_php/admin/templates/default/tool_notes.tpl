
{include file="page_header.tpl"}

{literal}

<script language="Javascript" type="text/javascript">
<!--

	function toggleLine(myline)
	{
		if (document.all)
		{
			if (document.all.item(myline).style.display == "none")
			{
				document.all.item(myline).style.display = "";
			}
			else
			{
				document.all.item(myline).style.display = "none";
			}
		}
		else
		{
			if (document.getElementById(myline).style.display == "none")
			{
				document.getElementById(myline).style.display = "";
			}
			else
			{
				document.getElementById(myline).style.display = "none";
			}
		}
	}

//-->
</script>

{/literal}

<table width="100%" cellpadding="2" cellspacing="0" border="0">
	<tr>
		<td align="left" valign="center"><span class="alert">{$tool_alert_message}</span></td>
	</tr>
</table>

<br>

<table width="100%" border="0" cellpadding="0">
<tr>
	<td align="left" valign="top" width="50%">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Notes</th>
		</tr>
		<tr>
			<td><b>ID</b></td>
			<td><b>Title</b></td>
			<td><b>Mode</b></td>
			<td><b>Last Update</b></td>
			<td><b>Active</b></td>
{if $restriction_tool_notes_global}
			<td><b>Global</b></td>
{/if}
		</tr>
		{section name=note loop=$tool_note_list}
		<tr class="{cycle values="row1,row0"}">
			<td>{$tool_note_list[note].note_id}</td>
			<td><a href="tool_notes.php?note_id={$tool_note_list[note].note_id}">{$tool_note_list[note].note_title}</a></td>
			<td>{if $tool_note_list[note].note_mode  == 0}Text{else}Popup{/if}</td>
			<td>{$tool_note_list[note].note_date|date_format:"%Y/%m/%d %H:%M:%S"}</td>
			<td>{if $tool_note_list[note].note_active  == 1}Yes{else}No{/if}</td>
{if $restriction_tool_notes_global}
			<td>{if $tool_note_list[note].note_global  == 1}Yes{else}No{/if}</td>
{/if}
		</tr>
		{/section}
		</table>
	</td>

	<td>&nbsp;</td>

	<td align="right" valign="top" width="50%">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="tool_notes.php" method="post">
		<tr>
			<th colspan="10">Notes Details</th>
		</tr>
{if $tool_note_edit_data.note_id}
		<tr>
			<td align="right">Id :</td>
			<td><input type="text" name="tool_form_note_id" value="{$tool_note_edit_data.note_id}" size="10" readonly></td>
		</tr>
{/if}
		<tr>
			<td align="right">Title :</td>
			<td><input type="text" name="tool_form_note_title" value="{$tool_note_edit_data.note_title}" maxlength="64" size="100%"></td>
		</tr>

		<tr>
			<td align="right">Mode : </td>
			<td><select name="tool_form_note_mode" onchange="toggleLine('note_mode_text'); toggleLine('note_mode_popup_uri'); toggleLine('note_mode_popup_restriction');">
				<option value="text" {if $tool_note_edit_data.note_mode == 0}selected{/if}>Text</option>
				<option value="popup" {if $tool_note_edit_data.note_mode == 1}selected{/if}>Popup</option>
			</select></td>
		</tr>

		<tr id="note_mode_text" name="note_mode_text" {if $tool_note_edit_data.note_mode == 1}style="display: none;"{/if}>
			<td align="right">Text :</td>
			<td><textarea name="tool_form_note_data" rows="15">{$tool_note_edit_data.note_data}</textarea></td>
		</tr>

		<tr id="note_mode_popup_uri" name="note_mode_popup_uri" {if $tool_note_edit_data.note_mode == 0}style="display: none;"{/if}>
			<td align="right">URI :</td>
			<td><input type="text" name="tool_form_note_popup_uri" value="{$tool_note_edit_data.note_popup_uri}" maxlength="255" size="100%"></td>
		</tr>

		<tr id="note_mode_popup_restriction" name="note_mode_popup_restriction" {if $tool_note_edit_data.note_mode == 0}style="display: none;"{/if}>
			<td align="right">Restriction :</td>
			<td><input type="text" name="tool_form_note_popup_restriction" value="{$tool_note_edit_data.note_popup_restriction}" maxlength="64" size="100%"></td>
		</tr>

		<tr>
			<td align="right">Active :</td>
			<td>
				<select name="tool_form_note_active">
					<option value="1" {if $tool_note_edit_data.note_active == 1}selected{/if}>Yes</option>
					<option value="0" {if $tool_note_edit_data.note_active == 0}selected{/if}>No</option>
				</select>
			</td>
		</tr>
{if $restriction_tool_notes_global}
		<tr>
			<td align="right">Global :</td>
			<td>
				<select name="tool_form_note_global">
					<option value="1" {if $tool_note_edit_data.note_global == 1}selected{/if}>Yes</option>
					<option value="0" {if $tool_note_edit_data.note_global == 0}selected{/if}>No</option>
				</select>
			</td>
		</tr>
{/if}
		<tr>
			<td>&nbsp;</td>
			<td>

{if $tool_note_edit_data.note_id}
				<input type="submit" name="toolaction" value="update">
				<input type="submit" name="toolaction" value="delete" onclick="if (confirm('Are you sure you want to DELETE this note ?')) return true; return false;">
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
