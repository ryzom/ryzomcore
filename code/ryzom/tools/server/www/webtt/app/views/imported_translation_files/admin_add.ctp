<div class="translationFiles form">
<?php echo $this->Form->create('TranslationFile');?>
	<fieldset>
		<legend><?php __('Admin Add Translation File'); ?></legend>
	<?php
		echo $this->Form->input('language_id');
		echo $this->Form->input('filename');
	?>
	</fieldset>
<?php echo $this->Form->end(__('Submit', true));?>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>

		<li><?php echo $this->Html->link(__('List Translation Files', true), array('action' => 'index'));?></li>
		<li><?php echo $this->Html->link(__('List Languages', true), array('controller' => 'languages', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Language', true), array('controller' => 'languages', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List File Identifiers', true), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New File Identifier', true), array('controller' => 'file_identifiers', 'action' => 'add')); ?> </li>
	</ul>
</div>