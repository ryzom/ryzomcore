<?php
/*
	Ryzom Core Web-Based Translation Tool
	Copyright (C) 2011 Piotr Kaczmarek <p.kaczmarek@openlink.pl>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
?>
<?php
class PathResolverComponent extends Object {
	function getAssociationsTree($model)
	{
		$names = array();
		foreach ($model->belongsTo as $childModel => $junk)
		{
			if ($model->alias != $model->{$childModel}->alias)
				$names[] = $this->getAssociationsTree($model->{$childModel});
		}
		return array($model->alias => $names);
	}

	function t($i)
	{
		return str_repeat("\t",$i);
	}

	function findModelPath($name, $assocTree, $path = null)
	{
		foreach ($assocTree as $model => $childs)
		{
			if (!isset($path))
				$path = array($model => "");
			foreach ($childs as $childModelArray)
			{
				foreach ($childModelArray as $childModel => $newAssocTree)
				{
					$newPath[$childModel] = $path;
					if ($name == $childModel)
					{
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
		
	}

	function beforeRender($controller)
	{
		if (!isset($controller->{$controller->modelClass}))
			return 0;
		$model = $controller->{$controller->modelClass};
		if (!isset($model->belongsTo))
			return 0;
		$assocTree = $this->getAssociationsTree($model);
		$rootModel = 'Language';
		$path = $this->findModelPath($rootModel, $assocTree);
		if (!$path && $model->alias == $rootModel)
			$path = array($rootModel => array());
		$controller->set('assocPath', $path);

	}
}
?>