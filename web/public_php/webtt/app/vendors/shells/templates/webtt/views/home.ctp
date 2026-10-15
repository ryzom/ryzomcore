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
 * @since         CakePHP(tm) v 0.10.0.1076
 * @license       MIT License (http://www.opensource.org/licenses/mit-license.php)
 *
 * WebTT bake template based on 960grid template
 * http://bakery.cakephp.org/articles/tom_m/2010/05/26/960-fluid-grid-system-bake-templates
 *
 */
?>
<div class="grid_16">

<?php
$output = "<h2>Sweet, \"" . Inflector::humanize($app) . "\" got Baked by CakePHP!</h2>\n";
$output .="
<?php
if (Configure::read() > 0):
	Debugger::checkSecurityKeys();
endif;
?>
<p>
<?php
	if (is_writable(TMP)):
		echo '<span class=\"notice success\">';
			__('Your tmp directory is writable.');
		echo '</span>';
	else:
		echo '<span class=\"notice\">';
			__('Your tmp directory is NOT writable.');
		echo '</span>';
	endif;
?>
</p>
<p>
<?php
	\$settings = Cache::settings();
	if (!empty(\$settings)):
		echo '<span class=\"notice success\">';
				printf(__('The %s is being used for caching. To change the config edit APP/config/core.php ', true), '<em>'. \$settings['engine'] . 'Engine</em>');
		echo '</span>';
	else:
		echo '<span class=\"notice\">';
				__('Your cache is NOT working. Please check the settings in APP/config/core.php');
		echo '</span>';
	endif;
?>
</p>
<p>
<?php
	\$filePresent = null;
	if (file_exists(CONFIGS . 'database.php')):
		echo '<span class=\"notice success\">';
			__('Your database configuration file is present.');
			\$filePresent = true;
		echo '</span>';
	else:
		echo '<span class=\"notice\">';
			__('Your database configuration file is NOT present.');
			echo '<br/>';
			__('Rename config/database.php.default to config/database.php');
		echo '</span>';
	endif;
?>
</p>
<?php
if (!empty(\$filePresent)):
	if (!class_exists('ConnectionManager')) {
		require LIBS . 'model' . DS . 'connection_manager.php';
	}
	\$db = ConnectionManager::getInstance();
 	\$connected = \$db->getDataSource('default');
?>
<p>
<?php
	if (\$connected->isConnected()):
		echo '<span class=\"notice success\">';
 			__('Cake is able to connect to the database.');
		echo '</span>';
	else:
		echo '<span class=\"notice\">';
			__('Cake is NOT able to connect to the database.');
		echo '</span>';
	endif;
?>
</p>\n";
$output .= "<?php endif;?>\n";
$output .= "<h3><?php __('Editing this Page') ?></h3>\n";
$output .= "<p>\n";
$output .= "<?php\n";
$output .= "\tprintf(__('To change the content of this page, edit: %s\n";
$output .= "\t\tTo change its layout, edit: %s\n";
$output .= "\t\tYou can also add some CSS styles for your pages at: %s', true),\n";
$output .= "\t\tAPP . 'views' . DS . 'pages' . DS . 'home.ctp.<br />',  APP . 'views' . DS . 'layouts' . DS . 'default.ctp.<br />', APP . 'webroot' . DS . 'css');\n";
$output .= "?>\n";
$output .= "</p>\n";
?>

</div>
<div class="clear"></div>
