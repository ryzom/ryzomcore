<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Identifiers', true); ?></h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('action' => 'index')); ?> </li>
			</ul>
				
			<h5>Languages</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index', 'identifier_id' => $identifier['Identifier']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add', 'identifier_id' => $identifier['Identifier']['id'])); ?> </li>
			</ul>

			<h5>Comments</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index', 'identifier_id' => $identifier['Identifier']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add', 'identifier_id' => $identifier['Identifier']['id'])); ?> </li>
			</ul>
		</div>
		</div>
	</div>

	<?php echo $this->element('neighbours'); ?>
</div>

<div class="grid_13">
<p class="help">You can see translations for this identifier in "Related Translations" section. Click "View" on the list to see translation details. Click "New related Translation" below the list to add one.</p>
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
			<textarea rows="10" style="width: 100%" readonly="true"><?php echo $identifier['Identifier']['reference_string']; ?></textarea>
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

<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo (__('Related', true)); ?></a>
	</h2>
	<div class="block" id="related-records">
		<!-- RELATED -->
		<!-- IdentifierColumn -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Identifier Columns', true));?></h3>
			<?php if (!empty($identifier['IdentifierColumn'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Identifier Id'); ?></th>
						<th><?php __('Column Name'); ?></th>
						<th><?php __('Reference String'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
					</tr>
				</thead>
				<?php
				$i = 0;
				foreach ($identifier['IdentifierColumn'] as $identifierColumn):
					$class = null;
					if ($i++ % 2 == 0) {
						$class = ' class="altrow"';
					}
				?>
				<tr<?php echo $class;?>>
					<td><?php echo $identifierColumn['id'];?></td>
					<td><?php echo $identifierColumn['identifier_id'];?></td>
					<td><?php echo $identifierColumn['column_name'];?></td>
					<td><?php echo $identifierColumn['reference_string'];?></td>
					<td><?php echo $identifierColumn['created'];?></td>
					<td><?php echo $identifierColumn['modified'];?></td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
				</ul>
			</div>
		</div>
		<!-- Translation -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Translations', true));?></h3>
			<?php if (!empty($identifier['Translation'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Identifier Id'); ?></th>
						<th><?php __('Translation Text'); ?></th>
						<th><?php __('User Id'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
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
						<?php echo ' | '. $this->Html->link(__('Edit', true), array('controller' => 'translations', 'action' => 'edit', $translation['id'])); ?>
						<?php echo ' | '. $this->Html->link(__('Delete', true), array('controller' => 'translations', 'action' => 'delete', $translation['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translation['id'])); ?>
						<?php echo ' | '. $this->Html->link(__('Vote', true), array('controller' => 'votes', 'action' => 'vote', 'translation_id' => $translation['id'])); ?>
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
						<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index', 'identifier_id' => $identifier['Identifier']['id']));?></li>
						<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add', 'identifier_id' => $identifier['Identifier']['id']));?></li>
				</ul>
			</div>
		</div>
		<!-- Comment -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Comments', true));?></h3>
			<?php if (!empty($identifier['Comment'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Translation Id'); ?></th>
						<th><?php __('Identifier Id'); ?></th>
						<th><?php __('User Id'); ?></th>
						<th><?php __('Comment'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
				<?php
				$i = 0;
				foreach ($identifier['Comment'] as $comment):
					$class = null;
					if ($i++ % 2 == 0) {
						$class = ' class="altrow"';
					}
				?>
				<tr<?php echo $class;?>>
					<td><?php echo $comment['id'];?></td>
					<td><?php echo $comment['translation_id'];?></td>
					<td><?php echo $comment['identifier_id'];?></td>
					<td><?php echo $comment['user_id'];?></td>
					<td><?php echo $comment['comment'];?></td>
					<td><?php echo $comment['created'];?></td>
					<td><?php echo $comment['modified'];?></td>
					<td class="actions">
						<?php echo $this->Html->link(__('View', true), array('controller' => 'comments', 'action' => 'view', $comment['id'])); ?>
						<?php echo ' | '. $this->Html->link(__('Edit', true), array('controller' => 'comments', 'action' => 'edit', $comment['id'])); ?>
						<?php echo ' | '. $this->Html->link(__('Delete', true), array('controller' => 'comments', 'action' => 'delete', $comment['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $comment['id'])); ?>
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
						<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index', 'identifier_id' => $identifier['Identifier']['id']));?></li>
						<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add', 'identifier_id' => $identifier['Identifier']['id']));?></li>
				</ul>
			</div>
		</div>
		<!-- /RELATED -->
	</div>
</div>

</div>
<div class="clear"></div>
