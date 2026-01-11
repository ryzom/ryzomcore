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
class Translation extends AppModel {
	var $name = 'Translation';
	var $displayField = 'translation_text';

	var $scaffoldForbiddenActions = array();
	var $scaffoldActions = array("add" => "fk", "index" => "fk");

	var $belongsTo = array(
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
		'IdentifierColumn' => array(
			'className' => 'IdentifierColumn',
			'foreignKey' => 'identifier_column_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
		'User' => array(
			'className' => 'User',
			'foreignKey' => 'user_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
		'ParentTranslation' => array(
			'className' => 'Translation',
			'foreignKey' => 'parent_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
	);

	var $hasMany = array(
		'Vote' => array(
			'className' => 'Vote',
			'foreignKey' => 'translation_id',
			'dependent' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),
/*		'Comment' => array(
			'className' => 'Comment',
			'foreignKey' => 'translation_id',
			'dependent' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),*/
		'ChildTranslation' => array(
			'className' => 'Translation',
			'foreignKey' => 'parent_id',
			'dependent' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),
	);

	function setBest()
	{
		if (!$this->id)
			return false;
		$this->read();
		if (!isset($this->data['Translation']['identifier_id']))
			return false;

		// set best on chosen translation
		$ret = $this->save(array('best' => 1));
		$best_id = $this->id;

		// reset best on other translations
		$ret = $this->updateAll(array('Translation.best' => 0), array(
			'AND' => array(
				'Translation.identifier_id' => $ret['Translation']['identifier_id'],
				'Translation.id !=' => $best_id,
			),
		));
		$this->log($ret);
		
		return $this->id;
	}
	
	function makeHash($ent)
	{
		if (isset($ent['columns']) && is_array($ent['columns']))
		{
			sort($ent['columns']);
			return md5(serialize($ent['columns']));
		}
		else if (isset($ent['string']))
			return md5($ent['string']);
	}
}
