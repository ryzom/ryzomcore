<?php
/**
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2010, Cake Software Foundation, Inc. (http://cakefoundation.org)
 * @link          http://cakephp.org CakePHP(tm) Project
 * @package       cake
 * @subpackage    cake.cake.console.libs.templates.views
 * @since         CakePHP(tm) v 1.2.0.5234
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 */
?>
<div class="grid_4">	
	<div class="box">			
		<?php //// ACTIONS ?>
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="block" id="admin-actions">
			<h5><?php echo $pluralHumanName; ?></h5>
			<ul class="menu">				
			<?php
			echo "\t<li><?php echo \$this->Html->link(sprintf(__('Edit %s', true), __('{$singularHumanName}', true)), array('action' => 'edit', \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
			echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('Delete %s', true), __('{$singularHumanName}', true)), array('action' => 'delete', \${$singularVar}['{$modelClass}']['{$primaryKey}']), null, sprintf(__('Are you sure you want to delete # %s?', true), \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
			echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List %s', true), __('{$pluralHumanName}', true)), array('action' => 'index')); ?> </li>\n";
			echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('{$singularHumanName}', true)), array('action' => 'add')); ?> </li>\n";
			?>
			</ul>			
				<?php
					$done = array();
					foreach ($associations as $type => $data) {
						foreach ($data as $alias => $details) {
							echo "\n\t\t\t<h5>".Inflector::humanize($details['controller'])."</h5>";
							echo "\n\t\t\t<ul class=\"menu\">\n";
							if ($details['controller'] != $this->name && !in_array($details['controller'], $done)) {
								echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index')); ?> </li>\n";
								echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add')); ?> </li>\n";
								$done[] = $details['controller'];
							}
							echo "\t\t\t</ul>\n";
						}
					}
				?>
		</div>
	</div>
</div>

<div class="grid_12">

<div class="box">
	<div class="<?php echo $pluralVar;?> view">
	<h2><?php echo "<?php  __('{$singularHumanName}');?>";?></h2>
		<div class="block">
			<dl><?php echo "<?php \$i = 0; \$class = ' class=\"altrow\"';?>\n";?>
				<?php
				foreach ($fields as $field) {
					$isKey = false;
					if (!empty($associations['belongsTo'])) {
						foreach ($associations['belongsTo'] as $alias => $details) {
							if ($field === $details['foreignKey']) {
								$isKey = true;
								echo "\t\t<dt<?php if (\$i % 2 == 0) echo \$class;?>><?php __('" . Inflector::humanize(Inflector::underscore($alias)) . "'); ?></dt>\n";
								echo "\t\t<dd<?php if (\$i++ % 2 == 0) echo \$class;?>>\n\t\t\t<?php echo \$this->Html->link(\${$singularVar}['{$alias}']['{$details['displayField']}'], array('controller' => '{$details['controller']}', 'action' => 'view', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?>\n\t\t\t&nbsp;\n\t\t</dd>\n";
								break;
							}
						}
					}
					if ($isKey !== true) {
						echo "\t\t<dt<?php if (\$i % 2 == 0) echo \$class;?>><?php __('" . Inflector::humanize($field) . "'); ?></dt>\n";
						echo "\t\t<dd<?php if (\$i++ % 2 == 0) echo \$class;?>>\n\t\t\t<?php echo \${$singularVar}['{$modelClass}']['{$field}']; ?>\n\t\t\t&nbsp;\n\t\t</dd>\n";
					}
				}
				?>
			</dl>
		</div>
	</div>
</div>

