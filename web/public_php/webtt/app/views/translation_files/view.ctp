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
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Languages', true)), array('controller' => 'languages', 'action' => 'index')); ?> </li>
			</ul>

			<h5>Identifiers</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List all %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index')); ?> </li>
				<li><?php echo $this->Html->link(sprintf(__('List related %s', true), __('Identifiers', true)), array('controller' => 'identifiers', 'action' => 'index', 'translation_file_id' => $translationFile['TranslationFile']['id'])); ?> </li>
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

<!--<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo (__('Related', true)); ?></a>
	</h2>
	<div class="block" id="related-records">
		<!-- RELATED -->
		<!-- Identifier -->
			<div class="actions">
				<ul>
				</ul>
			</div>
		<!-- /RELATED -->
	</div>
</div>-->

</div>
<div class="clear"></div>
