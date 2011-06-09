<div class="translations form">
<?php echo $this->Form->create('Translation');?>
	<fieldset>
		<legend><?php __('Edit Translation'); ?></legend>
	<?php
		echo $this->Form->input('id');
		echo $this->Form->input('identifier_id');
		echo $this->Form->input('translation_text');
		echo $this->Form->input('user_id');
	?>
	</fieldset>
<?php echo $this->Form->end(__('Submit', true));?>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>

		<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('Translation.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('Translation.id'))); ?></li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Translations', true), array('action' => 'index'));?></li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Users', true), array('controller' => 'users', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Votes', true), array('controller' => 'votes', 'action' => 'index')); ?> </li>
	</ul>
</div>