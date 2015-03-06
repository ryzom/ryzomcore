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
class FileIdentifier extends AppModel {
	var $name = 'FileIdentifier';
	var $displayField = 'command';
	var $order = 'FileIdentifier.id';

	var $scaffoldForbiddenActions = array("index", "add", "admin_add", "edit", "admin_edit", "delete", "admin_delete");

	var $belongsTo = array(
		'ImportedTranslationFile' => array(
			'className' => 'ImportedTranslationFile',
			'foreignKey' => 'imported_translation_file_id',
			'conditions' => '',
			'fields' => '',
//			'order' => ''
		),
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
			'conditions' => '',
			'fields' => '',
//			'order' => ''
		)
	);
}
