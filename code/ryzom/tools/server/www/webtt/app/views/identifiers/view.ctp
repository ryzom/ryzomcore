<div class="identifiers view">
<h2><?php  __('Identifier');?></h2>
	<dl><?php $i = 0; $class = ' class="altrow"';?>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Id'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['id']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Language'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $this->Html->link($identifier['Language']['name'], array('controller' => 'languages', 'action' => 'view', $identifier['Language']['id'])); ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Translation Index'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['translation_index']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Identifier'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['identifier']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Reference String'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['reference_string']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Translated'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['translated']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Created'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['created']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Modified'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $identifier['Identifier']['modified']; ?>
			&nbsp;
		</dd>
	</dl>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>
		<li><?php echo $this->Html->link(__('List Identifiers', true), array('action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Languages', true), array('controller' => 'languages', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Translations', true), array('controller' => 'translations', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List File Identifiers', true), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
	</ul>
</div>
<div class="related">
	<h3><?php __('Related Translations');?></h3>
	<?php if (!empty($identifier['Translation'])):?>
	<table cellpadding = "0" cellspacing = "0">
	<tr>
		<th><?php __('Id'); ?></th>
		<th><?php __('Identifier Id'); ?></th>
		<th><?php __('Translation Text'); ?></th>
		<th><?php __('User Id'); ?></th>
		<th><?php __('Created'); ?></th>
		<th><?php __('Modified'); ?></th>
		<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
		$i = 0;
		foreach ($identifier['Translation'] as $translation):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
		<tr<?php echo $class;?>>
			<td><?php echo $translation['id'];?></td>
			<td><?php echo $translation['identifier_id'];?></td>
			<td><?php echo $translation['translation_text'];?></td>
			<td><?php echo $translation['user_id'];?></td>
			<td><?php echo $translation['created'];?></td>
			<td><?php echo $translation['modified'];?></td>
			<td class="actions">
				<?php echo $this->Html->link(__('View', true), array('controller' => 'translations', 'action' => 'view', $translation['id'])); ?>
				<?php echo $this->Html->link(__('Edit', true), array('controller' => 'translations', 'action' => 'edit', $translation['id'])); ?>
				<?php echo $this->Html->link(__('Delete', true), array('controller' => 'translations', 'action' => 'delete', $translation['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translation['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
	</table>
<?php endif; ?>

	<div class="actions">
		<ul>
			<li><?php echo $this->Html->link(__('New Translation', true), array('controller' => 'translations', 'action' => 'add'));?> </li>
		</ul>
	</div>
</div>
<div class="related">
	<h3><?php __('Related File Identifiers');?></h3>
	<?php if (!empty($identifier['FileIdentifier'])):?>
	<table cellpadding = "0" cellspacing = "0">
	<tr>
		<th><?php __('Id'); ?></th>
		<th><?php __('Translation File Id'); ?></th>
		<th><?php __('Command'); ?></th>
		<th><?php __('Translation Index'); ?></th>
		<th><?php __('Identifier Id'); ?></th>
		<th><?php __('Reference String'); ?></th>
		<th><?php __('Created'); ?></th>
		<th><?php __('Modified'); ?></th>
		<th class="actions"><?php __('Actions');?></th>
	</tr>
	<?php
		$i = 0;
		foreach ($identifier['FileIdentifier'] as $fileIdentifier):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
		<tr<?php echo $class;?>>
			<td><?php echo $fileIdentifier['id'];?></td>
			<td><?php echo $fileIdentifier['translation_file_id'];?></td>
			<td><?php echo $fileIdentifier['command'];?></td>
			<td><?php echo $fileIdentifier['translation_index'];?></td>
			<td><?php echo $fileIdentifier['identifier_id'];?></td>
			<td><?php echo $fileIdentifier['reference_string'];?></td>
			<td><?php echo $fileIdentifier['created'];?></td>
			<td><?php echo $fileIdentifier['modified'];?></td>
			<td class="actions">
				<?php echo $this->Html->link(__('View', true), array('controller' => 'file_identifiers', 'action' => 'view', $fileIdentifier['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
	</table>
<?php endif; ?>
</div>
