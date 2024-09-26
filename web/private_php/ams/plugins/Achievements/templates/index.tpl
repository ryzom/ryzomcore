{block name=content}
{if isset($smarty.get.plugin_action) and $smarty.get.plugin_action eq 'get_achievements'}
<div class="row-fluid">	
	<div class="box col-md-12">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title>
				<span class="icon-user"></span> Achievements
			</div>
			<div class="panel-body">
			{if isset($hook_info.Achievements.no_char)}<div class="alert alert-error"><p>{$hook_info.Achievements.no_char}</p></div>{/if}	
				<div class="row-fluid">
				{$hook_info.Achievements.char_achievements}							
				</div>
			</div><!--/span-->
		</div><!--/span-->
	</div><!--/row-->
</div><!--/row-->
{else}
<div class="row-fluid">	
			<div class="box col-md-4">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="icon-th"></span> Select your Character
					</div>
					<div class="panel-body">
						<div class="row-fluid">
							<form id="generateKey" class="form-vertical" method="post" action="index.php?page=layout_plugin&&name={$arrkey}&&plugin_action=get_achievements">
							<div class="control-group">
								<div class="control-group">
									<label class="control-label">Character:</label>
									<div class="controls">
									<select name="Character">	
									{foreach from=$hook_info.Achievements.Character item=element}
									<option value="{$element}">{$element}</option>
									{/foreach}
									</select>	
									</div>
								</div>
								<div class="control-group">
									<label class="control-label"></label>
									<div class="controls">
										<button type="submit" name="get_data" value="true" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Get Achievements</button>
									</div>
								</div>
							</form>		
							</div>                   
						</div>
					</div><!--/span-->
				</div><!--/span-->
			</div><!--/span-->
</div><!--/row-->
{/if}			
{/block}
