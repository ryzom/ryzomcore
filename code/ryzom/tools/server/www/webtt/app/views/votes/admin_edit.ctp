<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Votes', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('Vote.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('Vote.id'))); ?></li>						<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Votes', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Users</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('User', true)), array('controller' => 'users', 'action' => 'add')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Admin Edit %s', true), __('Vote', true)); ?></h2>
    
	<div class="votes form">
	<?php echo $this->Form->create('Vote');?>
		<fieldset>
						<legend><?php printf(__('Vote # %s', true), $this->Form->value('Vote.id')); ?></legend>
					<?php
		echo $this->Form->input('id');
		echo $this->Form->input('translation_id');
		echo $this->Form->input('user_id');
	?>
		</fieldset>
	<?php echo $this->Form->end(__('Submit', true));?>
	</div>

</div>
<div class="clear"></div>
