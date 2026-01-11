	<?php if (isset($identifierNeighbours)): ?>
	<?php //var_dump($identifierNeighbours); ?>
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-neighbour-actions"><?php echo sprintf(__('Neighbour %s', true), __('Identifiers', true)); ?></a>
		</h2>
		<div class="inbox">
		<div class="block" id="neighbour-actions">
		<?php foreach (array('prev', 'current', 'next') as $pos): ?>
		<?php foreach ($identifierNeighbours[$pos] as $neighbourIdentifier): ?>
			<ul class="list menu">
				<li<?php if ($pos == 'current') { echo " class=\"current\""; } ?>><?php echo $this->Html->link($neighbourIdentifier['Identifier']['identifier'], array('controller' => 'identifiers', 'action' => 'view', $neighbourIdentifier['Identifier']['id'])); ?> </li>
			</ul>
		<?php endforeach; ?>
		<?php endforeach; ?>
		</div>
		</div>
	</div>
	<?php endif; ?>
