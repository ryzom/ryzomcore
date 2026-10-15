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
class Identifier extends AppModel {
	var $name = 'Identifier';
	var $displayField = 'identifier';
	var $actsAs = array('Containable');

	var $validate = array(
		'identifier' => array(
			'A_Za_z0_9' => array(
				'rule' => '/[A-Za-z0-9_@]+/',
				'message' => 'Identifier must consist only of the following caracteres: "A-Z", "a-z", "0-9", "@" and "_"',
				//'allowEmpty' => false,
				//'required' => false,
				//'last' => false, // Stop validation after this rule
				//'on' => 'create', // Limit validation to 'create' or 'update' operations
			),
		),
	);

	var $scaffoldForbiddenActions = array("add", "admin_add", "edit", "admin_edit", "delete", "admin_delete");

	var $belongsTo = array(
		'TranslationFile' => array(
			'className' => 'TranslationFile',
			'foreignKey' => 'translation_file_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
	);

	var $hasOne = array(
		'BestTranslation' => array(
			'className' => 'Translation',
			'foreignKey' => 'identifier_id',
			'dependent' => false,
			'conditions' => array('BestTranslation.best' => true),
			'fields' => '',
			'order' => '',
		),
	);

	var $hasMany = array(
		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'identifier_id',
			'dependent' => true,
			'conditions' => '',
			'fields' => '',
			'order' => '',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		),
		'Comment' => array(
			'className' => 'Comment',
			'foreignKey' => 'identifier_id',
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
		'FileIdentifier' => array(
			'className' => 'FileIdentifier',
			'foreignKey' => 'identifier_id',
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
		'IdentifierColumn' => array(
			'className' => 'IdentifierColumn',
			'foreignKey' => 'identifier_id',
			'dependent' => true,
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
	
	function withoutBestTranslation($conditions = array())
	{
/*		$this->contain(array(
			'FileIdentifier' => array('ImportedTranslationFile' => array(
				'conditions' => array('ImportedTranslationFile.id' => 248)
			)),
		));

		$res = $this->find('all', array('conditions' => array('Identifier.id' => array(125219, 131609, 67133))));*/
//		$fileIdentifier_ids = $this->FileIdentifier->find('list', array('fields' => array('FileIdentifier.id', 'FileIdentifier.id'), 'conditions' => array('FileIdentifier.imported_translation_file_id' => 248)));
		// TOTHINK: try to achieve that with Linkable behaviour
		if (isset($conditions['ImportedTranslationFile.id']))
			return $identifier_ids = $this->FileIdentifier->find('list', array('fields' => array('Identifier.id', 'Identifier.id'), 'conditions' => array('FileIdentifier.imported_translation_file_id' => $conditions['ImportedTranslationFile.id']), 'recursive' => 1));
		else
			return false;
	}
	
	function getNeighbours($id)
	{
		$identifierNeighbours['current'][] = $this->read(null, $id);
		if ($identifierNeighbours['current'])
		{
			$identifierNeighbours['prev'] = $this->find('all', array('order' => 'Identifier.id DESC', 'limit' => 5, 'conditions' => array('Identifier.translation_file_id' => $identifierNeighbours['current'][0]['Identifier']['translation_file_id'], 'Identifier.id <' => $identifierNeighbours['current'][0]['Identifier']['id'])));
			$identifierNeighbours['next'] = $this->find('all', array('order' => 'Identifier.id ASC', 'limit' => 5, 'conditions' => array('Identifier.translation_file_id' => $identifierNeighbours['current'][0]['Identifier']['translation_file_id'], 'Identifier.id >' => $identifierNeighbours['current'][0]['Identifier']['id'])));
		}
		return $identifierNeighbours;
	}
}
