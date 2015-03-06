{include file="page_header.tpl"}

{literal}
<script language="Javascript" type="text/javascript">
<!--
	function CheckAll()
	{
		for (var i=0; i<document.qlist.elements.length; i++)
		{
			var e = document.qlist.elements[i];
			if (e.type == 'checkbox' && e.name != 'allbox')
				e.checked = document.qlist.allbox.checked;
		}
	}

	function CheckToggle(checkname)
	{
		checkname.checked = !checkname.checked;
	}

	var total_secs;

	function TimerDown(secs)
	{
		total_secs = secs;
		CountDown();
	}

	function TimerDisplay(secs)
	{
		timer_min = Math.floor(secs / 60);
		timer_sec = secs % 60;

		if (timer_min < 10) timer_min = '0'+ timer_min;
		if (timer_sec < 10) timer_sec = '0'+ timer_sec;

		return timer_min+':'+timer_sec;
	}

	function CountDown()
	{
		total_secs--;
		if (total_secs >= 0)
		{
			document.fcounter.counter.value = TimerDisplay(total_secs);
			down=setTimeout("CountDown()",1000);
		}
	}

	function toggleBox(mybox1, mybox2)
	{
		if (document.all)
		{
			if (document.all.item(mybox1).style.display == "none")
			{
				document.all.item(mybox1).style.display = "";
				document.all.item(mybox2).style.display = "none";
			}
			else
			{
				document.all.item(mybox2).style.display = "";
				document.all.item(mybox1).style.display = "none";
			}
		}
		else
		{
			if (document.getElementById(mybox1).style.display == "none")
			{
				document.getElementById(mybox1).style.display = "";
				document.getElementById(mybox2).style.display = "none";
			}
			else
			{
				document.getElementById(mybox2).style.display = "";
				document.getElementById(mybox1).style.display = "none";
			}
		}
	}

	function openWindow(w_uri, w_title)
	{
		window.open(w_uri, w_title, 'width=800,height=600,directories=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,toolbar=no');
	}

//-->
</script>
{/literal}

<br>

<table width="100%" border="0" cellpadding="0" cellspacing="10">
<tr>
	<td align="left" valign="top" width="150px">

