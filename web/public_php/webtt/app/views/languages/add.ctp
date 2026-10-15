<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Languages', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Languages', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Add %s', true), __('Language', true)); ?></h2>
    
	<div class="languages form">
	<?php echo $this->Form->create('Language');?>
		<fieldset>
				 		<legend><?php printf(__('Language', true)); ?></legend>
					<?php
		echo $this->Form->input('name');
		echo $this->Form->input('code');
	?>
		</fieldset>
		<div class="box">
<?php echo $this->Form->end(__('Submit', true));?>
</div>	</div>

</div>
<div class="clear"></div>
