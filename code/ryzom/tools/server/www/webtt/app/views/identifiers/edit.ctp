<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Identifiers', true); ?></h5>
			<ul class="menu">
									<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Languages</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
			</ul>

			<h5>File Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>

	<?php echo $this->element('neighbours'); ?></div>

<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Edit %s', true), __('Identifier', true)); ?></h2>
    
	<div class="identifiers form">
	<?php echo $this->Form->create('Identifier');?>
		<fieldset>
						<legend><?php printf(__('Identifier # %s', true), $this->Form->value('Identifier.id')); ?></legend>
					<?php
		echo $this->Form->input('id');
		echo $this->Form->input('language_id');
		echo $this->Form->input('translation_index');
		echo $this->Form->input('identifier');
		echo $this->Form->input('arguments');
		echo $this->Form->input('reference_string');
		echo $this->Form->input('translated');
	?>
		</fieldset>
	<?php echo $this->Form->end(__('Submit', true));?>
	</div>

</div>
<div class="clear"></div>
