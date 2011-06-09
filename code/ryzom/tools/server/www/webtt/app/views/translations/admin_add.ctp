<div class="translations form">
<?php echo $this->Form->create('Translation');?>
	<fieldset>
		<legend><?php __('Admin Add Translation'); ?></legend>
	<?php
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

		<li><?php echo $this->Html->link(__('List Translations', true), array('action' => 'index'));?></li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Users', true), array('controller' => 'users', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New User', true), array('controller' => 'users', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Votes', true), array('controller' => 'votes', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Vote', true), array('controller' => 'votes', 'action' => 'add')); ?> </li>
	</ul>
</div>