
{include file="page_header.tpl"}

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

<h1>Admin Tool v2.0</h1>
Author: Anthony 'YoGiN' Powles &lt;<a href="mailto:yogin@nevrax.com?subject=Nevrax AdminTool Feedback">yogin@nevrax.com</a>&gt;
<br>Copyright &#169; Nevrax 2000 - 2006
<br><br><br>

{if $ie_check}
<br>
This part of the tool uses some features that Internet Explorer does NOT support.<br>
Therefore, if you need to access them, i would recommend using <a href="http://www.mozilla.com/firefox/">Mozilla Firefox!</a>
{/if}

<br><br>

{include file="page_footer.tpl"}
