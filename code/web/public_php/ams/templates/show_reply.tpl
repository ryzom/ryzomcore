{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-12">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-user"></span> Show Reply
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <legend>Reply ID#{$reply_id} of Ticket <a href="index.php?page=show_ticket&id={$ticket_id}">#{$ticket_id}</a></legend>
		<table class="table table-bordered" >
		    <tr>
			<td>
			    <p><span class="label label-info"> {$reply_timestamp}</span>
			    {if $author_permission eq '1'}
			    <span class="label label-success"><strong><span class="icon-user icon-white"></span>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="white"> {$authorName}</font>{else} {$authorName} {/if}</a></strong></span></p>
			    {else if $author_permission gt '1'}
			    <span class="label label-warning"><strong><span class="icon-star icon-white"></span>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="white"> {$authorName}</font>{else} {$authorName} {/if}</a></strong></span></p>
			    {/if}
			    <p><pre{if $author_permission gt '1'} {if $hidden eq 0} style="background-color:rgb(248, 200, 200);"{else if $hidden eq 1}style="background-color:rgb(207, 254, 255);"{/if}{/if}> {if $hidden eq 1}<i>{/if}{$reply_content}{if $hidden eq 1}</i>{/if}</pre></p>

			</td>
		    </tr>
		</table>
	    </div>
        </div>
		</div>
    </div><!--/span-->
</div><!--/row-->
{/block}

