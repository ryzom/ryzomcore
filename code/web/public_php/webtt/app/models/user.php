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
class User extends AppModel {
	var $name = 'User';
	var $displayField = 'name';

	var $actsAs = array('Null' => array('confirm_hash'));

	var $recursive = 0;

	var $validate = array(
		'username' => array(
			'alphaNumeric',
			'uniqueCheck' => array(
				'rule' => 'isUnique',
				'message' => 'That username has already been taken.',
			),
		),
		'email' => array('rule' => 'email', 'message' => 'Wrong format'),
		'name' => array('rule' => 'notEmpty'),
//		'password' => array('rule' => 'notEmpty'),
		'passwd' => array('rule' => 'notEmpty'),
	);

	var $scaffoldForbiddenActions = array("add", "edit", "delete");
	var $scaffoldForbiddenFields = array("" => array("activated","password","confirm_hash","created","modified"),/* "admin_" => array("password")*/);

	var $hasMany = array(
		'Translation' => array(
			'className' => 'Translation',
			'foreignKey' => 'user_id',
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
		'Vote' => array(
			'className' => 'Vote',
			'foreignKey' => 'user_id',
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
		'Comment' => array(
			'className' => 'Comment',
			'foreignKey' => 'user_id',
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

}
