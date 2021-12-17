<div class="grid_3">
	<div class="box menubox">		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Imported Translation Files', true); ?></h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('action' => 'index')); ?> </li>
			</ul>
				
			<h5>Languages</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Language', true)), array('controller' => 'languages', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Raw Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Raw Files', true)), array('controller' => 'raw_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>File Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">

<div class="box">
	<div class="importedTranslationFiles view">
	<h2><?php  __('Imported Translation File');?></h2>
		<div class="block">
			<div class="dl">
			<?php $i = 1; $class = ' altrow';?>
						<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Id'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $importedTranslationFile['ImportedTranslationFile']['id']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Language'); ?></div>
		<div class="dd<?php if ($i % 2 == 0) echo $class;?>">
			<?php echo $this->Html->link($importedTranslationFile['Language']['name'], array('controller' => 'languages', 'action' => 'view', $importedTranslationFile['Language']['id'])); ?>
		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Filename'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $importedTranslationFile['ImportedTranslationFile']['filename']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Merged'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $importedTranslationFile['ImportedTranslationFile']['merged']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('File Last Modified Date'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $importedTranslationFile['ImportedTranslationFile']['file_last_modified_date']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Created'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $importedTranslationFile['ImportedTranslationFile']['created']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Modified'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $importedTranslationFile['ImportedTranslationFile']['modified']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
			</div>
		</div>
	</div>
</div>

<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo (__('Related', true)); ?></a>
	</h2>
	<div class="block" id="related-records">
					<div class="related">
				<h3><?php printf(__('Related %s', true), __('Raw Files', true));?></h3>
			<?php if (!empty($importedTranslationFile['RawFile'])):?>
				<dl>	<?php $i = 0; $class = ' class="altrow"';?>
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Filename');?></div>
		<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
	<?php echo $importedTranslationFile['RawFile']['filename'];?>
</div>
		<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Size');?></div>
		<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
	<?php echo $importedTranslationFile['RawFile']['size'];?>
</div>
		<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Modified');?></div>
		<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
	<?php echo $importedTranslationFile['RawFile']['modified'];?>
</div>
				</dl>
			<?php endif; ?>
				<div class="actions">
					<ul>
						<li><?php echo $this->Html->link(sprintf(__('Edit %s', true), __('Raw File', true)), array('controller' => 'raw_files', 'action' => 'edit', $importedTranslationFile['RawFile']['filename'])); ?></li>
					</ul>
				</div>
			</div>
					<div class="related">
			<h3><?php printf(__('Related %s', true), __('File Identifiers', true));?></h3>
			<?php if (!empty($importedTranslationFile['FileIdentifier'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
								<th><?php __('Id'); ?></th>
		<th><?php __('Imported Translation File Id'); ?></th>
		<th><?php __('Command'); ?></th>
		<th><?php __('Translation Index'); ?></th>
		<th><?php __('Identifier Id'); ?></th>
		<th><?php __('Arguments'); ?></th>
		<th><?php __('Created'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
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
			<td><?php echo $fileIdentifier['arguments'];?></td>
			<td><?php echo $fileIdentifier['created'];?></td>
			<td class="actions">
				<?php echo $this->Html->link(__('View', true), array('controller' => 'file_identifiers', 'action' => 'view', $fileIdentifier['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
			</table>
		<?php endif; ?>

			<div class="actions">
				<ul>
					<li></li>
				</ul>
			</div>
		</div>
			</div>
</div>

</div>
<div class="clear"></div>
