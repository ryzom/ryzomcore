<div class="languages view">
<h2><?php  __('Language');?></h2>
	<dl><?php $i = 0; $class = ' class="altrow"';?>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Id'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $language['Language']['id']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Name'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $language['Language']['name']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Code'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $language['Language']['code']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Created'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $language['Language']['created']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Modified'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $language['Language']['modified']; ?>
			&nbsp;
		</dd>
	</dl>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>
		<li><?php echo $this->Html->link(__('Edit Language', true), array('action' => 'edit', $language['Language']['id'])); ?> </li>
		<li><?php echo $this->Html->link(__('Delete Language', true), array('action' => 'delete', $language['Language']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $language['Language']['id'])); ?> </li>
		<li><?php echo $this->Html->link(__('List Languages', true), array('action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Language', true), array('action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add')); ?> </li>
		<li><?php echo $this->Html->link(__('List Translation Files', true), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
		<li><?php echo $this->Html->link(__('New Translation File', true), array('controller' => 'translation_files', 'action' => 'add')); ?> </li>
	</ul>
</div>
<div class="related">
	<h3><?php __('Related Identifiers');?></h3>
	<?php if (!empty($language['Identifier'])):?>
	<table cellpadding = "0" cellspacing = "0">
	<tr>
		<th><?php __('Id'); ?></th>
		<th><?php __('Language Id'); ?></th>
		<th><?php __('Translation Index'); ?></th>
		<th><?php __('Identifier'); ?></th>
		<th><?php __('Reference String'); ?></th>
		<th><?php __('Translated'); ?></th>
		<th><?php __('Created'); ?></th>
		<th><?php __('Modified'); ?></th>
		<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
		$i = 0;
		foreach ($language['Identifier'] as $identifier):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
		<tr<?php echo $class;?>>
			<td><?php echo $identifier['id'];?></td>
			<td><?php echo $identifier['language_id'];?></td>
			<td><?php echo $identifier['translation_index'];?></td>
			<td><?php echo $identifier['identifier'];?></td>
			<td><?php echo $identifier['reference_string'];?></td>
			<td><?php echo $identifier['translated'];?></td>
			<td><?php echo $identifier['created'];?></td>
			<td><?php echo $identifier['modified'];?></td>
			<td class="actions">
				<?php echo $this->Html->link(__('View', true), array('controller' => 'identifiers', 'action' => 'view', $identifier['id'])); ?>
				<?php echo $this->Html->link(__('Edit', true), array('controller' => 'identifiers', 'action' => 'edit', $identifier['id'])); ?>
				<?php echo $this->Html->link(__('Delete', true), array('controller' => 'identifiers', 'action' => 'delete', $identifier['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $identifier['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
	</table>
<?php endif; ?>

	<div class="actions">
		<ul>
			<li><?php echo $this->Html->link(__('New Identifier', true), array('controller' => 'identifiers', 'action' => 'add'));?> </li>
		</ul>
	</div>
</div>
<div class="related">
	<h3><?php __('Related Translation Files');?></h3>
	<?php if (!empty($language['TranslationFile'])):?>
	<table cellpadding = "0" cellspacing = "0">
	<tr>
		<th><?php __('Id'); ?></th>
		<th><?php __('Language Id'); ?></th>
		<th><?php __('Filename'); ?></th>
		<th><?php __('Created'); ?></th>
		<th><?php __('Modified'); ?></th>
		<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
		$i = 0;
		foreach ($language['TranslationFile'] as $translationFile):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
		<tr<?php echo $class;?>>
			<td><?php echo $translationFile['id'];?></td>
			<td><?php echo $translationFile['language_id'];?></td>
			<td><?php echo $translationFile['filename'];?></td>
			<td><?php echo $translationFile['created'];?></td>
			<td><?php echo $translationFile['modified'];?></td>
			<td class="actions">
				<?php echo $this->Html->link(__('View', true), array('controller' => 'translation_files', 'action' => 'view', $translationFile['id'])); ?>
				<?php echo $this->Html->link(__('Edit', true), array('controller' => 'translation_files', 'action' => 'edit', $translationFile['id'])); ?>
				<?php echo $this->Html->link(__('Delete', true), array('controller' => 'translation_files', 'action' => 'delete', $translationFile['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translationFile['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
	</table>
<?php endif; ?>

	<div class="actions">
		<ul>
			<li><?php echo $this->Html->link(__('New Translation File', true), array('controller' => 'translation_files', 'action' => 'add'));?> </li>
		</ul>
	</div>
</div>
