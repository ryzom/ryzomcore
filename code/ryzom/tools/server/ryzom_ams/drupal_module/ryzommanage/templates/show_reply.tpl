{block name=content}
            <h1>Show Reply</h1>
            
                <h2>Reply ID#{$reply_id} of Ticket <a href="ams?page=show_ticket&id={$ticket_id}">#{$ticket_id}</a></h2>
		<table>
		    <tr>
			<td>
			    <p><font color="blue"> {$reply_timestamp}</font>
			    {if $author_permission eq '1'}
			    <span class="label label-success"><strong>{if isset($isMod) and $isMod eq "TRUE"} <a href="ams?page=show_user&id={$author}">{$authorName}{else} {$authorName} {/if}</a></strong></span></p>
			    {else if $author_permission gt '1'}
			    <span class="label label-warning"><strong>{if isset($isMod) and $isMod eq "TRUE"} <a href="ams?page=show_user&id={$author}">{$authorName}{else} {$authorName} {/if}</a></strong></span></p>
			    {/if}
			    <p><pre{if $author_permission gt '1'} {if $hidden eq 0} style="background-color:rgb(248, 200, 200);"{else if $hidden eq 1}style="background-color:rgb(207, 254, 255);"{/if}{/if}> {if $hidden eq 1}<i>{/if}{$reply_content}{if $hidden eq 1}</i>{/if}</pre></p>
			 
			</td>
		    </tr>
		</table>
	    </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
