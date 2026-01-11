
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
		<form action="tool_guild_locator.php?domain={$tool_domain_selected}&shard={$tool_shard_selected}" method="post" name="fcounter">
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
			<td align="center" class="{if $tool_domain_selected == $tool_domain_list[domain].domain_id}domainlistselected{else}domainlist{/if}"><a href="tool_guild_locator.php?domain={$tool_domain_list[domain].domain_id}">{$tool_domain_list[domain].domain_name}</a></td>
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
			<td align="center" class="{if $tool_shard_selected == $tool_shard_list[shard].shard_id}shardlistselected{else}shardlist{/if}"><a href="tool_guild_locator.php?domain={$tool_domain_selected}&shard={$tool_shard_list[shard].shard_id}">{$tool_shard_list[shard].shard_name}</a></td>
		</tr>
{/if}
{/section}
		</table>
{/if}

	</td>

	<td width="10px">&nbsp;</td>

	<td align="right" valign="top">

{if !$tool_domain_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0">You need to select a domain.</td>
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


		<form action="tool_guild_locator.php" method="post" name="qlist">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="heads"><input class="check" type="checkbox" name="allbox" value="1" onclick="CheckAll();"></td>
			<td class="heads">AliasName</td>
			<td class="heads">Shard</td>
			<td class="heads">ShortName</td>
			<td class="heads">Hostname</td>
			<td class="heads">Running State</td>
			<td class="heads">Running Tags</td>
			<td class="heads">State</td>
			<td class="heads">Report</td>
			<td class="heads">Counters</td>
			<td class="heads">User SL</td>
			<td class="heads">Tick SL</td>
			<td class="heads">Memory</td>
			<td class="heads">NbPlayers</td>
			<td class="heads">UpTime</td>
		</tr>
{assign var="egs_counter" value="0"}
{section name=service loop=$tool_services_list}
{assign var="service_shard_id" value=$tool_services_list[service].ShardName}
{if $tool_shard_filters.$service_shard_id || $tool_shard_filters._all_ || ($tool_shard_filters._unknown_ && !$tool_services_list[service].ShardName)}
{cycle assign="trclass" values="row0,row1"}
{assign var="tdclass1" value=""}
{assign var="tdclass2" value=""}
{if $tool_services_list[service]._flags_.rs_stopped}{assign var="tdclass1" value="class=\"cell_inactive1\""}{assign var="tdclass2" value="class=\"cell_inactive2\""}{assign var="trclass" value="row_stopped"}{/if}
{if $tool_services_list[service]._flags_.rs_starting}{assign var="tdclass1" value="class=\"cell_inactive1\""}{assign var="tdclass2" value="class=\"cell_inactive2\""}{assign var="trclass" value="row_starting"}{/if}
{if $tool_services_list[service]._flags_.alert_red}{assign var="trclass" value="row_red"}{/if}
{if $tool_services_list[service]._flags_.alert_orange_dark}{assign var="trclass" value="row_orange_dark"}{/if}
{if $tool_services_list[service]._flags_.alert_orange_light}{assign var="trclass" value="row_orange_light"}{/if}
{assign var="check_name" value=$tool_services_list[service].AliasName}
{assign var="egs_counter" value="`$egs_counter+1`"}
<!-- {$egs_counter} -->
		<tr class="{$trclass}">
			<td><input class="check" type="checkbox" name="service_{$tool_services_list[service].AliasName}" value="{$tool_services_list[service].AliasName}" {if $tool_service_select_list.$check_name || (!$tool_service_select_list && $tool_services_list[service]._flags_.rs_online)}checked{/if}></td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].AliasName}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{if $tool_services_list[service].ShardName != ""}{$tool_services_list[service].ShardName}{else}?{/if}{if $tool_services_list[service].ShardId != ""}/{$tool_services_list[service].ShardId}{/if}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].ShortName}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].Hostname}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningState}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass1}>{$tool_services_list[service].RunningTags}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].State}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].NoReportSince}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].StartCounter}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].UserSpeedLoop}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].TickSpeedLoop}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].ProcessUsedMemory}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].NbPlayers}</td>
			<td onclick="CheckToggle(document.qlist.service_{$tool_services_list[service].AliasName})" {$tdclass2}>{$tool_services_list[service].UpTime}</td>
		</tr>
{/if}
{/section}
		</table>

		<!-- ugly trick to block the first submit button being triggered when hitting ENTER to send the form -->
		<div style="display: none;"><input type="submit" name="fake" value="fake" onclick="alert('PLEASE DO NOT USE THE &lt;ENTER&gt; KEY !'); return false;"></div>
		<!-- end ugly trick :) -->

