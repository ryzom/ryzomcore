
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

<table width="100%" cellpadding="2" cellspacing="0" border="0">
	<tr>
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

<table width="100%" border="0" cellpadding="0" cellspacing="10">
<tr>
	<td align="left" valign="top" width="150px">

{if $tool_domain_selected && $tool_frame_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Refresh</th>
		</tr>
		<form action="tool_graphs.php?toolmode={$toolmode}&domain={$tool_domain_selected}&shard={$tool_shard_selected}" method="post" name="fcounter">
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
			<td align="center" class="{if $tool_domain_selected == $tool_domain_list[domain].domain_id}domainlistselected{else}domainlist{/if}"><a href="tool_graphs.php?toolmode={$toolmode}&domain={$tool_domain_list[domain].domain_id}">{$tool_domain_list[domain].domain_name}</a></td>
		</tr>
{/section}
		</table>

{if $tool_domain_selected}
		<br>
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<th colspan="10">Time Frame</th>
		</tr>
{section name=frame loop=$tool_frame_list}
		<tr>
			<td align="center" class="{if $tool_frame_selected == $tool_frame_list[frame].value}shardlistselected{else}shardlist{/if}"><a href="tool_graphs.php?toolmode={$toolmode}&domain={$tool_domain_selected}&shard={$tool_shard_selected}&lowframe={$tool_frame_list[frame].value}">{$tool_frame_list[frame].title}</a></td>
		</tr>
{/section}
		</table>

{/if}

	</td>

	<td width="10px">&nbsp;</td>

	<td align="right" valign="top">

{if $tool_domain_error}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0"><span class="alert">{$tool_domain_error}</span></td>
		</tr>
		</table>
{elseif !$tool_domain_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0">You need to select a domain.</td>
		</tr>
		</table>
{elseif !$tool_frame_selected}
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0">You need to select a time frame.</td>
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
		<table width="100%" border="0" cellpadding="1" bgcolor="#cccccc" class="view">
		<tr>
			<td class="row0" width="100%" valign="top">
{if $tool_rrd_output}
{section name=rrd loop=$tool_rrd_output}
				<b>{$tool_rrd_output[rrd].desc}</b><br>
				<img src="{$tool_rrd_output[rrd].img}" border="0"><br>
{/section}
{/if}
			</td>
		</tr>
		</table>
{/if}

	</td>
</tr>
</table>


{include file="page_footer.tpl"}
