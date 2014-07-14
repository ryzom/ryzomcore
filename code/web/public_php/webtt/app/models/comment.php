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
class Comment extends AppModel {
	var $name = 'Comment';
	var $displayField = 'id';

	var $scaffoldActions = array("add" => "fk");

	var $belongsTo = array(
/*		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'translation_id',
			'conditions' => '',
			'fields' => '',
			'order' => ''
		),*/
		'Identifier' => array(
			'className' => 'Identifier',
			'foreignKey' => 'identifier_id',
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
		)
	);
}
