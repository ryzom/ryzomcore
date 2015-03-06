{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-12">
	<div class="panel panel-default">
        <div class="panel-heading" data-original-title="">
            <span class="icon-th"></span> Create a new Ticket
        </div>
        <div class="panel-body">
            <div class="row-fluid">
                <form id="changePassword" class="form-vertical" method="post" action="index.php?page=createticket&id={$target_id}">
                    <legend>New ticket</legend>

                    <div class="control-group {if isset($TITLE_ERROR) and $TITLE_ERROR eq "TRUE"}error{/if}">
                        <label class="control-label">Title</label>
                        <div class="controls">
                            <div class="input-prepend">
                                <input type="text" class="span8" id="Title" name="Title" {if isset($Title)}value='{$Title}'{/if}>
								{if isset($TITLE_ERROR) and $TITLE_ERROR eq "TRUE"}<span class="help-inline">{$TITLE_ERROR_MESSAGE}</span>{/if}
                            </div>
                        </div>
                    </div>

                    <div class="control-group">
                        <label class="control-label">Category</label>
                        <div class="controls">
                            <select name="Category">
                                {foreach from=$category key=k item=v}
                                        <option value="{$k}">{$v}</option>
                                {/foreach}
                            </select>
                        </div>
                    </div>

                    <div class="control-group {if isset($CONTENT_ERROR) and $CONTENT_ERROR eq "TRUE"}error{/if}">
                        <label class="control-label">Description</label>
                        <div class="controls">
                            <div class="input-prepend">
							<textarea rows="12" class="span12" id="Content" name="Content">{if isset($Content)}{$Content}{/if}</textarea>
                            {if isset($CONTENT_ERROR) and $CONTENT_ERROR eq "TRUE"}<span class="help-inline">{$CONTENT_ERROR_MESSAGE}</span>{/if}
							</div>
                        </div>
                    </div>

                    <input type="hidden" name="function" value="create_ticket">
                    <input type="hidden" name="target_id" value="{$target_id}">
                    <div class="control-group">
                        <label class="control-label"></label>
                        <div class="controls">
                            <button type="submit" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Send Ticket</button>
                        </div>
                    </div>
                </form>
            </div>
        </div>
		</div>
    </div><!--/span-->
</div><!--/row-->
{/block}

