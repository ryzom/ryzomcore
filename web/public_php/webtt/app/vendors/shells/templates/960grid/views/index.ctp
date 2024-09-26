<?php
/**
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) : Rapid Development Framework (http://cakephp.org)
 * Copyright 2005-2009, Cake Software Foundation, Inc. (http://cakefoundation.org)
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @copyright     Copyright 2005-2009, Cake Software Foundation, Inc. (http://cakefoundation.org)
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
				<li><?php echo "<?php echo \$this->Html->link(sprintf(__('New %s', true), __('{$singularHumanName}', true)), array('action' => 'add')); ?>";?></li>
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
    <h2 id="page-heading"><?php echo "<?php __('{$pluralHumanName}');?>";?></h2>
	
	<?php //// TABLE WITH RECORDS ?>
	
	<table cellpadding="0" cellspacing="0">
    <?php 
    //// TABLE HEADERS
    echo "<?php \$tableHeaders = \$html->tableHeaders(array("; 
    
    foreach($fields as $field) { 
    	echo "\$paginator->sort('{$field}'),";
    }
    echo "__('Actions', true),";
    echo "));\n";
    
    echo "echo '<thead>'.\$tableHeaders.'</thead>'; ?>\n\n"; 
    
    //// TABLE ROWS
    
	echo "<?php
	\$i = 0;
	foreach (\${$pluralVar} as \${$singularVar}):
		\$class = null;
		if (\$i++ % 2 == 0) {
			\$class = ' class=\"altrow\"';
		}
	?>\n";
	echo "\t<tr<?php echo \$class;?>>\n";
		foreach ($fields as $field) {
			$isKey = false;
			if (!empty($associations['belongsTo'])) {
				foreach ($associations['belongsTo'] as $alias => $details) {
					if ($field === $details['foreignKey']) {
						$isKey = true;
						echo "\t\t<td>\n\t\t\t<?php echo \$this->Html->link(\${$singularVar}['{$alias}']['{$details['displayField']}'], array('controller' => '{$details['controller']}', 'action' => 'view', \${$singularVar}['{$alias}']['{$details['primaryKey']}'])); ?>\n\t\t</td>\n";
						break;
					}
				}
			}
			if ($isKey !== true) {
				echo "\t\t<td><?php echo \${$singularVar}['{$modelClass}']['{$field}']; ?>&nbsp;</td>\n";
			}
		}

		echo "\t\t<td class=\"actions\">\n";
		echo "\t\t\t<?php echo \$this->Html->link(__('View', true), array('action' => 'view', \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?>\n";
	 	echo "\t\t\t<?php echo ' | ' . \$this->Html->link(__('Edit', true), array('action' => 'edit', \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?>\n";
	 	echo "\t\t\t<?php echo ' | ' . \$this->Html->link(__('Delete', true), array('action' => 'delete', \${$singularVar}['{$modelClass}']['{$primaryKey}']), null, sprintf(__('Are you sure you want to delete # %s?', true), \${$singularVar}['{$modelClass}']['{$primaryKey}'])); ?>\n";
		echo "\t\t</td>\n";
	echo "\t</tr>\n";

	echo "<?php endforeach; ?>\n";
	
	//// TABLE FOOTER
    echo "<?php echo '<tfoot class=\'dark\'>'.\$tableHeaders.'</tfoot>'; ?>";
    ?>
    </table>
    
    <?php //// PAGINATION ?>
      
	<p>
	<?php echo "<?php
	echo \$this->Paginator->counter(array(
	'format' => __('Page %page% of %pages%, showing %current% records out of %count% total, starting on record %start%, ending on %end%', true)
	));
	?>";?>
	</p>

	<div class="paging">
	<?php echo "\t<?php echo \$this->Paginator->prev('<< '.__('previous', true), array(), null, array('class'=>'disabled'));?>\n";?>
	 | <?php echo "\t<?php echo \$this->Paginator->numbers();?>\n"?> |
	<?php echo "\t<?php echo \$this->Paginator->next(__('next', true).' >>', array(), null, array('class' => 'disabled'));?>\n";?>
	</div>
		
</div>
<div class="clear"></div>
