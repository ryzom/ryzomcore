<?php
/* Copyright (C) 2012 Winch Gate Property Limited
 * 
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

// init database table used by webig
$db = ryDB::getInstance('webig');
$db->setDbDefs('players', array('id' => SQL_DEF_INT, 'cid' => SQL_DEF_INT, 'name' => SQL_DEF_TEXT, 'gender' => SQL_DEF_INT, 'creation_date' => SQL_DEF_DATE, 'deleted' => SQL_DEF_BOOLEAN, 'last_login' => SQL_DEF_TEXT, 'dev_shard' => SQL_DEF_BOOLEAN));
$db->setDbDefs('accounts', array('uid' => SQL_DEF_INT, 'web_privs' => SQL_DEF_TEXT));

?>