{if $egs_counter > 0}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="100px"><b>Commands : </b></td>
			<td>&nbsp;
				<input type="submit" name="services_gl" value="display guilds"	onclick="if (confirm('Are you sure you want to DISPLAY GUILDS from the selected services ?')) return true; return false;">&nbsp;
			</td>
		</tr>
		</table>

{if $tool_guild_data}
{assign var="service_name" value=""}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
{section name=guild loop=$tool_guild_data}
{if $service_name != $tool_guild_data[guild].service}
{assign var="service_name" value=$tool_guild_data[guild].service}
		<tr>
			<th colspan="8">{$service_name}</th>
		</tr>
		<tr>
			<th>Guild Name</th>
			<th>Guild ID</th>
			<th>Guild Members</th>
			<th>Guild EID</th>
		</tr>
{/if}
{cycle assign="trclass" values="row0,row1"}
		<tr class="{$trclass}">
			<td>{$tool_guild_data[guild].name}</td>
			<td align="center"><a href="tool_guild_locator.php?services_gl=dumpguild&servicealias={$service_name|lower}&guildshardid={$tool_guild_data[guild].shardid}&guildid={$tool_guild_data[guild].guildid}">{$tool_guild_data[guild].shardid} - {$tool_guild_data[guild].guildid}</a></td>
			<td align="center">{$tool_guild_data[guild].members}</td>
			<td align="center">{$tool_guild_data[guild].guildeid}</td>
		</tr>
{/section}
		</table>
{/if}

{if $tool_guild_dump_data}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th>Guild Name</th>
			<th>ID (EID)</th>
			<th>Money</th>
			<th>Race</th>
			<th>Members</th>
		</tr>
		<tr>
			<td>{$tool_guild_dump_data.guild_name}</td>
			<td align="center"><a href="tool_guild_locator.php?services_gl=dumpguild&servicealias={$tool_service|lower}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}">{$tool_guild_dump_data.shard_id}:{$tool_guild_dump_data.guild_id} {$tool_guild_dump_data.guild_eid}</a></td>
			<td align="center">{$tool_guild_dump_data.guild_money}</td>
			<td align="center">{$tool_guild_dump_data.guild_race}</td>
			<td align="center">{$tool_guild_dump_data.members_count}</td>
		</tr>
		<tr>
			<td colspan="8">Description: {$tool_guild_dump_data.guild_description}</td>
		</tr>
		</table>
		<br>

{if $restriction_tool_guild_locator_manage_guild}
		<input type="hidden" name="servicealias" value="{$tool_service|lower}">
		<input type="hidden" name="guildshardid" value="{$tool_guild_dump_data.shard_id}">
		<input type="hidden" name="guildid" 	 value="{$tool_guild_dump_data.guild_id}">

		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td align="right" width="15%">Rename&nbsp;Guild&nbsp;</td>
			<td><input type="text" name="new_guild_name" value="{$tool_guild_dump_data.guild_name}" size="50">&nbsp;
				<input type="submit" name="services_gl" value="update name"  style="width:150px;" onclick="if (confirm('Are you sure you want to update this guilds name ?')) return true; else return false;">
				</td>
		</tr>
		<tr>
			<td align="right" width="15%">Change&nbsp;Description&nbsp;</td>
			<td><input type="text" name="new_guild_description" value="{$tool_guild_dump_data.guild_description}" size="50">&nbsp;
				<input type="submit" name="services_gl" value="update description" style="width:150px;" onclick="if (confirm('Are you sure you want to update this guilds description ?')) return true; else return false;">
				</td>
		</tr>
		</table>
		<br>
{/if}

