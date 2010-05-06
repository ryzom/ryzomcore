<?php
// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

        session_start();
	include('foo.php');
        $publicAccess = true;
        // set cookies for filters
        if (isset($admfilter_shard) && !isset($filter_shard))                   $filter_shard = $admfilter_shard;
        else if (isset($filter_shard) && $filter_shard=="")                             setCookie("admfilter_shard");
        else if (isset($filter_shard))                                                                  setCookie("admfilter_shard", $filter_shard, time()+3600*24*15);

        if (isset($admfilter_server) && !isset($filter_server))                 $filter_server = $admfilter_server;
        else if (isset($filter_server) && $filter_server=="")                   setCookie("admfilter_server");
        else if (isset($filter_server))                                                                 setCookie("admfilter_server", $filter_server, time()+3600*24*15);

        if (isset($admfilter_service) && !isset($filter_service))               $filter_service = $admfilter_service;
        else if (isset($filter_service) && $filter_service=="")                 setCookie("admfilter_service");
        else if (isset($filter_service))                                                                setCookie("admfilter_service", $filter_service, time()+3600*24*15);

        if (isset($admfilter_entity) && !isset($filter_entity))                 $filter_entity = $admfilter_entity;
        else if (isset($filter_entity) && $filter_entity=="")                   setCookie("admfilter_entity");
        else if (isset($filter_entity))                                                                 setCookie("admfilter_entity", $filter_entity, time()+3600*24*15);


	include('sql_connection.php');
	include('session_auth.php');
	include('login_form.php');
	include('html_headers.php');

	$result = defaultConnectToDatabase();
	if ($result)
		die($result);


	if(!auth($error))
	{
		LoginForm();
		die();
	}
	
	$IsNevrax = (strtolower($group) == 'nevraxgroup');

	if ((!isset($publicAccess) || $publicAccess == false) && $admlogin != "root" && (!$allowNevrax || !$IsNevrax))
	{
		htmlProlog($_SERVER['PHP_SELF'], "Acces not granted");
		echo "You are not allowed to go to this page.<br>\n";
		echo "<a href='index.php'>Index page</a>\n";
		htmlEpilog();
		die;
	}

	include('init.php');

//        print "POST VARS: "; print_r($HTTP_POST_VARS); print "<br><br>";
//        print "GET VARS: "; print_r($HTTP_GET_VARS); print "<br><br>";
//        print "COOKIE VARS "; print_r($HTTP_COOKIE_VARS); print "<br><br>";
//        print "SESSION VARS "; print_r($_SESSION); print "<br><br>";

?>
