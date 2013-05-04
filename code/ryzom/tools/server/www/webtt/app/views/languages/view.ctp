<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo __('Languages', true); ?></h5>
			<ul class="menu">
							<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Languages', true)), array('action' => 'index')); ?> </li>
			</ul>
				
			<h5>Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index', 'language_id' => $language['Language']['id'])); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index')); ?> </li>
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
					<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Translation Files', true)), array('controller' => 'translation_files', 'action' => 'index', 'language_id' => $language['Language']['id']));?></li>
				</ul>
			</div>
		</div>
		<!-- /RELATED -->
	</div>
</div>

</div>
<div class="clear"></div>
