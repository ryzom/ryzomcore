{block name=content}
<div class="row-fluid">
{if isset($hook_info)}
{foreach from=$hook_info key=arrkey item=element}
{if $arrkey eq $smarty.get.name}
{include file=$element.template_path}
{/if}
{/foreach}
{/if}	
</div>
{/block}

