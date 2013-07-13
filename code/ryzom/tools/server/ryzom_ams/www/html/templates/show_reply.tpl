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
			    <p><span class="label label-info"> {$reply_timestamp} {$author_permission}</span>
			    {if $author_permission eq '1'}
			    <!-- <span class="label label-important"><strong></i>[User]:</strong></span>-->
			    {else if $author_permission eq '2'}
			    <span class="label label-important"><strong><i class="icon-star icon-white"></i>[CSR]</strong></span>
			    {/if}
			    <span class="label label-warning"><strong><i class="icon-user icon-white"></i>{if isset($isAdmin) and $isAdmin eq "TRUE"} <a href="index.php?page=show_user&id={$author}"><font color="white">{$authorName}</font>{else}{$authorName} {/if}</a></strong></span></p>

			    <p><pre{if $author_permission eq '2'} style="background-color:rgb(248, 200, 200);"{/if}>{$reply_content}</pre></p>
			</td>
		    </tr>
		</table>
	    </div>                   
        </div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
