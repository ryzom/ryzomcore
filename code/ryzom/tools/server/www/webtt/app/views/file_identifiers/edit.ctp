<div class="fileIdentifiers form">
<?php echo $this->Form->create('FileIdentifier');?>
	<fieldset>
		<legend><?php __('Edit File Identifier'); ?></legend>
	<?php
		echo $this->Form->input('id');
		echo $this->Form->input('translation_file_id');
		echo $this->Form->input('command');
		echo $this->Form->input('translation_index');
		echo $this->Form->input('identifier_id');
		echo $this->Form->input('reference_string');
	?>
	</fieldset>
<?php echo $this->Form->end(__('Submit', true));?>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>

		<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('FileIdentifier.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('FileIdentifier.id'))); ?></li>
		<li><?php echo $this->Html->link(__('List File Identifiers', true), array('action' => 'index'));?></li>
		<li><?php echo $this->Html->link(__('List Translation Files', true), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation File', true), array('controller' => 'translation_files', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
	</ul>
</div>