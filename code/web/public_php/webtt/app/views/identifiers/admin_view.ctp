<div class="grid_3">
	<div class="box menubox">		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Identifiers', true); ?></h5>
			<ul class="menu">
							<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('action' => 'index')); ?> </li>
			</ul>
				
			<h5>Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Best Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Translations</h5>
			<ul class="menu">
			</ul>

			<h5>Comments</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add')); ?> </li>
			</ul>

			<h5>File Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('File Identifiers', true)), array('controller' => 'file_identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Identifier Columns</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifier Columns', true)), array('controller' => 'identifier_columns', 'action' => 'index')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">

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

				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Language Id'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $identifier['Identifier']['language_id']; ?>
				</div>
				<?php $i++; ?>
				<div style="clear: both"></div>

				<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Translation File'); ?></div>
				<div class="dd<?php if ($i % 2 == 0) echo $class;?>">
					<?php echo $this->Html->link($identifier['TranslationFile']['filename_template'], array('controller' => 'translation_files', 'action' => 'view', $identifier['TranslationFile']['id'])); ?>
				</div>
				<?php $i++; ?>
				<div style="clear: both"></div>

				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Translation Index'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $identifier['Identifier']['translation_index']; ?>
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

<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo (__('Related', true)); ?></a>
	</h2>
	<div class="block" id="related-records">
		<!-- RELATED -->
			<!-- BestTranslation -->
			<div class="related">
				<h3><?php printf(__('Related %s (%s)', true), __('Translations', true), __('Best Translation', true));?></h3>
				<?php if (!empty($identifier['BestTranslation']['id'])):?>
				<div class="dl">
				<?php $i = 0; $class = ' class="altrow"';?>

					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Id');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['id'];?>
					</div>
					<div style="clear: both"></div>
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Identifier Id');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['identifier_id'];?>
					</div>
					<div style="clear: both"></div>
<!--					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Identifier Column Id');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['identifier_column_id'];?>
					</div>
					<div style="clear: both"></div>-->
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Translation Text');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['translation_text'];?>
					</div>
					<div style="clear: both"></div>
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('User Id');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['user_id'];?>
					</div>
					<div style="clear: both"></div>
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Best');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['best'];?>
					</div>
					<div style="clear: both"></div>
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Created');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['created'];?>
					</div>
					<div style="clear: both"></div>
					<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Modified');?></div>
					<div class="dd<?php if ($i++ % 2 == 0) echo $class;?>">
						<?php echo $identifier['BestTranslation']['modified'];?>
					</div>
					<div style="clear: both"></div>
				</div>
				<?php endif; ?>

				<div class="actions">
					<ul>
<?php if (!empty($identifier['BestTranslation']['id'])):?>
						<li><?php echo $this->Html->link(sprintf(__('Edit %s', true), __('Best Translation', true)), array('controller' => 'translations', 'action' => 'edit', $identifier['BestTranslation']['id'])); ?></li>
					<?php endif; ?>
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
						<th><?php __('Identifier Column Id'); ?></th>
						<th><?php __('Translation Text'); ?></th>
						<th><?php __('User Id'); ?></th>
						<th><?php __('Best'); ?></th>
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
					<td><?php echo $translation['identifier_column_id'];?></td>
					<td><?php echo $translation['translation_text'];?></td>
					<td><?php echo $translation['user_id'];?></td>
					<td><?php echo $translation['best'];?></td>
					<td><?php echo $translation['created'];?></td>
					<td><?php echo $translation['modified'];?></td>
					<td class="actions">
						<?php echo $this->Html->link(__('View', true), array('controller' => 'translations', 'action' => 'view', $translation['id'])); ?>
						<?php echo ' | '. $this->Html->link(__('Edit', true), array('controller' => 'translations', 'action' => 'edit', $translation['id'])); ?>
						<?php echo ' | '. $this->Html->link(__('Delete', true), array('controller' => 'translations', 'action' => 'delete', $translation['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translation['id'])); ?>
						<?php echo ' | '. $this->Html->link(sprintf(__('Set as %s', true), __('Best Translation', true)), array('controller' => 'translations', 'action' => 'setBest', $translation['id']));?>
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
						<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add'));?></li>
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
						<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add'));?></li>
				</ul>
			</div>
		</div>
		<!-- FileIdentifier -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('File Identifiers', true));?></h3>
			<?php if (!empty($identifier['FileIdentifier'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Id'); ?></th>
						<th><?php __('Imported Translation File Id'); ?></th>
						<th><?php __('Command'); ?></th>
						<th><?php __('Translation Index'); ?></th>
						<th><?php __('Identifier Id'); ?></th>
						<th><?php __('Arguments'); ?></th>
						<th><?php __('Reference String'); ?></th>
						<th><?php __('Created'); ?></th>
						<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
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
					<td><?php echo $fileIdentifier['imported_translation_file_id'];?></td>
					<td><?php echo $fileIdentifier['command'];?></td>
					<td><?php echo $fileIdentifier['translation_index'];?></td>
					<td><?php echo $fileIdentifier['identifier_id'];?></td>
					<td><?php echo $fileIdentifier['arguments'];?></td>
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

			<div class="actions">
				<ul>
				</ul>
			</div>
		</div>
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
						<th class="actions"><?php __('Actions');?></th>
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
					<td class="actions">
						<?php echo $this->Html->link(__('View', true), array('controller' => 'identifier_columns', 'action' => 'view', $identifierColumn['id'])); ?>
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
