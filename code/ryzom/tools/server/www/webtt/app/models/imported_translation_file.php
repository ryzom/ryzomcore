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
class ImportedTranslationFile extends AppModel {
	var $name = 'ImportedTranslationFile';
	var $displayField = 'filename';

	var $scaffoldForbiddenActions = array("index", "add", "admin_add", "edit", "admin_edit", "delete", "admin_delete");

	var $actsAs = array('Containable');

	var $belongsTo = array(
		'TranslationFile' => array(
			'className' => 'TranslationFile',
			'foreignKey' => 'translation_file_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),
	);

	var $hasMany = array(
		'FileIdentifier' => array(
			'className' => 'FileIdentifier',
			'foreignKey' => 'imported_translation_file_id',
			'dependent' => true,
			'conditions' => '',
			'fields' => '',
			'order' => 'FileIdentifier.id',
			'limit' => '',
			'offset' => '',
			'exclusive' => '',
			'finderQuery' => '',
			'counterQuery' => ''
		)
	);

	var $hasOne = array(
		'RawFile' => array(
			'className' => 'RawFile',
			'foreignKey' => 'filename',
			'dependand' => false,
			'conditions' => '',
			'fields' => '',
			'order' => '',
		),
	);

}
