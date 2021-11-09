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
 * WebTT bake template based on 960grid template
 * http://bakery.cakephp.org/articles/tom_m/2010/05/26/960-fluid-grid-system-bake-templates
 *
 */
?>
<div class="grid_3">
	<div class="box menubox">
		<h2>
			<a href="#" id="toggle-admin-actions">Actions</a>
		</h2>
		<div class="inbox">
		<div class="block" id="admin-actions">
			<h5><?php echo "<?php echo __('" . $pluralHumanName . "', true); ?>"; ?></h5>
			<ul class="menu">
			<?php
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "edit", $scaffoldForbiddenActions)))
				echo "\t<li><?php echo \$this->Html->link(sprintf(__('Edit %s', true), __('{$singularHumanName}', true)), array('action' => 'edit', \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "delete", $scaffoldForbiddenActions)))
				echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('Delete %s', true), __('{$singularHumanName}', true)), array('action' => 'delete', \${$singularVar}['{$modelClass}']['{$primaryKey}']), null, sprintf(__('Are you sure you want to delete # %s?', true), \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "index", $scaffoldForbiddenActions)))
				echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List %s', true), __('{$pluralHumanName}', true)), array('action' => 'index')); ?> </li>\n";
			if (empty($scaffoldForbiddenActions) || (!empty($scaffoldForbiddenActions) && !in_array($scaffoldPrefix . "add", $scaffoldForbiddenActions)))
				echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('{$singularHumanName}', true)), array('action' => 'add')); ?> </li>\n";
			?>
			</ul>
				<?php
					$done = array();
					foreach ($associations as $_type => $_data)
					{
						foreach ($_data as $alias => $details)
						{
							if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions'])))
							{
								echo "\n\t\t\t<h5>".Inflector::humanize($details['controller'])."</h5>";
								echo "\n\t\t\t<ul class=\"menu\">\n";
								if ($details['controller'] != $this->name && !in_array($details['controller'], $done)) {
									if ($_type == 'hasMany')
									{
										echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List related %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index', '{$details['foreignKey']}' => \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
										if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions'])))
											echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New related %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add', '{$details['foreignKey']}' => \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?> </li>\n";
									}
									echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('List all %s', true), __('" . Inflector::humanize($details['controller']) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index')); ?> </li>\n";
									if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions'])))
									{
										echo "\t\t\t\t<li><?php echo \$this->Html->link(sprintf(__('New %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add')); ?> </li>\n";
									}
									$done[] = $details['controller'];
								}
								echo "\t\t\t</ul>\n";
							}
						}
					}
				?>
		</div>
		</div>
	</div>
</div>

<div class="grid_13">

<div class="box">
	<div class="<?php echo $pluralVar;?> view">
	<h2><?php echo "<?php  __('{$singularHumanName}');?>";?></h2>
		<div class="block">
			<div class="dl">
				<?php
				echo "<?php \$i = 1; \$class = ' altrow';?>\n";
				foreach ($fields as $field) {
					if (!(empty($scaffoldForbiddenFields) || !isset($scaffoldForbiddenFields[$scaffoldPrefix]) || (!empty($scaffoldForbiddenFields) && isset($scaffoldForbiddenFields[$scaffoldPrefix]) && !in_array($field, $scaffoldForbiddenFields[$scaffoldPrefix]))))
						continue;

					$isKey = false;
					if (!empty($associations['belongsTo'])) {
						foreach ($associations['belongsTo'] as $alias => $details) {
							if ($field === $details['foreignKey'] && $field !== $primaryKey) {
								$isKey = true;
								echo "\t\t\t\t<div class=\"dt<?php if (\$i % 2 == 0) echo \$class;?>\"><?php __('" . Inflector::humanize(Inflector::underscore($alias)) . "'); ?></div>\n";
								echo "\t\t\t\t<div class=\"dd<?php if (\$i % 2 == 0) echo \$class;?>\">\n\t\t\t\t\t<?php echo \$this->Html->link(\${$singularVar}['{$alias}']['{$details['displayField']}'], array('controller' => '{$details['controller']}', 'action' => 'view', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?>\n\t\t\t\t</div>\n";
								break;
							}
						}
					}
					if ($isKey !== true) {
						echo "\t\t\t\t<div class=\"dt<?php if (\$i == 1) echo \" dh\"; else if (\$i % 2 == 0) echo \$class;?>\"><?php __('" . Inflector::humanize($field) . "'); ?></div>\n";
						echo "\t\t\t\t<div class=\"dd<?php if (\$i == 1) echo \" dh\"; else if (\$i % 2 == 0) echo \$class;?>\">\n\t\t\t\t\t<?php echo \${$singularVar}['{$modelClass}']['{$field}']; ?>\n\t\t\t\t</div>\n";
					}
					echo "\t\t\t\t<?php \$i++; ?>\n";
					echo "\t\t\t\t<div style=\"clear: both\"></div>\n\n";
				}
				?>
			</div>
		</div>
	</div>
