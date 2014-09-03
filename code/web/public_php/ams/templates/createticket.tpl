{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box col-md-12">
	<div class="box-inner">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-th"></i> Create a new Ticket</h2>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <form id="changePassword" class="form-vertical" method="post" action="index.php?page=createticket&id={$target_id}">
                    <legend>New ticket</legend>

                    <div class="control-group">
                        <label class="control-label">Title</label>
                        <div class="controls">
                            <div class="input-prepend">
                                <input type="text" class="span8" id="Title" name="Title">
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

                    <div class="control-group">
                        <label class="control-label">Description</label>
                        <div class="controls">
                            <div class="input-prepend">
				    <textarea rows="12" class="span12" id="Content" name="Content"></textarea>
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

