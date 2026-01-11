<div class="grid_3">
	<div class="box menubox">		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Translation Files', true); ?></h5>
			<ul class="menu">
							<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('action' => 'index')); ?> </li>
			</ul>
				
			<h5>Languages</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Language', true)), array('controller' => 'languages', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Imported Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">

<div class="box">
	<div class="translationFiles view">
	<h2><?php  __('Translation File');?></h2>
		<div class="block">
			<div class="dl">
			<?php $i = 1; $class = ' altrow';?>
						<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Id'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $translationFile['TranslationFile']['id']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Language'); ?></div>
		<div class="dd<?php if ($i % 2 == 0) echo $class;?>">
			<?php echo $this->Html->link($translationFile['Language']['name'], array('controller' => 'languages', 'action' => 'view', $translationFile['Language']['id'])); ?>
		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Filename Template'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $translationFile['TranslationFile']['filename_template']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Created'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $translationFile['TranslationFile']['created']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Modified'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $translationFile['TranslationFile']['modified']; ?>

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
		<!-- RELATED -->
		<!-- ImportedTranslationFile -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Imported Translation Files', true));?></h3>
			<?php if (!empty($translationFile['ImportedTranslationFile'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Language Id'); ?></th>
						<th><?php __('Translation File Id'); ?></th>
						<th><?php __('Filename'); ?></th>
						<th><?php __('Merged'); ?></th>
						<th><?php __('File Last Modified Date'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
				<?php
				$i = 0;
				foreach ($translationFile['ImportedTranslationFile'] as $importedTranslationFile):
					$class = null;
					if ($i++ % 2 == 0) {
						$class = ' class="altrow"';
					}
				?>
				<tr<?php echo $class;?>>
					<td><?php echo $importedTranslationFile['id'];?></td>
					<td><?php echo $importedTranslationFile['language_id'];?></td>
					<td><?php echo $importedTranslationFile['translation_file_id'];?></td>
					<td><?php echo $importedTranslationFile['filename'];?></td>
					<td><?php echo $importedTranslationFile['merged'];?></td>
					<td><?php echo $importedTranslationFile['file_last_modified_date'];?></td>
					<td><?php echo $importedTranslationFile['created'];?></td>
					<td><?php echo $importedTranslationFile['modified'];?></td>
					<td class="actions">
						<?php echo $this->Html->link(__('View', true), array('controller' => 'imported_translation_files', 'action' => 'view', $importedTranslationFile['id'])); ?>
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
				</ul>
			</div>
		</div>
		<!-- Identifier -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Identifiers', true));?></h3>
			<?php if (!empty($translationFile['Identifier'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Language Id'); ?></th>
						<th><?php __('Translation File Id'); ?></th>
						<th><?php __('Translation Index'); ?></th>
						<th><?php __('Identifier'); ?></th>
						<th><?php __('Arguments'); ?></th>
						<th><?php __('Translated'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
				<?php
				$i = 0;
				foreach ($translationFile['Identifier'] as $identifier):
					$class = null;
					if ($i++ % 2 == 0) {
						$class = ' class="altrow"';
					}
				?>
				<tr<?php echo $class;?>>
					<td><?php echo $identifier['id'];?></td>
					<td><?php echo $identifier['language_id'];?></td>
					<td><?php echo $identifier['translation_file_id'];?></td>
					<td><?php echo $identifier['translation_index'];?></td>
					<td><?php echo $identifier['identifier'];?></td>
					<td><?php echo $identifier['arguments'];?></td>
					<td><?php echo $identifier['translated'];?></td>
					<td><?php echo $identifier['created'];?></td>
					<td><?php echo $identifier['modified'];?></td>
					<td class="actions">
						<?php echo $this->Html->link(__('View', true), array('controller' => 'identifiers', 'action' => 'view', $identifier['id'])); ?>
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
				</ul>
			</div>
		</div>
		<!-- /RELATED -->
	</div>
</div>

</div>
<div class="clear"></div>
