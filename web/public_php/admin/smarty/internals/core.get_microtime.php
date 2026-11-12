<?php
/**
 * Smarty plugin
 * @package Smarty
 * @subpackage plugins
 */

/**
 * Get seconds and microseconds
 * @return double
 */
function smarty_core_get_microtime($params, &$smarty)
{
    $mtime = microtime();
    $mtime = explode(" ", $mtime);
    $mtime = (float)($mtime[1]) + (float)($mtime[0]);
    return ($mtime);
}


/* vim: set expandtab: */

?>
