
{* this closes the global service form *}
{* due to incompatibilities with jscript functions with moz/ie i need to do individiual forms :( *}
</form>

{literal}
<script language="Javascript" type="text/javascript">
<!--
	var total_secs_restart;
	var total_secs_restart2;

	function TimerDownRestart(secs)
	{
		total_secs_restart = secs;
		CountDownRestart();
	}

	function CountDownRestart()
	{
		total_secs_restart--;
		if (total_secs_restart >= 0)
		{
			document.restart1.restart_wait_timer_fake.value = 'Waiting for '+ TimerDisplay(total_secs_restart) +' minutes !';
			downRestart=setTimeout("CountDownRestart()",1000);
		}
		else
		{
			/*document.restart1.restart_wait_timer_fake.disabled = false;*/
			document.restart1.submit();
		}
	}

	function TimerDownRestart2(secs)
	{
		total_secs_restart2 = secs;
		CountDownRestart2();
	}

	function CountDownRestart2()
	{
		total_secs_restart2--;
		if (total_secs_restart2 >= 0)
		{
			document.restart2.restart_shutdown_timer_fake.value = 'Waiting for '+ TimerDisplay(total_secs_restart2) +' minutes !';
			downRestart2=setTimeout("CountDownRestart2()",1000);
		}
		else
		{
			/*document.restart2.restart_shutdown_timer_fake.disabled = false;*/
			document.restart2.submit();
		}
	}

//-->
</script>
{/literal}

		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<input type="hidden" name="restart_sequence_id" value="{$tool_restart_info.restart_sequence_id}">
			<input type="hidden" name="restart_sequence_step" value="{$tool_restart_info.restart_sequence_step}">
			<input type="hidden" name="restart_shard_id" value="{$tool_restart_shard_id}">
			<input type="hidden" name="restart_su" value="{$tool_shard_su_name}">
			<input type="hidden" name="restart_egs" value="{$tool_restart_egs_name}">
			<input type="hidden" name="restart_stop_services" value="{$tool_restart_stop_actions}">

			<td align="center" width="100px"><b>Restart Sequence</b></td>
			<td><table width="100%" border="0" cellpadding="1">
{*
 **** STEP 0 ****
 *}
			<form action="index.php" method="post" name="restart0">
			<input type="hidden" name="restart_sequence_id" value="{$tool_restart_info.restart_sequence_id}">
			<input type="hidden" name="restart_sequence_step" value="{$tool_restart_info.restart_sequence_step}">
			<input type="hidden" name="restart_shard_id" value="{$tool_restart_shard_id}">
			<input type="hidden" name="restart_su" value="{$tool_shard_su_name}">
			<input type="hidden" name="restart_egs" value="{$tool_restart_egs_name}">
			<input type="hidden" name="restart_stop_services" value="{$tool_restart_stop_actions}">

			<tr {if $tool_restart_info.restart_sequence_step == 0}class="row_restart_active"{/if}>
				<td><u>Step 0</u></td>
				<td>
					<input type="hidden" name="restart_ws_state" value="{$tool_restart_ws_state}">
					<input type="submit" name="restart_check_ws" value="Stop the Shard" style="width:550px" {if $tool_restart_info.restart_sequence_step != 0}disabled{/if} onclick="if (confirm('Are you sure you want to STOP THIS SHARD ?\nCancelling the sequence after this point is a bad idea !')) return true; return false;"><br><br>
					<select name="restart_message_reboot_id" style="width:550px" {if $tool_restart_info.restart_sequence_step != 0}disabled{/if}>
{section name=reboot loop=$tool_restart_message_reboot_list}
						<option value="{$tool_restart_message_reboot_list[reboot].restart_message_id}" {if $tool_shard_language == $tool_restart_message_reboot_list[reboot].restart_message_lang}selected{/if}>[{$tool_restart_message_reboot_list[reboot].restart_message_lang}] - {$tool_restart_message_reboot_list[reboot].restart_message_value}</option>
{/section}
					</select>

				</td>
				<td align="right">
{if $tool_restart_info.restart_sequence_step == 0}
					<input type="submit" name="restart_end" value="Cancel" onclick="if (confirm('Are you sure you want to CANCEL the restart sequence ?')) return true; return false;">
{/if}
				</td>
			</tr>
			</form>

{*
 **** STEP 1 ****
 *}

			<form action="index.php" method="post" name="restart1">
			<input type="hidden" name="restart_sequence_id" value="{$tool_restart_info.restart_sequence_id}">
			<input type="hidden" name="restart_sequence_step" value="{$tool_restart_info.restart_sequence_step}">
			<input type="hidden" name="restart_shard_id" value="{$tool_restart_shard_id}">
			<input type="hidden" name="restart_su" value="{$tool_shard_su_name}">
			<input type="hidden" name="restart_egs" value="{$tool_restart_egs_name}">
			<input type="hidden" name="restart_stop_services" value="{$tool_restart_stop_actions}">
			<input type="hidden" name="restart_wait_timer" value="1">

			<tr><td colspan="3"><hr size="1"></td></tr>
			<tr {if $tool_restart_info.restart_sequence_step == 1}class="row_restart_active"{/if}>
				<td><u>Step 1</u></td>
				<td>{if $tool_restart_info.restart_sequence_step == 1}{math assign="restart_timer" equation="x - y" x=$tool_restart_info.restart_sequence_timer y=$system_time}{else}{assign var=restart_timer value='-1'}{/if}
{if $restart_timer >= 0}
					<input type="submit" name="restart_wait_timer_fake" value="Waiting for 10:00 minutes !" style="width:550px" disabled>
					<script language="Javascript" type="text/javascript">
						<!--
						TimerDownRestart({$restart_timer});
						-->
					</script>
{else}
					<input type="submit" name="restart_wait_timer_fake" value="Nothing to do !" style="width:550px" disabled>
{if $tool_restart_info.restart_sequence_step == 1}
					<script language="Javascript" type="text/javascript">
						<!--
							document.restart1.submit();
						-->
					</script>
{/if}
{/if}
				</td>
				<td align="right">
{if $tool_restart_info.restart_sequence_step == 1}
					<input type="submit" name="restart_cancel" value="Cancel" onclick="if (confirm('Are you sure you want to CANCEL the restart sequence ?')) return true; return false;">
{/if}
				</td>
			</tr>
			</form>

{*
 **** STEP 2 ****
 *}

			<form action="index.php" method="post" name="restart2">
			<input type="hidden" name="restart_sequence_id" value="{$tool_restart_info.restart_sequence_id}">
			<input type="hidden" name="restart_sequence_step" value="{$tool_restart_info.restart_sequence_step}">
			<input type="hidden" name="restart_shard_id" value="{$tool_restart_shard_id}">
			<input type="hidden" name="restart_su" value="{$tool_shard_su_name}">
			<input type="hidden" name="restart_egs" value="{$tool_restart_egs_name}">
			<input type="hidden" name="restart_stop_services" value="{$tool_restart_stop_actions}">
			<input type="hidden" name="restart_shutdown_timer" value="1">

			<tr><td colspan="3"><hr size="1"></td></tr>
			<tr {if $tool_restart_info.restart_sequence_step == 2}class="row_restart_active"{/if}>
				<td><u>Step 2</u></td>
				<td>{if $tool_restart_info.restart_sequence_step == 2}{math assign="restart_timer2" equation="x - y" x=$tool_restart_info.restart_sequence_timer y=$system_time}{else}{assign var=restart_timer2 value='-1'}{/if}
{if $restart_timer2 >= 0}
					<input type="submit" name="restart_shutdown_timer_fake" value="Waiting for 00:30 minutes !" style="width:550px" disabled>
					<script language="Javascript" type="text/javascript">
						<!--
						TimerDownRestart2({$restart_timer2});
						-->
					</script>
{else}
					<input type="submit" name="restart_shutdown_timer_fake" value="Nothing to do !" style="width:550px" disabled>
{if $tool_restart_info.restart_sequence_step == 2}
					<script language="Javascript" type="text/javascript">
						<!--
							document.restart2.submit();
						-->
					</script>
{/if}

{/if}
				</td>
				<td align="right">&nbsp;</td>
			</tr>
			</form>

{*
 **** STEP 3 ****
 *}

			<form action="index.php" method="post" name="restart3">
			<input type="hidden" name="restart_sequence_id" value="{$tool_restart_info.restart_sequence_id}">
			<input type="hidden" name="restart_sequence_step" value="{$tool_restart_info.restart_sequence_step}">
			<input type="hidden" name="restart_shard_id" value="{$tool_restart_shard_id}">
			<input type="hidden" name="restart_su" value="{$tool_shard_su_name}">
			<input type="hidden" name="restart_egs" value="{$tool_restart_egs_name}">
			<input type="hidden" name="restart_stop_services" value="{$tool_restart_stop_actions}">

			<tr><td colspan="3"><hr size="1"></td></tr>
			<tr {if $tool_restart_info.restart_sequence_step == 3}class="row_restart_active"{/if}>
				<td><u>Step 3</u></td>
				<td>
					<input type="hidden" name="restart_start_group_id" value="0">
					<input type="hidden" name="restart_start_group_list" value="">
					<input type="hidden" name="restart_stop_group_list" value="">

					<input type="submit" name="restart_stop_group" value="Stop All Services" style="width:200px" onclick="if (confirm('Are you sure you want to STOP these services ?')) {ldelim} document.restart3.restart_stop_group_list.value='{$tool_restart_stop_actions}'; return true; {rdelim} return false;" {if $tool_restart_info.restart_sequence_step != 3}disabled{/if}>{* The following services will be stopped : {$tool_restart_stop_actions} *}<br><br>

{section name=start loop=$tool_restart_start_actions}
					<input type="submit" name="restart_start_group" value="{$smarty.section.start.iteration}/{$smarty.section.start.total} - Start {$tool_restart_start_actions[start].restart_group_name}" style="width:200px" onclick="if (confirm('Are you sure you want to START these services ?')) {ldelim} document.restart3.restart_start_group_id.value='{$tool_restart_start_actions[start].restart_group_id}'; document.restart3.restart_start_group_list.value='{$tool_restart_start_actions[start].service_list}'; return true; {rdelim} return false;" {if $tool_restart_info.restart_sequence_step != 3}disabled{/if}> The following services will be started : {$tool_restart_start_actions[start].restart_group_list}<br><br>
{/section}

					<input type="submit" name="restart_over" value="Hand Over to Customer Support Teams" style="width:550px" {if $tool_restart_info.restart_sequence_step != 3}disabled{/if}>

				</td>
				<td align="right">
{if $tool_restart_info.restart_sequence_step == 3}
					<input type="submit" name="restart_giveup" value="Give up, Shard cannot be started" onclick="if (confirm('Are you sure you want to GIVE UP the restart sequence ?\nThis will leave the shard in its current state !')) return true; return false;">
{/if}
				</td>
			</tr>
			</form>

{*
 **** STEP 4 ****
 *}

			<form action="index.php" method="post" name="restart4">
			<input type="hidden" name="restart_sequence_id" value="{$tool_restart_info.restart_sequence_id}">
			<input type="hidden" name="restart_sequence_step" value="{$tool_restart_info.restart_sequence_step}">
			<input type="hidden" name="restart_shard_id" value="{$tool_restart_shard_id}">
			<input type="hidden" name="restart_su" value="{$tool_shard_su_name}">
			<input type="hidden" name="restart_egs" value="{$tool_restart_egs_name}">
			<input type="hidden" name="restart_stop_services" value="{$tool_restart_stop_actions}">

			<tr><td colspan="3"><hr size="1"></td></tr>
			<tr {if $tool_restart_info.restart_sequence_step == 4}class="row_restart_active"{/if}>
				<td><u>Step 4</u></td>
				<td>
					Success, the Shard is now in the hands of the Customer Support Team.
				</td>
				<td align="right">
{if $tool_restart_info.restart_sequence_step == 4}
					<input type="submit" name="restart_end" value="Done">
{/if}
				</td>
			</tr>
			</form>


			</table></td>
		</tr>
		</table>

		{* this is just a dummy form to match the global services end form tag *}
		<form action="index.php" method="post" name="restartdummy">