{if $tool_domain_selected && $tool_shard_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Refresh</th>
		</tr>
		<form action="index.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}" method="post" name="fcounter">
		<tr>
			<td>
				<select name="services_refresh" style="width:100%;" onchange="this.form.submit();">
{section name=refresh loop=$tool_refresh_list}
					<option value="{$tool_refresh_list[refresh].secs}" {if $tool_refresh_rate == $tool_refresh_list[refresh].secs}selected{/if}>{$tool_refresh_list[refresh].desc}</option>
{/section}
				</select>
			</td>
		</tr>
{if $tool_refresh_rate > 0}
		<tr>
			<td>
				<input type="text" name="counter" value="" readonly class="refresh_counter">
				<script language="Javascript" type="text/javascript">
					<!--
					TimerDown({$tool_refresh_rate});
					-->
				</script>
			</td>
		</tr>
{/if}
		</form>
		</table>
		<br>
{/if}

		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Domains</th>
		</tr>
{section name=domain loop=$tool_domain_list}
		<tr>
			<td align="center" class="{if $tool_domain_selected == $tool_domain_list[domain].domain_id}domainlistselected{else}domainlist{/if}"><a href="index.php?domain={$tool_domain_list[domain].domain_id}">{$tool_domain_list[domain].domain_name}</a></td>
		</tr>
{/section}
		</table>

{if $tool_domain_selected}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Shards</th>
		</tr>
{section name=shard loop=$tool_shard_list}
{if $tool_domain_selected == $tool_shard_list[shard].shard_domain_id}
		<tr>
			<td align="center" class="{if $tool_shard_selected == $tool_shard_list[shard].shard_id}shardlistselected{else}shardlist{/if}"><a href="index.php?domain={$tool_domain_selected}&shard={$tool_shard_list[shard].shard_id}">{$tool_shard_list[shard].shard_name}</a></td>
		</tr>
{/if}
{/section}
		</table>
{/if}

{if $restriction_tool_notes && $tool_note_list}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Notes</th>
		</tr>
{section name=note loop=$tool_note_list}
		<tr>
{if $tool_note_list[note].note_mode == 0}
			<td align="center"><a href="javascript:void(0);" onclick="return overlib('{$tool_note_list[note].note_data}', WIDTH, 250, STICKY, DRAGGABLE, CAPTION, '{$tool_note_list[note].note_title2}', CENTER, CLOSECLICK, ANCHOR, 'ol_anchor_right', ANCHORALIGN, 'LL', 'UR');" onmouseout="nd();">{$tool_note_list[note].note_title}</a></td>
{elseif $tool_note_list[note].note_mode == 1}
			<td align="center"><a href="javascript:openWindow('{$tool_note_list[note].note_popup_uri}','{$tool_note_list[note].note_title}');">{$tool_note_list[note].note_title}</a></td>
{/if}
		</tr>
{/section}
		</table>
{/if}

{if $tool_hd_list}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">HardDrives</th>
		</tr>
{section name=hd loop=$tool_hd_list}
{if $tool_hd_list[hd].hd_percent >= 85}{assign var="hdtrclass" value="row_red"}
{elseif $tool_hd_list[hd].hd_percent >= 75}{assign var="hdtrclass" value="row_orange_light"}
{else}{assign var="hdtrclass" value="row0"}{/if}
		<tr class="{$hdtrclass}">
			<td align="left" ><a href="javascript:void(0);" onmouseover="return overlib('{$tool_hd_list[hd].summary}', OFFSETX, 40, OFFSETY, 10);" onmouseout="return nd();">{$tool_hd_list[hd].hd_server}</a></td>
			<td align="right">{$tool_hd_list[hd].hd_percent}%</td>
		</tr>
{/section}
		<tr>
			<th colspan="10"><small>{$tool_hd_time|date_format:"%Y/%m/%d %H:%M:%S"}</small></th>
		</tr>
		</table>
{/if}

	</td>

	<td width="10px">&nbsp;</td>

	<td align="right" valign="top">
{if tool_domain_selected && $tool_shard_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<form action="index.php" method="post">
{if $tool_annotation_info || $tool_has_lock}
		<tr>
			<th width="10%">Annotation</th>
			<td><input type="text" name="annotation" value="{$tool_annotation_info.annotation_data}" maxlength="255" size="80" {if !$tool_has_lock}readonly{/if}> {if $tool_has_lock}<input type="submit" name="lock" value="update annotation">{/if}
{if $tool_annotation_info}
			({$tool_annotation_info.annotation_user_name} @ {$tool_annotation_info.annotation_date|date_format:"%Y/%m/%d %H:%M:%S"})
{/if}
			</td>
		</tr>
{/if}
		<tr>
			<th width="10%">Lock</th>
			<td>
{if $tool_no_lock}
{* if (!$tool_lock_info || $tool_lock_info.lock_shard_id) && !$tool_cant_lock && ($tool_shard_restart_status == 0) && !$tool_no_domain_lock *}
{if (!$tool_lock_info || $tool_lock_info.lock_shard_id) && !$tool_cant_lock && !$tool_no_domain_lock}
				{if $restriction_tool_main_lock_shard}<input type="submit" name="lock" value="lock shard">{/if}
{else}
				Lock unavailable, a restart sequence is active !
{/if}
{if ($tool_shard_restart_status == 0) && ($tool_domain_has_shard_restart == 0)}
				{if $restriction_tool_main_lock_domain}<input type="submit" name="lock" value="lock domain">{/if}
{/if}
{elseif $tool_has_shard_lock}
{if $tool_shard_restart_status == 0}
				<input type="submit" name="lock" value="unlock shard">
{/if}
{if ($tool_shard_restart_status == 0) && ($tool_domain_has_shard_restart == 0)}
				{if $restriction_tool_main_lock_domain}<input type="submit" name="lock" value="lock domain">{/if}
{elseif $tool_shard_restart_status > 0}
				Restart Sequence is active !
{/if}

{if $restriction_tool_main_easy_restart && ($tool_shard_restart_status == 0)}
				<input type="submit" name="lock" value="restart sequence" class="restart" onclick="if (confirm('Are you sure you want to engage the RESTART SEQUENCE for this shard ?')) return true; return false;">
				<input type="hidden" name="restart_ws_state" value="{$tool_restart_ws_state}">
{/if}

{elseif $tool_has_domain_lock}
				<input type="submit" name="lock" value="unlock domain">
{/if}
{if $tool_lock_info}
{if $tool_lock_info.lock_domain_id} Domain{elseif $tool_lock_info.lock_shard_id} Shard{/if}
				Locked by <b>{$tool_lock_info.lock_user_name}</b> @ {$tool_lock_info.lock_date|date_format:"%Y/%m/%d %H:%M:%S"}
{else}
				Unlocked.
{/if}
			</td>
		</tr>

		</form>
		</table>
		<br>
{/if}

{if !$tool_domain_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0">You need to select a domain.</td>
		</tr>
		</table>
{elseif !$tool_shard_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0">You need to select a shard.</td>
		</tr>
		</table>
{elseif $tool_domain_error}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">{$tool_domain_error}</span></td>
		</tr>
		</table>
{else}
{if $tool_as_error}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">{$tool_as_error}</span></td>
		</tr>
		</table>
		<br>
{/if}
		<form action="index.php" method="post" name="qlist">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="heads"><input class="check" type="checkbox" name="allbox" value="1" onclick="CheckAll();"></td>
			<td class="heads">AliasName</td>
{if !$iPhone}
			<td class="heads">Shard</td>
{*<td class="heads">LongName</td>*}
			<td class="heads">ShortName</td>
{*<td class="heads">ServiceAlias</td>*}
			<td class="heads">Hostname</td>
			<td class="heads">Running State</td>
			<td class="heads">Running Orders</td>
			<td class="heads">Running Tags</td>
{/if}
			<td class="heads">State</td>
			<td class="heads">Rep</td>
			<td class="heads">Start Cntrs</td>
{if !$iPhone}
			<td class="heads">User SL</td>
{/if}
			<td class="heads">Tick SL</td>
			<td class="heads">Mem</td>
			<td class="heads">Nb Play</td>
			<td class="heads">UpTime</td>
		</tr>
{section name=service loop=$tool_services_list}
{assign var="service_shard_id" value=$tool_services_list[service].ShardName}
{if $tool_shard_filters.$service_shard_id || $tool_shard_filters._all_ || ($tool_shard_filters._unknown_ && !$tool_services_list[service].ShardName)}
{cycle assign="trclass" values="row0,row1"}
{assign var="tdclass1" value=""}
{assign var="tdclass2" value=""}
{if $tool_services_list[service]._flags_.rs_stopped}{assign var="tdclass1" value="class=\"cell_inactive1\""}{assign var="tdclass2" value="class=\"cell_inactive2\""}{assign var="trclass" value="row_stopped"}{/if}
{if $tool_services_list[service]._flags_.rs_starting}{* assign var="tdclass1" value="class=\"cell_inactive1\"" *}{assign var="tdclass2" value="class=\"cell_inactive2\""}{assign var="trclass" value="row_starting"}{/if}
{if $tool_services_list[service]._flags_.alert_red && ($tool_services_list[service]._flags_.rs_starting || $tool_services_list[service]._flags_.rs_online)}{assign var="trclass" value="row_red"}
{elseif $tool_services_list[service]._flags_.alert_orange_dark && ($tool_services_list[service]._flags_.rs_starting || $tool_services_list[service]._flags_.rs_online)}{assign var="trclass" value="row_orange_dark"}
{elseif $tool_services_list[service]._flags_.alert_orange_light && ($tool_services_list[service]._flags_.rs_starting || $tool_services_list[service]._flags_.rs_online)}{assign var="trclass" value="row_orange_light"}{/if}
{assign var="check_name" value=$tool_services_list[service].AliasName}
		<tr class="{$trclass}">
			<td><input class="check" type="checkbox" name="service_{$tool_services_list[service].AliasName}" value="{$tool_services_list[service].AliasName}" {if $tool_service_select_list.$check_name}checked{/if}></td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].AliasName}</td>
{if !$iPhone}
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{if $tool_services_list[service].ShardName != ""}{$tool_services_list[service].ShardName}{else}?{/if}{if $tool_services_list[service].ShardId != ""}/{$tool_services_list[service].ShardId}{/if}</td>
{*<td>{$tool_services_list[service].LongName}</td>*}
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].ShortName}</td>
{*<td>{$tool_services_list[service].ServiceAlias}</td>*}
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].Hostname}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningState}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningOrders}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningTags}</td>
{/if}
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].State}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].NoReportSince}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].StartCounter}</td>
{if !$iPhone}
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].UserSpeedLoop}</td>
{/if}
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].TickSpeedLoop}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].ProcessUsedMemory}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].NbPlayers}</td>
			<td nowrap onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].UpTime}</td>
		</tr>
{/if}
{/section}
		</table>

		<!-- ugly trick to block the first submit button being triggered when hitting ENTER to send the form -->
		<div style="display: none;"><input type="submit" name="fake" value="fake" onclick="alert('PLEASE DO NOT USE THE &lt;ENTER&gt; KEY !'); return false;"></div>
		<!-- end ugly trick :) -->