{if $tool_guild_errors}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
{section name=err loop=$tool_guild_errors}
		<tr>
			<td>{$tool_guild_errors[err]}</td>
		</tr>
{/section}
		</table>
		<br>
{/if}

		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="8">Leader</th>
		</tr>
		<tr>
			<th>Name</th>
			<th>Shard</th>
			<th>EntityID</th>
			<th>Index</th>
			<th>Enter Time</th>
{if $restriction_tool_guild_locator_manage_members}
			<th>&nbsp;</th>
{/if}
		</tr>
{section name=leader loop=$tool_guild_dump_data.Leader}
		<tr class="{cycle values="row0,row1"}">
			<td>{$tool_guild_dump_data.Leader[leader].name}</td>
			<td align="center">{$tool_guild_dump_data.Leader[leader].shard}</td>
			<td align="center">{$tool_guild_dump_data.Leader[leader].eid}</td>
			<td align="center">{$tool_guild_dump_data.Leader[leader].index}</td>
			<td align="center">{$tool_guild_dump_data.Leader[leader].entertime}</td>
{if $restriction_tool_guild_locator_manage_members}
			<td align="right">&nbsp;</td>
{/if}
		</tr>
{sectionelse}
		<tr><td colspan="8" align="center"><i>none</i></td></tr>
{/section}

		<tr>
			<th colspan="8">HighOfficers</th>
		</tr>
		<tr>
			<th>Name</th>
			<th>Shard</th>
			<th>EntityID</th>
			<th>Index</th>
			<th>Enter Time</th>
{if $restriction_tool_guild_locator_manage_members}
			<th>&nbsp;</th>
{/if}
		</tr>
{section name=hofficer loop=$tool_guild_dump_data.HighOfficer}
		<tr class="{cycle values="row0,row1"}">
			<td>{$tool_guild_dump_data.HighOfficer[hofficer].name}</td>
			<td align="center">{$tool_guild_dump_data.HighOfficer[hofficer].shard}</td>
			<td align="center">{$tool_guild_dump_data.HighOfficer[hofficer].eid}</td>
			<td align="center">{$tool_guild_dump_data.HighOfficer[hofficer].index}</td>
			<td align="center">{$tool_guild_dump_data.HighOfficer[hofficer].entertime}</td>
{if $restriction_tool_guild_locator_manage_members}
			<td align="right">[&nbsp;Set&nbsp;:&nbsp;
				<a href="tool_guild_locator.php?services_gl=setleader&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.HighOfficer[hofficer].eid}" onclick="if (confirm('Are you sure you want to promote guild High Officer &lt;{$tool_guild_dump_data.HighOfficer[hofficer].name}&gt; as Leader ?')) return true; else return false;">Leader</a>&nbsp;|&nbsp;
				<a href="tool_guild_locator.php?services_gl=demote&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.HighOfficer[hofficer].eid}&grade=Officer" onclick="if (confirm('Are you sure you want to demote guild High Officer &lt;{$tool_guild_dump_data.HighOfficer[hofficer].name}&gt; as Officer ?')) return true; else return false;">Officer</a>&nbsp;|&nbsp;
				<a href="tool_guild_locator.php?services_gl=demote&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.HighOfficer[hofficer].eid}&grade=Member" onclick="if (confirm('Are you sure you want to demote guild High Officer &lt;{$tool_guild_dump_data.HighOfficer[hofficer].name}&gt; as Member ?')) return true; else return false;">Member</a>&nbsp;]&nbsp;
			</td>
{/if}
		</tr>
{sectionelse}
		<tr><td colspan="8" align="center"><i>none</i></td></tr>
{/section}

		<tr>
			<th colspan="8">Officers</th>
		</tr>
		<tr>
			<th>Name</th>
			<th>Shard</th>
			<th>EntityID</th>
			<th>Index</th>
			<th>Enter Time</th>
{if $restriction_tool_guild_locator_manage_members}
			<th>&nbsp;</th>
{/if}
		</tr>
{section name=officer loop=$tool_guild_dump_data.Officer}
		<tr class="{cycle values="row0,row1"}">
			<td>{$tool_guild_dump_data.Officer[officer].name}</td>
			<td align="center">{$tool_guild_dump_data.Officer[officer].shard}</td>
			<td align="center">{$tool_guild_dump_data.Officer[officer].eid}</td>
			<td align="center">{$tool_guild_dump_data.Officer[officer].index}</td>
			<td align="center">{$tool_guild_dump_data.Officer[officer].entertime}</td>
{if $restriction_tool_guild_locator_manage_members}
			<td align="right">[&nbsp;Set&nbsp;:&nbsp;
				<a href="tool_guild_locator.php?services_gl=setleader&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.Officer[officer].eid}" onclick="if (confirm('Are you sure you want to promote guild Officer &lt;{$tool_guild_dump_data.Officer[officer].name}&gt; as Leader ?')) return true; else return false;">Leader</a>&nbsp;|&nbsp;
				<a href="tool_guild_locator.php?services_gl=promote&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.Officer[officer].eid}&grade=HighOfficer" onclick="if (confirm('Are you sure you want to promote guild Member &lt;{$tool_guild_dump_data.Officer[officer].name}&gt; as High Officer ?')) return true; else return false;">HighOfficer</a>&nbsp;|&nbsp;
				<a href="tool_guild_locator.php?services_gl=demote&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.Officer[officer].eid}&grade=Member" onclick="if (confirm('Are you sure you want to demote guild Officer &lt;{$tool_guild_dump_data.Officer[officer].name}&gt; as Member ?')) return true; else return false;">Member</a>&nbsp;]&nbsp;
			</td>
{/if}
		</tr>
{sectionelse}
		<tr><td colspan="8" align="center"><i>none</i></td></tr>
{/section}

		<tr>
			<th colspan="8">Members</th>
		</tr>
		<tr>
			<th>Name</th>
			<th>Shard</th>
			<th>EntityID</th>
			<th>Index</th>
			<th>Enter Time</th>
{if $restriction_tool_guild_locator_manage_members}
			<th>&nbsp;</th>
{/if}
		</tr>
{section name=member loop=$tool_guild_dump_data.Member}
		<tr class="{cycle values="row0,row1"}">
			<td>{$tool_guild_dump_data.Member[member].name}</td>
			<td align="center">{$tool_guild_dump_data.Member[member].shard}</td>
			<td align="center">{$tool_guild_dump_data.Member[member].eid}</td>
			<td align="center">{$tool_guild_dump_data.Member[member].index}</td>
			<td align="center">{$tool_guild_dump_data.Member[member].entertime}</td>
{if $restriction_tool_guild_locator_manage_members}
			<td align="right">[&nbsp;Set&nbsp;:&nbsp;
				<a href="tool_guild_locator.php?services_gl=setleader&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.Member[member].eid}" onclick="if (confirm('Are you sure you want to promote guild Member &lt;{$tool_guild_dump_data.Member[member].name}&gt; as Leader ?')) return true; else return false;">Leader</a>&nbsp;|&nbsp;
				<a href="tool_guild_locator.php?services_gl=promote&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.Member[member].eid}&grade=HighOfficer" onclick="if (confirm('Are you sure you want to promote guild Member &lt;{$tool_guild_dump_data.Member[member].name}&gt; as High Officer ?')) return true; else return false;">HighOfficer</a>&nbsp;|&nbsp;
				<a href="tool_guild_locator.php?services_gl=promote&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&eid={$tool_guild_dump_data.Member[member].eid}&grade=Officer" onclick="if (confirm('Are you sure you want to promote guild Member &lt;{$tool_guild_dump_data.Member[member].name}&gt; as Officer ?')) return true; else return false;">Officer</a>&nbsp;]&nbsp;
			</td>
{/if}
		</tr>
{sectionelse}
		<tr><td colspan="8" align="center"><i>none</i></td></tr>
{/section}
		</table>
		<br>

		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="8">Owned Outposts</th>
		</tr>
		<tr>
			<th>Name</th>
			<th>Alias</th>
			<th>Sheet</th>
		</tr>
{section name=outpost loop=$tool_guild_dump_data.outposts}
		<tr class="{cycle values="row0,row1"}">
			<td align="center">{$tool_guild_dump_data.outposts[outpost].name}</td>
			<td align="center">{$tool_guild_dump_data.outposts[outpost].alias}</td>
			<td align="center">{$tool_guild_dump_data.outposts[outpost].sheet}</td>
		</tr>
{sectionelse}
		<tr><td colspan="8" align="center"><i>none</i></td></tr>
{/section}
		</table>
		<br>

		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="8">Challenged Outposts</th>
		</tr>
		<tr>
			<th>Name</th>
			<th>Alias</th>
			<th>Sheet</th>
		</tr>
{section name=outpost loop=$tool_guild_dump_data.challenged_outposts}
		<tr class="{cycle values="row0,row1"}">
			<td align="center">{$tool_guild_dump_data.challenged_outposts[outpost].name}</td>
			<td align="center">{$tool_guild_dump_data.challenged_outposts[outpost].alias}</td>
			<td align="center">{$tool_guild_dump_data.challenged_outposts[outpost].sheet}</td>
		</tr>
{sectionelse}
		<tr><td colspan="8" align="center"><i>none</i></td></tr>
{/section}
		</table>
		<br>

