<div class="grid_3">
	<div class="box menubox">		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Users', true); ?></h5>
			<ul class="menu">
							<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Users', true)), array('action' => 'index')); ?> </li>
			</ul>
				
			<h5>Translations</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index', 'user_id' => $user['User']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add', 'user_id' => $user['User']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Votes</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index', 'user_id' => $user['User']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add', 'user_id' => $user['User']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add')); ?> </li>
			</ul>

			<h5>Comments</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index', 'user_id' => $user['User']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add', 'user_id' => $user['User']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('New %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add')); ?> </li>
			</ul>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">

<div class="box">
	<div class="users view">
	<h2><?php  __('User');?></h2>
		<div class="block">
			<div class="dl">
				<?php $i = 1; $class = ' altrow';?>
				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Id'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $user['User']['id']; ?>
				</div>
				<?php $i++; ?>
				<div style="clear: both"></div>

				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Name'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $user['User']['name']; ?>
				</div>
				<?php $i++; ?>
				<div style="clear: both"></div>

				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Email'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $user['User']['email']; ?>
				</div>
				<?php $i++; ?>
				<div style="clear: both"></div>

				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Username'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $user['User']['username']; ?>
				</div>
				<?php $i++; ?>
				<div style="clear: both"></div>

				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Role'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $user['User']['role']; ?>
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
		<!-- Translation -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Translations', true));?></h3>
			<?php if (!empty($user['Translation'])):?>
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
				foreach ($user['Translation'] as $translation):
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
					</td>
				</tr>
				<?php endforeach; ?>
			</table>
			<?php endif; ?>

			<div class="actions">
				<ul>
					<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Translations', true)), array('controller' => 'translations', 'action' => 'index', 'user_id' => $user['User']['id']));?></li>
					<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Translation', true)), array('controller' => 'translations', 'action' => 'add', 'user_id' => $user['User']['id']));?></li>
				</ul>
			</div>
		</div>
		<!-- Vote -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Votes', true));?></h3>
			<?php if (!empty($user['Vote'])):?>
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
				foreach ($user['Vote'] as $vote):
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
					<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Votes', true)), array('controller' => 'votes', 'action' => 'index', 'user_id' => $user['User']['id']));?></li>
					<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Vote', true)), array('controller' => 'votes', 'action' => 'add', 'user_id' => $user['User']['id']));?></li>
				</ul>
			</div>
		</div>
		<!-- Comment -->
		<div class="related">
			<h3><?php printf(__('Related %s', true), __('Comments', true));?></h3>
			<?php if (!empty($user['Comment'])):?>
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
				foreach ($user['Comment'] as $comment):
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
					<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Comments', true)), array('controller' => 'comments', 'action' => 'index', 'user_id' => $user['User']['id']));?></li>
					<li><?php echo $this->Html->link(sprintf(__('New related %s', true), __('Comment', true)), array('controller' => 'comments', 'action' => 'add', 'user_id' => $user['User']['id']));?></li>
				</ul>
			</div>
		</div>
		<!-- /RELATED -->
	</div>
</div>

</div>
<div class="clear"></div>
