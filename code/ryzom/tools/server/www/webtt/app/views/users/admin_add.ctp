<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Users', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Users', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Votes</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Comments</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Admin Add %s', true), __('User', true)); ?></h2>
    
	<div class="users form">
	<?php echo $this->Form->create('User');?>
		<fieldset>
				 		<legend><?php printf(__('User', true)); ?></legend>
					<?php
		echo $this->Form->input('name');
		echo $this->Form->input('email');
		echo $this->Form->input('activated');
		echo $this->Form->input('username');
		echo $this->Form->input('password');
		echo $this->Form->input('role');
		echo $this->Form->input('confirm_hash');
	?>
		</fieldset>
		<div class="box">
<?php echo $this->Form->end(__('Submit', true));?>
</div>	</div>

</div>
<div class="clear"></div>