</div>

<?php if(
		(!empty($associations['hasOne'])) ||
		(!empty($associations['hasMany'])) ||
		(!empty($associations['hasAndBelongsToMany']))
		) {
		?>
<div class="box">
	<h2>
		<a href="#" id="toggle-related-records"><?php echo "<?php echo (__('Related', true)); ?>"; ?></a>
	</h2>
	<div class="block" id="related-records">
		<!-- RELATED --><?php echo "\n";
		if (!empty($associations['hasOne'])) :
			foreach ($associations['hasOne'] as $alias => $details): ?>
			<!-- <?php echo $alias; ?> -->
			<div class="related">
				<h3><?php echo "<?php printf(__('Related %s (%s)', true), __('" . Inflector::humanize($details['controller']) . "', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true));?>";?></h3>
				<?php echo "<?php if (!empty(\${$singularVar}['{$alias}']['{$details['primaryKey']}'])):?>\n";?>
				<div class="dl">
				<?php
				echo "<?php \$i = 0; \$class = ' class=\"altrow\"';?>\n";?><?php echo "\n";
					foreach ($details['fields'] as $field) {
						echo "\t\t\t\t\t<div class=\"dt<?php if (\$i % 2 == 0) echo \$class;?>\"><?php __('" . Inflector::humanize($field) . "');?></div>\n";
						echo "\t\t\t\t\t<div class=\"dd<?php if (\$i++ % 2 == 0) echo \$class;?>\">\n";
						echo "\t\t\t\t\t\t<?php echo \${$singularVar}['{$alias}']['{$field}'];?>\n\t\t\t\t\t</div>\n";
						echo "\t\t\t\t\t<div style=\"clear: both\"></div>\n";
					}
					?>
				</div>
				<?php echo "<?php endif; ?>\n";?>

				<div class="actions">
					<ul>
					<?php echo "<?php if (!empty(\${$singularVar}['{$alias}']['{$details['primaryKey']}'])):?>\n";
					if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "view", $details['scaffoldForbiddenActions']))) {
						?>
						<li><?php echo "<?php echo \$this->Html->link(sprintf(__('View %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'view', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?>";
						?></li><? echo "\n";
					}
					?>
					<?php echo "<?php endif; ?>\n";?>
					<?php echo "<?php if (!empty(\${$singularVar}['{$alias}']['{$details['primaryKey']}'])):?>\n";
					if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions']))) {
						?>
						<li><?php echo "<?php echo \$this->Html->link(sprintf(__('Edit %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'edit', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?>";
						?></li><? echo "\n";
					}
					?>
					<?php echo "<?php endif; ?>\n";?>
					</ul>
				</div>
			</div><?php
			echo "\n";
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
		if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions']))):
			
			$otherSingularVar = Inflector::variable($alias);
			$otherPluralHumanName = Inflector::humanize($details['controller']);
			?>
		<!-- <?php echo $alias; ?> -->
		<div class="related">
			<h3><?php echo "<?php printf(__('Related %s', true), __('{$otherPluralHumanName}', true));?>";?></h3>
			<?php echo "<?php if (!empty(\${$singularVar}['{$alias}'])):?>\n";?>
			<table cellpadding = "0" cellspacing = "0">
				<thead>
					<tr><?php echo "\n";
							foreach ($details['fields'] as $field) {
								echo "\t\t\t\t\t\t<th><?php __('" . Inflector::humanize($field) . "'); ?></th>\n";
							}
						?>
						<th class="actions"><?php echo "<?php __('Actions');?>";?></th>
					</tr>
				</thead>
				<?php
				echo "<?php
				\$i = 0;
				foreach (\${$singularVar}['{$alias}'] as \${$otherSingularVar}):
					\$class = null;
					if (\$i++ % 2 == 0) {
						\$class = ' class=\"altrow\"';
					}
				?>\n";
				echo "\t\t\t\t<tr<?php echo \$class;?>>\n";

						foreach ($details['fields'] as $field) {
							echo "\t\t\t\t\t<td><?php echo \${$otherSingularVar}['{$field}'];?></td>\n";
						}

						echo "\t\t\t\t\t<td class=\"actions\">\n";
						if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "view", $details['scaffoldForbiddenActions'])))
							echo "\t\t\t\t\t\t<?php echo \$this->Html->link(__('View', true), array('controller' => '{$details['controller']}', 'action' => 'view', \${$otherSingularVar}['{$details['primaryKey']}'])); ?>\n";
						if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "edit", $details['scaffoldForbiddenActions'])))
							echo "\t\t\t\t\t\t<?php echo ' | '. \$this->Html->link(__('Edit', true), array('controller' => '{$details['controller']}', 'action' => 'edit', \${$otherSingularVar}['{$details['primaryKey']}'])); ?>\n";
						if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "delete", $details['scaffoldForbiddenActions'])))
							echo "\t\t\t\t\t\t<?php echo ' | '. \$this->Html->link(__('Delete', true), array('controller' => '{$details['controller']}', 'action' => 'delete', \${$otherSingularVar}['{$details['primaryKey']}']), null, sprintf(__('Are you sure you want to delete # %s?', true), \${$otherSingularVar}['{$details['primaryKey']}'])); ?>\n";
						echo "\t\t\t\t\t</td>\n";
					echo "\t\t\t\t</tr>\n";

			echo "\t\t\t\t<?php endforeach; ?>\n";
			?>
			</table>
			<?php echo "<?php endif; ?>\n\n";?>
			<div class="actions">
				<ul><?php echo "\n";
				if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "index", $details['scaffoldForbiddenActions']))) { ?>
					<li><?php
					echo "<?php echo \$this->Html->link(sprintf(__('List related %s', true), __('" . $otherPluralHumanName . "', true)), array('controller' => '{$details['controller']}', 'action' => 'index'";
					echo ", '{$details['foreignKey']}' => \${$singularVar}['{$modelClass}']['{$primaryKey}']";
					echo "));?>";
					?></li><? echo "\n";
				}
				if (empty($details['scaffoldForbiddenActions']) || (!empty($details['scaffoldForbiddenActions']) && !in_array($scaffoldPrefix . "add", $details['scaffoldForbiddenActions']))) { ?>
					<li><?php
					echo "<?php echo \$this->Html->link(sprintf(__('New related %s', true), __('" . Inflector::humanize(Inflector::underscore($alias)) . "', true)), array('controller' => '{$details['controller']}', 'action' => 'add'";
					echo ", '{$details['foreignKey']}' => \${$singularVar}['{$modelClass}']['{$primaryKey}']";
					echo "));?>";
					?></li><? echo "\n";
				}
				?>
				</ul>
			</div>
		</div><?php
		echo "\n";
		endif;
		endforeach; ?>
		<!-- /RELATED -->
	</div>
</div>
<?php } ?>

</div>
<div class="clear"></div>
