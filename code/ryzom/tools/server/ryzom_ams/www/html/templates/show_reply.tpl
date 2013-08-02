{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span12">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-user"></i> Show Reply</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Reply ID#{$reply_id} of Ticket <a href="index.php?page=show_ticket&id={$ticket_id}">#{$ticket_id}</a></legend>
		<table class="table table-bordered" >
		    <tr>
			<td>
			    <p><span class="label label-info"> {$reply_timestamp}</span>
			    {if $author_permission eq '1'}
			    <span class="label label-success"><strong><i class="icon-user icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="white"> {$authorName}</font>{else} {$authorName} {/if}</a></strong></span></p>
			    {else if $author_permission gt '1'}
			    <span class="label label-warning"><strong><i class="icon-star icon-white"></i>{if isset($isMod) and $isMod eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="white"> {$authorName}</font>{else} {$authorName} {/if}</a></strong></span></p>
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
	
