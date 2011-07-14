<div class="grid_3">
	<div class="box menubox">		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Languages', true); ?></h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('Edit %s', true), __('Language', true)), array('action' => 'edit', $language['Language']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('Delete %s', true), __('Language', true)), array('action' => 'delete', $language['Language']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $language['Language']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Language', true)), array('action' => 'add')); ?> </li>
			</ul>
				
			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Imported Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">

<div class="box">
	<div class="languages view">
	<h2><?php  __('Language');?></h2>
		<div class="block">
			<div class="dl">
			<?php $i = 1; $class = ' altrow';?>
						<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Id'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $language['Language']['id']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Name'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $language['Language']['name']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Code'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $language['Language']['code']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Created'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $language['Language']['created']; ?>

		</div>
		<?php $i++; ?>
		<div style="clear: both"></div>
		<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Modified'); ?></div>
		<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
			<?php echo $language['Language']['modified']; ?>

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
		<!-- Identifier -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Identifiers', true));?></h3>
			<?php if (!empty($language['Identifier'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Language Id'); ?></th>
						<th><?php __('Translation File Id'); ?></th>
						<th><?php __('Translation Index'); ?></th>
						<th><?php __('Identifier'); ?></th>
						<th><?php __('Arguments'); ?></th>
						<th><?php __('Reference String'); ?></th>
						<th><?php __('Translated'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
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
					<td><?php echo $identifier['translation_file_id'];?></td>
					<td><?php echo $identifier['translation_index'];?></td>
					<td><?php echo $identifier['identifier'];?></td>
					<td><?php echo $identifier['arguments'];?></td>
					<td><?php echo $identifier['reference_string'];?></td>
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
		<!-- ImportedTranslationFile -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Imported Translation Files', true));?></h3>
			<?php if (!empty($language['ImportedTranslationFile'])):?>
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
				foreach ($language['ImportedTranslationFile'] as $importedTranslationFile):
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
		<!-- TranslationFile -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Translation Files', true));?></h3>
			<?php if (!empty($language['TranslationFile'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Language Id'); ?></th>
						<th><?php __('Filename Template'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
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
					<td><?php echo $translationFile['filename_template'];?></td>
					<td><?php echo $translationFile['created'];?></td>
					<td><?php echo $translationFile['modified'];?></td>
					<td class="actions">
						<?php echo $this->Html->link(__('View', true), array('controller' => 'translation_files', 'action' => 'view', $translationFile['id'])); ?>
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
