<div class="identifiers form">
<?php echo $this->Form->create('Identifier');?>
	<fieldset>
		<legend><?php __('Admin Edit Identifier'); ?></legend>
	<?php
		echo $this->Form->input('id');
		echo $this->Form->input('language_id');
		echo $this->Form->input('translation_index');
		echo $this->Form->input('identifier');
		echo $this->Form->input('reference_string');
		echo $this->Form->input('translated');
	?>
	</fieldset>
<?php echo $this->Form->end(__('Submit', true));?>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>

		<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('Identifier.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('Identifier.id'))); ?></li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('action' => 'index'));?></li>
		<li><?php echo $this->Html->link(__('List Languages', true), array('controller' => 'languages', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Language', true), array('controller' => 'languages', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Translations', true), array('controller' => 'translations', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation', true), array('controller' => 'translations', 'action' => 'add')); ?> </li>
	</ul>
</div>