{if $restriction_tool_main_easy_restart && ($tool_shard_restart_status > 0)}

{include file="index_restart_sequence.tpl"}

{else}

{* if $restriction_tool_main_ws *}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>WS : </b>{if $restriction_tool_main_ws_old && $restriction_tool_main_ws}<br><small><a href="javascript:toggleBox('ws_old','ws_new');">new/old</a></small>{/if}</td>
			<td>
{if $restriction_tool_main_ws_old && $restriction_tool_main_ws}
<div id="ws_old" style="display: none;">
				&nbsp;
				<input type="submit" name="services_update" value="open ws"	 onclick="if (confirm('Are you sure you want to OPEN the selected WS services ?')) return true; return false;">&nbsp;
				<input type="submit" name="services_update" value="lock ws"	 onclick="if (confirm('Are you sure you want to LOCK the selected WS services ?')) return true; return false;">&nbsp;
				<input type="submit" name="services_update" value="close ws" onclick="if (confirm('Are you sure you want to CLOSE the selected WS services ?')) return true; return false;">&nbsp;
</div>
{/if}
<div id="ws_new">
				<table width="100%" border="0" cellpadding="1">
{if $restriction_tool_main_ws}
					<input type="hidden" name="ws_su" value="{$tool_shard_su_name}">
					<input type="hidden" name="ws_shard_name" value="">
					<input type="hidden" name="ws_shard_id" value="">
{/if}
{section name=shard loop=$tool_shard_run_list}
{assign var="sname" value=$tool_shard_run_list[shard]}
{if $tool_shard_infos[$sname] && $tool_shard_su_name}
					<tr>
						<td width="10%">&nbsp;<b>{$tool_shard_run_list[shard]}</b></td>
						<td width="10%">
							<select name="ws_state_{$sname}" {if !$restriction_tool_main_ws}disabled{/if}>
{section name=state loop=$tool_shard_ws_states}
								<option class="ws_{$tool_shard_ws_states[state]}" value="{$tool_shard_ws_states[state]}" {if $tool_shard_infos[$sname].state == $tool_shard_ws_states[state]}selected{/if}>{$tool_shard_ws_states[state]}</option>
{/section}
							</select></td>
						<td width="20%"><input type="text" name="ws_motd_{$sname}" value="{$tool_shard_infos[$sname].motd}" maxlength="255" size="40" {if !$restriction_tool_main_ws}disabled{/if}></td>
						<td width="20%">
{if $restriction_tool_main_ws}
							<input type="submit" name="ws_update" value="update WS" onclick="if (confirm('Are you sure you want to change the WS State for shard &lt;{$sname}&gt; ?')) {ldelim} this.form.ws_shard_name.value='{$sname}'; this.form.ws_shard_id.value='{$tool_shard_infos[$sname].shard_id}'; return true; {rdelim} else {ldelim} return false; {rdelim}">
{/if}
						&nbsp;</td>
						<td>&nbsp;</td>
					</tr>
{/if}
{/section}
				</table>
