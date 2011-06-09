<div class="importedTranslationFiles view">
<h2><?php  __('Imported Translation File');?></h2>
	<dl><?php $i = 0; $class = ' class="altrow"';?>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Id'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $importedTranslationFile['ImportedTranslationFile']['id']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Language'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $this->Html->link($importedTranslationFile['Language']['name'], array('controller' => 'languages', 'action' => 'view', $importedTranslationFile['Language']['id'])); ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Filename'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $importedTranslationFile['ImportedTranslationFile']['filename']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Created'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $importedTranslationFile['ImportedTranslationFile']['created']; ?>
			&nbsp;
		</dd>
		<dt<?php if ($i % 2 == 0) echo $class;?>><?php __('Modified'); ?></dt>
		<dd<?php if ($i++ % 2 == 0) echo $class;?>>
			<?php echo $importedTranslationFile['ImportedTranslationFile']['modified']; ?>
			&nbsp;
		</dd>
	</dl>
</div>
<div class="actions">
	<h3><?php __('Actions'); ?></h3>
	<ul>
		<li><?php echo $this->Html->link(__('List Translation Files', true), array('action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List Languages', true), array('controller' => 'languages', 'action' => 'index')); ?> </li>
		<li><br></li>
		<li><?php echo $this->Html->link(__('List File Identifiers', true), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
	</ul>
</div>
<div class="related">
	<h3><?php __('Related File Identifiers');?></h3>
	<?php if (!empty($importedTranslationFile['FileIdentifier'])):?>
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
		foreach ($importedTranslationFile['FileIdentifier'] as $fileIdentifier):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
		<tr<?php echo $class;?>>
			<td><?php echo $fileIdentifier['id'];?></td>
			<td><?php echo $fileIdentifier['imported_translation_file_id'];?></td>
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
<!--<div class="related">
	<h3><?php __('Related Raw Files');?></h3>
	<?php if (!empty($importedTranslationFile['RawFile'])):?>
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
		foreach ($importedTranslationFile['FileIdentifier'] as $fileIdentifier):
			$class = null;
			if ($i++ % 2 == 0) {
				$class = ' class="altrow"';
			}
		?>
		<tr<?php echo $class;?>>
			<td><?php echo $fileIdentifier['id'];?></td>
			<td><?php echo $fileIdentifier['imported_translation_file_id'];?></td>
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
	<?php
		endforeach; ?>
	</table>
	<?php endif; ?>
</div>
-->