<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('File Identifiers', true); ?></h5>
			<ul class="menu">
									<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Imported Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Admin Edit %s', true), __('File Identifier', true)); ?></h2>
    
	<div class="fileIdentifiers form">
	<?php echo $this->Form->create('FileIdentifier');?>
		<fieldset>
						<legend><?php printf(__('File Identifier # %s', true), $this->Form->value('FileIdentifier.id')); ?></legend>
					<?php
		echo $this->Form->input('id');
		echo $this->Form->input('imported_translation_file_id');
		echo $this->Form->input('command');
		echo $this->Form->input('translation_index');
		echo $this->Form->input('identifier_id');
		echo $this->Form->input('arguments');
		echo $this->Form->input('reference_string');
	?>
		</fieldset>
	<?php echo $this->Form->end(__('Submit', true));?>
	</div>

</div>
<div class="clear"></div>
