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
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Translations', true)), array('action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('action' => 'add')); ?> </li>
			</ul>
				
			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Users</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('controller' => 'users', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('User', true)), array('controller' => 'users', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Votes</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
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
				<?php echo ' | '. $this->Html->link(__('Delete', true), array('controller' => 'votes', 'action' => 'delete', $vote['id']), null, sprintf(__('Are you sure you want to delete # %s?', true), $vote['id'])); ?>
			</td>
		</tr>
	<?php endforeach; ?>
			</table>
		<?php endif; ?>

			<div class="actions">
				<ul>
					<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add'));?></li>
				</ul>
			</div>
		</div>
			</div>
</div>

</div>
<div class="clear"></div>
