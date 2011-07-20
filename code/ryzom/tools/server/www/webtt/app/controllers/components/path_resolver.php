<?php
class PathResolverComponent extends Object {
	function getAssociationsTree($model)
	{
		$names = array();
		foreach ($model->belongsTo as $childModel => $junk)
		{
			$names[] = $this->getAssociationsTree($model->{$childModel});
		}
		return array($model->alias => $names);
	}

	function node_path ($node_name, $tree) {
		foreach ($tree as $name => $val) {
			if ($name == $node_name) return $name;
		        foreach ($val as $subtree) {
		                $ret = $this->node_path($node_name, $subtree);
		        	if ($ret != '') return "$name => $ret";
	        	}
	        }
	}

	function findModelPath_old2($name, $assocTree, $path = null)
	{
//		debug($name, $assocTree, $path);
//		debug($name);
//		debug($assocTree);
//		debug($path);
		foreach ($assocTree as $model => $childs)
		{
			if (!isset($path))
				$path = array($model => "");
			if ($model == $name)
			{
//				$newPath[$childModel] = $path;
//				debug($childModel);
				return array($model => $model);
//				return $model;
			}
			foreach ($childs as $childModelArray => $subTree)
			{
//				debug(array($childModel => $newAssocTree));
//				debug_print_backtrace();
//				if ($ret = $this->findModelPath($name, $subTree, $newPath))
				if ($ret = $this->findModelPath($name, $subTree))
				{
					echo "## model: "; var_dump($model);
					echo "## key subTree: "; var_dump(key($subTree));
					echo "## ret:\n"; var_dump($ret);
//					var_dump(array(key($subTree) => $ret));
//					return array(key($subTree) => $ret);
//					return array($model => key($subTree));
//					return array($model => $ret);
					return array($model => $ret);
				}
			}
		}
//		return $path;
		
	}

	function t($i)
	{
		return str_repeat("\t",$i);
	}

	function getAssociationsGraph($name, $assocTree)
	{
		static $graph = array();
		foreach ($assocTree as $model => $childs)
		{
			if ($model == $name)
				return true;
			foreach ($childs as $childModelArray)
			{
				foreach ($childModelArray as $childModel => $newAssocTree)
				{
					if ($ret = $this->getAssociationsGraph($name, array($childModel => $newAssocTree)))
					{
						$graph[$childModel][] = $model;
//						var_dump($graph);
					}
				}
			}
		}
//		var_dump($graph);
		return $graph;
	}

	function findModelPath($name, $assocTree, $path = null)
	{
//		debug($name, $assocTree, $path);
//		debug($name);
//		debug($assocTree);
//		debug($path);
		foreach ($assocTree as $model => $childs)
		{
			if (!isset($path))
				$path = array($model => "");
			foreach ($childs as $childModelArray)
			{
				foreach ($childModelArray as $childModel => $newAssocTree)
				{
//					debug(array($childModel => $newAssocTree));
//					debug_print_backtrace();
					$newPath[$childModel] = $path;
					if ($name == $childModel)
					{
//						debug($childModel);
						return $newPath;
					}
					else
					{
						if ($ret = $this->findModelPath($name, array($childModel => $newAssocTree), $newPath))
							return $ret;
					}
				}
			}
		}
//		return $path;
		
	}
	
	function printPath($model)
	{
		if (!isset($model->belongsTo))
			return null;
		$assocTree = $this->getAssociationsTree($model);
		$path = $this->findModelPath('Language', $assocTree);
//		$path = $this->findModelPath('User', $assocTree);
//		var_dump($path);
//		return 0;
		$text = null;
		while ($path)
		{
			$model = key($path);
/*			foreach ($path as $model => $childs)
			{
				$controller = Inflector::pluralize(Inflector::underscore($model));
			}
			$path = $childs;*/
			$controller = Inflector::pluralize(Inflector::underscore($model));
			$path = $path[$model];
			$new_path[$controller] = $new_path;
			var_dump($new_path);
			$text .= " => " . $controller;
		}
		return $text;
	}
	function beforeRender($controller)
	{
		if (!isset($controller->{$controller->modelClass}))
			return 0;
		$model = $controller->{$controller->modelClass};
		if (!isset($model->belongsTo))
			return 0;
		$assocTree = $this->getAssociationsTree($model);
//		var_dump($assocTree);
		$path = $this->findModelPath('Language', $assocTree);
		if (!$path)
			$path = array('Language' => array());
//		var_dump($path);
		$controller->set('assocPath', $path);

	}
}
?>