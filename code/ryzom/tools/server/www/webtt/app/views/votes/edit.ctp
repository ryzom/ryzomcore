<div class="votes form">
<?php echo $this->Form->create('Vote');?>
	<fieldset>
		<legend><?php __('Edit Vote'); ?></legend>
	<?php
		echo $this->Form->input('id');
		echo $this->Form->input('translation_id');
		echo $this->Form->input('user_id');
	?>
	</fieldset>
<?php echo $this->Form->end(__('Submit', true));?>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>

		<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('Vote.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('Vote.id'))); ?></li>
		<li><?php echo $this->Html->link(__('List Votes', true), array('action' => 'index'));?></li>
		<li><?php echo $this->Html->link(__('List Translations', true), array('controller' => 'translations', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation', true), array('controller' => 'translations', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Users', true), array('controller' => 'users', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New User', true), array('controller' => 'users', 'action' => 'add')); ?> </li>
	</ul>
</div>