<div class="grid_3">
	<div class="box menubox">
				<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Translations', true); ?></h5>
			<ul class="menu">
						<li><?php echo $this->Html->link(__('Delete', true), array('action' => 'delete', $this->Form->value('Translation.id')), null, sprintf(__('Are you sure you want to delete # %s?', true), $this->Form->value('Translation.id'))); ?></li>						<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('action' => 'index'));?></li>			</ul>
			
			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Users</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Votes</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add')); ?> </li>
			</ul>
		</div>
		</div>
	</div>

	<?php echo $this->element('neighbours'); ?>
</div>


<div class="grid_13">
    <h2 id="page-heading"><?php printf(__('Edit %s', true), __('Translation', true)); ?></h2>
    
	<div class="translations form">
	<?php echo $this->Form->create('Translation');?>
		<fieldset>
						<legend><?php printf(__('Translation # %s', true), $this->Form->value('Translation.id')); ?></legend>
					<?php
		echo $this->Form->input('id');
		echo $this->Form->hidden('identifier_id', array('default' => $this->Form->value('Translation.identifier_id')));
		echo $this->Form->hidden('user_id', array('value' => $this->Session->read('Auth.User.id')));
		if (!empty($identifier['IdentifierColumn']))
		{
//			var_dump($this->data);
//			var_dump($translation);
//			var_dump($identifierColumnTranslations);
//			var_dump($identifierColumnsDetails2);
//			var_dump($mapChildTranslationsColumns);
			$i=$j=0;
			foreach($identifierColumns as $key => $column) {
				if (isset($mapChildTranslationsColumns[$key]))
					$i = $mapChildTranslationsColumns[$key];
				else
					$i = 'n'.$j++;
				echo $form->hidden('ChildTranslation.'.$i.'.identifier_column_id', array('default' => $key));
				echo $form->input('ChildTranslation.'.$i.'.identifier_column_id', array('type' => 'text', 'name'=>'buzu', 'value'=>$column, 'readonly' => 'readonly'));
				echo $form->input('ChildTranslation.'.$i.'.translation_text', array('rows' => 1, 'cols' => 80));
				echo $form->input('ChildTranslation.'.$i.'.id');
				echo $form->hidden('ChildTranslation.'.$i.'.user_id', array('value' => $this->Session->read('Auth.User.id')));
			}
		}
		else
		{
			echo $this->Form->input('identifier_id', array('type' => 'text', 'name'=>'buzu', 'value'=>$this->Form->value('Identifier.identifier'), 'readonly' => 'readonly'));
			echo $this->Form->input('translation_text', array('rows' => 8, 'cols' => 80));
		}
		
	?>
		</fieldset>
		<div class="box">
		<?php echo $this->Form->end(__('Save', true));?>
		</div>
	</div>

<div class="box">
	<div class="identifiers view">
	<h2><?php  __('Identifier');?></h2>
		<div class="block">
			<div class="dl">
			<?php $i = 1; $class = ' altrow';?>
						<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Id'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['id']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Translation File'); ?></div>
		<div class="dd<?php if ($i % 2 == 0) echo $class;?>">
			<?php echo $this->Html->link($identifier['TranslationFile']['filename_template'], array('controller' => 'translation_files', 'action' => 'view', $identifier['TranslationFile']['id'])); ?>
		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Identifier'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['identifier']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Arguments'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['arguments']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Reference String'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['reference_string']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Translated'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['translated']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Created'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['created']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Modified'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $identifier['Identifier']['modified']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
			</div>
		</div>
	</div>
</div>

</div>
<div class="clear"></div>
