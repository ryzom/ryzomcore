<?php

/*
 *
 * usage: {substr src="plop" start="0" length="4" delim="/"}
 *
 */

function smarty_function_substr($params, &$smarty)
{

    if (!isset($params['src'])) {
        $smarty->trigger_error("substr: missing 'src' parameter");
        return;
    }

    if (!isset($params['var'])) {
        $smarty->trigger_error("substr: missing 'var' parameter");
        return;
    }

	$tmp = "";
	if (isset($params['start']) && isset($params['length']))
	{
		$tmp = substr($params['src'], $params['start'], $params['length']);
	}
	elseif (isset($params['start']) && isset($params['delim']))
	{
		$tmp = substr($params['src'], $params['start'], strpos($params['src'], $params['delim']));
	}
	elseif (isset($params['delim']))
	{
		$tmp = substr($params['src'], strpos($params['src'], $params['delim']));
	}
	elseif (isset($params['start']))
	{
		$tmp = substr($params['src'], $params['start']);
	}
	elseif (isset($params['length']))
	{
		$tmp = substr($params['src'], 0, $params['length']);
	}
	else
	{
        $smarty->trigger_error("substr: missing start/stop/delim parameters");
        return;
	}


	$smarty->assign($params['var'], $tmp);

}


?>