<div class="grid_3">
	<div class="box menubox">		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Translations', true); ?></h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('Edit %s', true), __('Translation', true)), array('action' => 'edit', $translation['Translation']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('Delete %s', true), __('Translation', true)), array('action' => 'delete', $translation['Translation']['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $translation['Translation']['id'])); ?> </li>
			</ul>
				
			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Users</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Votes</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index', 'translation_id' => $translation['Translation']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add', 'translation_id' => $translation['Translation']['id'])); ?> </li>
			</ul>
		</div>
		</div>
	</div>

	<?php echo $this->element('neighbours'); ?>
</div>

<div class="grid_13">

<div class="box">
	<div class="translations view">
		<h2><?php  __('Translation');?></h2>
		<div class="block">
			<div class="dl">
			<?php $i = 1; $class = ' altrow';?>
			<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Id'); ?></div>
			<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
				<?php echo $translation['Translation']['id']; ?>
			</div>
			<?php $i++; ?>
			<div style="clear: both"></div>
			<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('Identifier'); ?></div>
			<div class="dd<?php if ($i % 2 == 0) echo $class;?>">
				<?php echo $this->Html->link($translation['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $translation['Identifier']['id'])); ?>
			</div>
			<?php $i++; ?>
			<div style="clear: both"></div>
			<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Translation Text'); ?></div>
			<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
				<?php echo $translation['Translation']['translation_text']; ?>
	
			</div>
			<?php $i++; ?>
			<div style="clear: both"></div>
			<div class="dt<?php if ($i % 2 == 0) echo $class;?>"><?php __('User'); ?></div>
			<div class="dd<?php if ($i % 2 == 0) echo $class;?>">
				<?php echo $this->Html->link($translation['User']['name'], array('controller' => 'users', 'action' => 'view', $translation['User']['id'])); ?>
			</div>
			<?php $i++; ?>
			<div style="clear: both"></div>
			<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Created'); ?></div>
			<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
				<?php echo $translation['Translation']['created']; ?>
	
			</div>
			<?php $i++; ?>
			<div style="clear: both"></div>
			<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Modified'); ?></div>
			<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
				<?php echo $translation['Translation']['modified']; ?>
	
			</div>
			<?php $i++; ?>
			<div style="clear: both"></div>
			</div>
		</div>

			<?php if (!empty($identifier['IdentifierColumn'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<th><?php __('Column Name'); ?></th>
						<th><?php __('Reference String'); ?></th>
						<th><?php __('Translation'); ?></th>
					</tr>
				</thead>
				<?php
				$i = 0;
				foreach ($columnTranslations as $identifierColumn):
					$class = null;
					if ($i++ % 2 == 0) {
						$class = ' class="altrow"';
					}
				?>
				<tr<?php echo $class;?>>
					<td><?php echo $identifierColumn['IdentifierColumn']['column_name'];?></td>
					<td><?php echo $identifierColumn['IdentifierColumn']['reference_string'];?></td>
					<td><?php echo $identifierColumn['Translation']['translation_text'];?></td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>


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

<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo (__('Related', true)); ?></a>
	</h2>
	<div class="block" id="related-records">
				<div class="related">
			<h3><?php printf(__('Related %s', true), __('Votes', true));?></h3>
			<?php if (!empty($translation['Vote'])):?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
								<th><?php __('Id'); ?></th>
		<th><?php __('Translation Id'); ?></th>
		<th><?php __('User Id'); ?></th>
		<th><?php __('Created'); ?></th>
		<th><?php __('Modified'); ?></th>
						<th class="actions"><?php __('Actions');?></th>
					</tr>
				</thead>
			<?php
				$i = 0;
				foreach ($translation['Vote'] as $vote):
					$class = null;
					if ($i++ % 2 == 0) {
						$class = ' class="altrow"';
					}
				?>
		<tr<?php echo $class;?>>
			<td><?php echo $vote['id'];?></td>
			<td><?php echo $vote['translation_id'];?></td>
			<td><?php echo $vote['user_id'];?></td>
			<td><?php echo $vote['created'];?></td>
			<td><?php echo $vote['modified'];?></td>
			<td class="actions">
				<?php echo $this->Html->link(__('View', true), array('controller' => 'votes', 'action' => 'view', $vote['id'])); ?>
				<?php echo ' | '. $this->Html->link(__('Edit', true), array('controller' => 'votes', 'action' => 'edit', $vote['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
			</table>
		<?php endif; ?>

			<div class="actions">
				<ul>
					<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index', 'translation_id' => $translation['Translation']['id']));?></li>
					<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'vote', 'translation_id' => $translation['Translation']['id']));?></li>
				</ul>
			</div>
		</div>
			</div>
</div>

</div>
<div class="clear"></div>