</div>
			</td>
		</tr>
		</table>
{* /if *}

{if $restriction_tool_main_start || $restriction_tool_main_stop || $restriction_tool_main_restart || $restriction_tool_main_kill || $restriction_tool_main_abort || $restriction_tool_main_reset_counters || $restriction_tool_main_service_autostart}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Services : </b></td>
			<td>&nbsp;
{if $restriction_tool_main_start}
				<input type="submit" name="services_update" value="start"	onclick="if (confirm('Are you sure you want to START the selected services ?')) return true; return false;">&nbsp;
{/if}
{if $restriction_tool_main_stop}
				<input type="submit" name="services_update" value="stop"	onclick="if (confirm('Are you sure you want to STOP the selected services ?')) return true; return false;">&nbsp;
{/if}
{if $restriction_tool_main_restart}
				<input type="submit" name="services_update" value="restart"	onclick="if (confirm('Are you sure you want to RESTART the selected services ?')) return true; return false;">&nbsp;
{/if}
{if $restriction_tool_main_kill}
				<input type="submit" name="services_update" value="kill"	onclick="if (confirm('Are you sure you want to KILL the selected services ?')) return true; return false;">&nbsp;
{/if}
{if $restriction_tool_main_abort}
				<input type="submit" name="services_update" value="abort"	onclick="if (confirm('Are you sure you want to ABORT the selected services ?')) return true; return false;">&nbsp;
{/if}
{if $restriction_tool_main_service_autostart}
				<input type="submit" name="services_update" value="activate"	onclick="if (confirm('Are you sure you want to ACTIVATE the selected services ?')) return true; return false;">&nbsp;
				<input type="submit" name="services_update" value="deactivate"	onclick="if (confirm('Are you sure you want to DEACTIVATE the selected services ?')) return true; return false;">&nbsp;
{/if}
{if $restriction_tool_main_reset_counters}
				<input type="submit" name="services_update" value="reset counters"	onclick="if (confirm('Are you sure you want to RESET START COUNTERS on the selected services (AES only) ?')) return true; return false;">&nbsp;
{/if}
			</td>
		</tr>
		</table>
{/if}

