<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Translations', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Users</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('User', true)), array('controller' => 'users', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Votes</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Admin Add %s', true), __('Translation', true)); ?></h2>
    
	<div class="translations form">
	<?php echo $this->Form->create('Translation');?>
		<fieldset>
				 		<legend><?php printf(__('Translation', true)); ?></legend>
					<?php
		echo $this->Form->input('identifier_id');
		echo $this->Form->input('translation_text');
		echo $this->Form->input('user_id');
	?>
		</fieldset>
	<?php echo $this->Form->end(__('Submit', true));?>
	</div>

</div>
<div class="clear"></div>
