<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Comments', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('Comment.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('Comment.id'))); ?></li>						<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Comments', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Users</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Edit %s', true), __('Comment', true)); ?></h2>
    
	<div class="comments form">
	<?php echo $this->Form->create('Comment');?>
		<fieldset>
						<legend><?php printf(__('Comment # %s', true), $this->Form->value('Comment.id')); ?></legend>
					<?php
		echo $this->Form->input('id');
		echo $this->Form->input('translation_id');
		echo $this->Form->input('identifier_id');
		echo $this->Form->input('user_id');
		echo $this->Form->input('comment');
	?>
		</fieldset>
		<div class="box">
<?php echo $this->Form->end(__('Submit', true));?>
</div>	</div>

</div>
<div class="clear"></div>