{if $restriction_tool_guild_locator_manage_forums}
		<a name="forumview">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="8">Guild Forums</th>
		</tr>
{if $tool_guild_forums_error}
		<tr>
			<td class="row0"><span class="alert">{$tool_guild_forums_error}</span></td>
		</tr>
{elseif $tool_guild_forums}
		<tr>
			<th>File</th>
			<th>Thread</th>
			<th>Action</th>
		</tr>
{section name=line loop=$tool_guild_forums}
		<tr class="{cycle values="row0,row1"}">

			<td align="center"><a href="?services_gl=dumpguild&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&subservices_gl=viewthread&threadid={$tool_guild_forums[line].thread}&recoverable={$tool_guild_forums[line].recover}#threadview">{$tool_guild_forums[line].file}</a></td>
			<td align="center">{$tool_guild_forums[line].thread}</td>
			<td align="center">{if $tool_guild_forums[line].recover == 1}<a href="?services_gl=dumpguild&servicealias={$tool_service}&guildshardid={$tool_guild_dump_data.shard_id}&guildid={$tool_guild_dump_data.guild_id}&subservices_gl=recoverthread&threadid={$tool_guild_forums[line].thread}&recoverable={$tool_guild_forums[line].recover}#forumview" onclick="if (confirm('Are you sure you want to RECOVER this thread ?')) return true; return false;">Recover</a>{/if}</td>
		</tr>
{/section}
{/if}
		</table>
		<br>

{if $tool_guild_thread}
		<a name="threadview">
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="8">Thread View</th>
		</tr>
		<tr>
			<td colspan="8">{$tool_guild_thread.topic.raw}</td>
		</tr>
{section name=msg loop=$tool_guild_thread.data}
		<tr class="{cycle values="row0,row1"}">
			<td colspan="8">{$tool_guild_thread.data[msg].raw}</td>
		</tr>
{/section}
		</table>
{/if}

{/if}

{/if}

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

{else}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">No EGS to work with!</span></td>
		</tr>
		</table>
{/if}

		</form>

{/if}

	</td>
</tr>
</table>


{include file="page_footer.tpl"}
