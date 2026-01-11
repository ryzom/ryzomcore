
{if !$iPhone}

<br><br>

<br>

{* DEBUG INFORMATION *}

{if $NELTOOL_DEBUG != null}
<hr size="1">
<pre>DEBUG
{php}
global $nel_debug;
foreach ($nel_debug as $dbgline) echo " • $dbgline\n";
{/php}</pre>
{/if}

{/if}

</body>
</html>