<?php if(
		(!empty($associations['hasOne'])) ||
		(!empty($associations['hasMany'])) ||
		(!empty($associations['hasAndBelongsToMany']))
		) { ?>
<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo "<?php echo (__('Related', true)); ?>"; ?></a>
	</h2>
	<div class="block" id="related-records">
		<?php
		if (!empty($associations['hasOne'])) :
			foreach ($associations['hasOne'] as $alias => $details): ?>
			<div class="related">
				<h3><?php echo "<?php printf(__('Related %s', true), __('" . Inflector::humanize($details['controller']) . "', true));?>";?></h3>
			<?php echo "<?php if (!empty(\${$singularVar}['{$alias}'])):?>\n";?>
				<dl><?php echo "\t<?php \$i = 0; \$class = ' class=\"altrow\"';?>\n";?>
			<?php
					foreach ($details['fields'] as $field) {
						echo "\t\t<dt<?php if (\$i % 2 == 0) echo \$class;?>><?php __('" . Inflector::humanize($field) . "');?></dt>\n";
						echo "\t\t<dd<?php if (\$i++ % 2 == 0) echo \$class;?>>\n\t<?php echo \${$singularVar}['{$alias}']['{$field}'];?>\n&nbsp;</dd>\n";
					}
			?>
				</dl>
			<?php echo "<?php endif; ?>\n";?>
				<div class="actions">
					<ul>
						<li><?php echo "<?php echo \$this->Html->link(sprintf(__('Edit %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'edit', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?></li>\n";?>
					</ul>
				</div>
			</div>
			<?php
			endforeach;
		endif;
		if (empty($associations['hasMany'])) {
			$associations['hasMany'] = array();
		}
		if (empty($associations['hasAndBelongsToMany'])) {
			$associations['hasAndBelongsToMany'] = array();
		}
		$relations = array_merge($associations['hasMany'], $associations['hasAndBelongsToMany']);
		$i = 0;
		foreach ($relations as $alias => $details):
			$otherSingularVar = Inflector::variable($alias);
			$otherPluralHumanName = Inflector::humanize($details['controller']);
			?>
		<div class="related">
			<h3><?php echo "<?php printf(__('Related %s', true), __('{$otherPluralHumanName}', true));?>";?></h3>
			<?php echo "<?php if (!empty(\${$singularVar}['{$alias}'])):?>\n";?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr>
						<?php
							foreach ($details['fields'] as $field) {
								echo "\t\t<th><?php __('" . Inflector::humanize($field) . "'); ?></th>\n";
							}
						?>
						<th class="actions"><?php echo "<?php __('Actions');?>";?></th>
					</tr>
				</thead>
		<?php
		echo "\t<?php
				\$i = 0;
				foreach (\${$singularVar}['{$alias}'] as \${$otherSingularVar}):
					\$class = null;
					if (\$i++ % 2 == 0) {
						\$class = ' class=\"altrow\"';
					}
				?>\n";
				echo "\t\t<tr<?php echo \$class;?>>\n";

						foreach ($details['fields'] as $field) {
							echo "\t\t\t<td><?php echo \${$otherSingularVar}['{$field}'];?></td>\n";
						}

						echo "\t\t\t<td class=\"actions\">\n";
						echo "\t\t\t\t<?php echo \$this->Html->link(__('View', true), array('controller' => '{$details['controller']}', 'action' => 'view', \${$otherSingularVar}['{$details['primaryKey']}'])); ?>\n";
						echo "\t\t\t\t<?php echo ' | '. \$this->Html->link(__('Edit', true), array('controller' => '{$details['controller']}', 'action' => 'edit', \${$otherSingularVar}['{$details['primaryKey']}'])); ?>\n";
						echo "\t\t\t\t<?php echo ' | '. \$this->Html->link(__('Delete', true), array('controller' => '{$details['controller']}', 'action' => 'delete', \${$otherSingularVar}['{$details['primaryKey']}']), null, sprintf(__('Are you sure you want to delete # %s?', true), \${$otherSingularVar}['{$details['primaryKey']}'])); ?>\n";
						echo "\t\t\t</td>\n";
					echo "\t\t</tr>\n";

		echo "\t<?php endforeach; ?>\n";
		?>
			</table>
		<?php echo "<?php endif; ?>\n\n";?>
			<div class="actions">
				<ul>
					<li><?php echo "<?php echo \$this->Html->link(sprintf(__('New %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add'));?>";?> </li>
				</ul>
			</div>
		</div>
		<?php endforeach;?>
	</div>
</div>
<?php } ?>

</div>
<div class="clear"></div>
