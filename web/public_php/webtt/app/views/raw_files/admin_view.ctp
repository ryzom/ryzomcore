<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox"><div class="block" id="admin-actions">
			<h5>Raw Files</h5>
			<ul class="menu">				
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Raw Files', true)), array('action' => 'index')); ?> </li>
			</ul>			
				
			<h5>Imported Translation Files</h5>
			<ul class="menu">
				<li><?php echo $this->Html->link(sprintf(__('List %s', true), __('Imported Translation Files', true)), array('controller' => 'imported_translation_files', 'action' => 'index')); ?> </li>
			</ul>
		</div></div>
	</div>
</div>

<div class="grid_13">

<div class="box">
	<div class="rawFiles view">
		<h2><?php  __('Raw File');?></h2>
		<div class="block">
			<div class="dl">
				<?php $i = 1; $class = ' altrow';?>
				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Filename'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $rawFile['RawFile']['filename']; ?>
					&nbsp;
				</div>
				<?php $i++; ?>
				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Size'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $rawFile['RawFile']['size']; ?>
					&nbsp;
				</div>
				<?php $i++; ?>
				<div class="dt<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>"><?php __('Modified'); ?></div>
				<div class="dd<?php if ($i == 1) echo " dh"; else if ($i % 2 == 0) echo $class;?>">
					<?php echo $this->Time->nice($rawFile['RawFile']['modified']); ?>
					&nbsp;
				</div>
				<?php $i++; ?>
			</div>
		</div>
	</div>
</div>

<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo (__('File Content', true)); ?></a>
	</h2>
	<div class="block" id="related-records">
		<div class="related">
		<textarea style="width: 100%; height: 300px" readonly>
		<?php echo htmlentities($fileContent, ENT_COMPAT, "UTF-8"); ?>
		</textarea>
		</div>
	</div>
</div>

</div>
<div class="clear"></div>