{if $restriction_tool_main_shard_autostart && $tool_shard_run_list}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Shards : </b></td>
			<td><table width="100%" border="0" cellpadding="1">
				<input type="hidden" name="shards_update_name" value="">
{section name=shard loop=$tool_shard_run_list}
{assign var="sname" value=$tool_shard_run_list[shard]}
				<tr>
					<td width="10%">&nbsp;<b>{$tool_shard_run_list[shard]}</b></td>
					<td width="10%">{if $sname != ""}<span class="{$tool_shard_orders[$sname]}">{$tool_shard_orders[$sname]|replace:'_':'&nbsp;'}</span>{/if}&nbsp;</td>
					<td width="80%">
						<input type="submit" name="shards_update" value="auto restart on"  onclick="if (confirm('Are you sure you want to set AUTO RESTART ON for shard &lt;{$tool_shard_run_list[shard]}&gt; ?')) {ldelim} this.form.shards_update_name.value='{$tool_shard_run_list[shard]}'; return true; {rdelim} else {ldelim} return false; {rdelim}">&nbsp;
						<input type="submit" name="shards_update" value="auto restart off" onclick="if (confirm('Are you sure you want to set AUTO RESTART OFF for shard &lt;{$tool_shard_run_list[shard]}&gt; ?')) {ldelim} this.form.shards_update_name.value='{$tool_shard_run_list[shard]}'; return true; {rdelim} else {ldelim} return false; {rdelim}">&nbsp;
					</td>
				</tr>
{/section}
			</table></td>
		</tr>
		</table>
{/if}

{if $restriction_tool_main_execute}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Command : </b></td>
			<td>&nbsp;<input type="text" name="service_command" value="{$tool_execute_command}" size="50">&nbsp;
				<input type="submit" name="services_update" value="execute"	{* onclick="if (confirm('Are you sure you want to EXECUTE this command on the selected services ?')) return true; return false;" *}>&nbsp;
			</td>
		</tr>
		</table>

{if $tool_execute_command}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th>Command Results for '{$tool_execute_command}' :</th>
		</tr>
		<tr>
			<td><textarea width="100%" rows="50" class="command" readonly >{section name=exe loop=$tool_execute_result}{$tool_execute_result[exe]}{/section}</textarea></td>
		</tr>
		</table>
{/if}
{/if}

{* end of: if $restriction_tool_main_easy_restart && ($tool_shard_restart_status > 0) *}
{/if}
		</form>
{/if}

	</td>
</tr>
</table>


{include file="page_footer.tpl"}
