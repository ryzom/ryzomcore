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

foreach($HTTP_POST_VARS as $key => $value) {
	$GLOBALS[$key] = $value;
}

foreach($HTTP_GET_VARS as $key => $value) {
	$GLOBALS[$key] = $value;
}


/*
if(isset($HTTP_POST_VARS["command"])) { $command = $HTTP_POST_VARS["command"]; }
if(isset($HTTP_POST_VARS["admlogin"])) { $admlogin = $HTTP_POST_VARS["admlogin"]; }
if(isset($HTTP_POST_VARS["admpassword"])) { $admpassword = $HTTP_POST_VARS["admpassword"]; }
if(isset($HTTP_POST_VARS["form_refreshRate"])) { $form_refreshRate = $HTTP_POST_VARS["form_refreshRate"]; }
if(isset($HTTP_POST_VARS["display_view"])) { $display_view = $HTTP_POST_VARS["display_view"]; }
if(isset($HTTP_POST_VARS["filter_shard"])) { $filter_shard = $HTTP_POST_VARS["filter_shard"]; }
if(isset($HTTP_POST_VARS["filter_server"])) { $filter_server = $HTTP_POST_VARS["filter_server"]; }
if(isset($HTTP_POST_VARS["filter_service"])) { $filter_service = $HTTP_POST_VARS["filter_service"]; }
if(isset($HTTP_POST_VARS["filter_entity"])) { $filter_entity = $HTTP_POST_VARS["filter_entity"]; }
if(isset($HTTP_POST_VARS["current_tid"])) { $current_tid = $HTTP_POST_VARS["current_tid"]; }
if(isset($HTTP_POST_VARS["viewname"])) { $viewname = $HTTP_POST_VARS["viewname"]; }
if(isset($HTTP_POST_VARS["createview"])) { $createview = $HTTP_POST_VARS["createview"]; }
if(isset($HTTP_POST_VARS["default_view"])) { $default_view = $HTTP_POST_VARS["default_view"]; }
if(isset($HTTP_POST_VARS["execCommand"])) { $execCommand = $HTTP_POST_VARS["execCommand"]; }
if(isset($HTTP_POST_VARS["preselServ"])) { $preselServ = $HTTP_POST_VARS["preselServ"]; }
if(isset($HTTP_POST_VARS["nulogin"])) { $nulogin = $HTTP_POST_VARS["nulogin"]; }
if(isset($HTTP_POST_VARS["nugroup"])) { $nugroup = $HTTP_POST_VARS["nugroup"]; }
if(isset($HTTP_POST_VARS["nuallowedIp"])) { $nuallowedIp = $HTTP_POST_VARS["nuallowedIp"]; }
if(isset($HTTP_POST_VARS["nupassword"])) { $nupassword = $HTTP_POST_VARS["nupassword"]; }
if(isset($HTTP_POST_VARS["nuconfirmpassword"])) { $nuconfirmpassword = $HTTP_POST_VARS["nuconfirmpassword"]; }
if(isset($HTTP_POST_VARS["createUid"])) { $createUid = $HTTP_POST_VARS["createUid"]; }
if(isset($HTTP_POST_VARS["executeQuery"])) { $executeQuery = $HTTP_POST_VARS["executeQuery"]; }
if(isset($HTTP_POST_VARS["updUid"])) { $updUid = $HTTP_POST_VARS["updUid"]; }
if(isset($HTTP_POST_VARS["updVars"])) { $updVars = $HTTP_POST_VARS["updVars"]; }
if(isset($HTTP_POST_VARS["chucookie"])) { $chucookie = $HTTP_POST_VARS["chucookie"]; }
if(isset($HTTP_POST_VARS["chugroup"])) { $chugroup = $HTTP_POST_VARS["chugroup"]; }
if(isset($HTTP_POST_VARS["forcePass"])) { $forcePass = $HTTP_POST_VARS["forcePass"]; }
if(isset($HTTP_POST_VARS["forcedPass"])) { $forcedPass = $HTTP_POST_VARS["forcedPass"]; }
if(isset($HTTP_POST_VARS["allowIp"])) { $allowIp = $HTTP_POST_VARS["allowIp"]; }
if(isset($HTTP_POST_VARS["updateList"])) { $updateList = $HTTP_POST_VARS["updateList"]; }
if(isset($HTTP_POST_VARS["newServerName"])) { $newServerName = $HTTP_POST_VARS["newServerName"]; }
if(isset($HTTP_POST_VARS["updServerName"])) { $updServerName = $HTTP_POST_VARS["updServerName"]; }
if(isset($HTTP_POST_VARS["serverName"])) { $serverName = $HTTP_POST_VARS["serverName"]; }
if(isset($HTTP_POST_VARS["shardName"])) { $shardName = $HTTP_POST_VARS["shardName"]; }
if(isset($HTTP_POST_VARS["serviceName"])) { $serviceName = $HTTP_POST_VARS["serviceName"]; }
if(isset($HTTP_POST_VARS["createService"])) { $createService = $HTTP_POST_VARS["createService"]; }
if(isset($HTTP_POST_VARS["rmService"])) { $rmService = $HTTP_POST_VARS["rmService"]; }
if(isset($HTTP_POST_VARS["graphState"])) { $graphState = $HTTP_POST_VARS["graphState"]; }

if(isset($HTTP_POST_VARS["vid"])) { $vid = $HTTP_POST_VARS["vid"]; }
if(isset($HTTP_POST_VARS["nvname"])) { $nvname = $HTTP_POST_VARS["nvname"]; }
if(isset($HTTP_POST_VARS["nvpath"])) { $nvpath = $HTTP_POST_VARS["nvpath"]; }
if(isset($HTTP_POST_VARS["nvstate"])) { $nvstate = $HTTP_POST_VARS["nvstate"]; }
if(isset($HTTP_POST_VARS["nvwarning"])) { $nvwarning = $HTTP_POST_VARS["nvwarning"]; }
if(isset($HTTP_POST_VARS["nverror"])) { $nverror = $HTTP_POST_VARS["nverror"]; }
if(isset($HTTP_POST_VARS["nvorder"])) { $nvorder = $HTTP_POST_VARS["nvorder"]; }
if(isset($HTTP_POST_VARS["nvgraphupdate"])) { $nvgraphupdate = $HTTP_POST_VARS["nvgraphupdate"]; }
if(isset($HTTP_POST_VARS["nvvartype"])) { $nvvartype = $HTTP_POST_VARS["nvvartype"]; }
if(isset($HTTP_POST_VARS["createVid"])) { $createVid = $HTTP_POST_VARS["createVid"]; }
if(isset($HTTP_POST_VARS["setgroup_1"])) { $setgroup_1 = $HTTP_POST_VARS["setgroup_1"]; }
if(isset($HTTP_POST_VARS["createVarGroup"])) { $createVarGroup = $HTTP_POST_VARS["createVarGroup"]; }
if(isset($HTTP_POST_VARS["chVarGroup"])) { $chVarGroup = $HTTP_POST_VARS["chVarGroup"]; }
if(isset($HTTP_POST_VARS["varGroup"])) { $varGroup = $HTTP_POST_VARS["varGroup"]; }
if(isset($HTTP_POST_VARS["exportVarSetup"])) { $exportVarSetup = $HTTP_POST_VARS["exportVarSetup"]; }
if(isset($HTTP_POST_VARS["chVar"])) { $chVar = $HTTP_POST_VARS["chVar"]; }
if(isset($HTTP_POST_VARS["chVarName"])) { $chVarName = $HTTP_POST_VARS["chVarName"]; }
if(isset($HTTP_POST_VARS["chVarGroup"])) { $chVarGroup = $HTTP_POST_VARS["chVarGroup"]; }
if(isset($HTTP_POST_VARS["chVarPath"])) { $chVarPath = $HTTP_POST_VARS["chVarPath"]; }
if(isset($HTTP_POST_VARS["chVarState"])) { $chVarState = $HTTP_POST_VARS["chVarState"]; }
if(isset($HTTP_POST_VARS["chVarWarning"])) { $chVarWarning = $HTTP_POST_VARS["chVarWarning"]; }
if(isset($HTTP_POST_VARS["chVarError"])) { $chVarError = $HTTP_POST_VARS["chVarError"]; }
if(isset($HTTP_POST_VARS["chVarOrder"])) { $chVarOrder = $HTTP_POST_VARS["chVarOrder"]; }
if(isset($HTTP_POST_VARS["chVarGraphUpdate"])) { $chVarGraphUpdate = $HTTP_POST_VARS["chVarGraphUpdate"]; }
if(isset($HTTP_POST_VARS["chVarType"])) { $chVarType = $HTTP_POST_VARS["chVarType"]; }
if(isset($HTTP_POST_VARS["selGroup"])) { $selGroup = $HTTP_POST_VARS["selGroup"]; }
if(isset($HTTP_POST_VARS["avv_1"])) { $avv_1 = $HTTP_POST_VARS["avv_1"]; }
if(isset($HTTP_POST_VARS["aovv_1"])) { $aovv_1 = $HTTP_POST_VARS["aovv_1"]; }
if(isset($HTTP_POST_VARS["avv_2"])) { $avv_2 = $HTTP_POST_VARS["avv_2"]; }
if(isset($HTTP_POST_VARS["aovv_2"])) { $aovv_2 = $HTTP_POST_VARS["aovv_2"]; }
if(isset($HTTP_POST_VARS["rmVar"])) { $rmVar = $HTTP_POST_VARS["rmVar"]; }
if(isset($HTTP_POST_VARS["rmVarGroup"])) { $rmVarGroup = $HTTP_POST_VARS["rmVarGroup"]; }
if(isset($HTTP_POST_VARS["fshard"])) { $fshard = $HTTP_POST_VARS["fshard"]; }
if(isset($HTTP_POST_VARS["serviceId"])) { $serviceId = $HTTP_POST_VARS["serviceId"]; }
if(isset($HTTP_POST_VARS["nViewCommandName"])) { $nViewCommandName = $HTTP_POST_VARS["nViewCommandName"]; }
if(isset($HTTP_POST_VARS["nViewCommand"])) { $nViewCommand = $HTTP_POST_VARS["nViewCommand"]; }
if(isset($HTTP_POST_VARS["crViewCommand"])) { $crViewCommand = $HTTP_POST_VARS["crViewCommand"]; }
if(isset($HTTP_POST_VARS["rmViewCommand"])) { $rmViewCommand = $HTTP_POST_VARS["rmViewCommand"]; }
if(isset($HTTP_POST_VARS["viewCommand"])) { $viewCommand = $HTTP_POST_VARS["viewCommand"]; }
if(isset($HTTP_POST_VARS["chViewAutoDisplay"])) { $chViewAutoDisplay = $HTTP_POST_VARS["chViewAutoDisplay"]; }

if(isset($HTTP_GET_VARS["tid"])) { $tid = $HTTP_GET_VARS["tid"]; }
if(isset($HTTP_GET_VARS["sel_vgid"])) { $sel_vgid = $HTTP_GET_VARS["sel_vgid"]; }
if(isset($HTTP_GET_VARS["moveView"])) { $moveView = $HTTP_GET_VARS["moveView"]; }
if(isset($HTTP_GET_VARS["offs"])) { $offs = $HTTP_GETT_VARS["offs"]; }
if(isset($HTTP_GET_VARS["editUsers"])) { $editUsers = $HTTP_GET_VARS["editUsers"]; }
if(isset($HTTP_GET_VARS["editVariables"])) { $editVariables = $HTTP_GET_VARS["editVariables"]; }
if(isset($HTTP_GET_VARS["editServices"])) { $editServices = $HTTP_GET_VARS["editServices"]; }
if(isset($HTTP_GET_VARS["editServers"])) { $editServers = $HTTP_GET_VARS["editServers"]; }
if(isset($HTTP_GET_VARS["editShards"])) { $editShards = $HTTP_GET_VARS["editShards"]; }
if(isset($HTTP_GET_VARS["editUser"])) { $editUser = $HTTP_GET_VARS["editUser"]; }
if(isset($HTTP_GET_VARS["selGroup"])) { $selGroup = $HTTP_GET_VARS["selGroup"]; }
if(isset($HTTP_GET_VARS["editTid"])) { $editTid = $HTTP_GET_VARS["editTid"]; }
if(isset($HTTP_GET_VARS["confirmRmUid"])) { $confirmRmUid = $HTTP_GET_VARS["confirmRmUid"]; }
if(isset($HTTP_GET_VARS["fshard"])) { $fshard = $HTTP_GET_VARS["fshard"]; }
if(isset($HTTP_GET_VARS["varGroup"])) { $varGroup = $HTTP_GET_VARS["varGroup"]; }
if(isset($HTTP_GET_VARS["removeView"])) { $removeView = $HTTP_GET_VARS["removeView"]; }
if(isset($HTTP_GET_VARS["addToView"])) { $addToView = $HTTP_GET_VARS["addToView"]; }
if(isset($HTTP_GET_VARS["changeVidGraph"])) { $changeVidGraph = $HTTP_GET_VARS["changeVidGraph"]; }
if(isset($HTTP_GET_VARS["current_select_0"])) { $current_select_0 = $HTTP_GET_VARS["current_select_0"]; }
*/

if(isset($HTTP_COOKIE_VARS["admcookielogin"])) { $admcookielogin = $HTTP_COOKIE_VARS["admcookielogin"]; }
if(isset($HTTP_COOKIE_VARS["admcookiepassword"])) { $admcookiepassword = $HTTP_COOKIE_VARS["admcookiepassword"]; }
if(isset($HTTP_COOKIE_VARS["admfilter_shard"])) { $admfilter_shard = $HTTP_COOKIE_VARS["admfilter_shard"]; }
if(isset($HTTP_COOKIE_VARS["admfilter_server"])) { $admfilter_server = $HTTP_COOKIE_VARS["admfilter_server"]; }
if(isset($HTTP_COOKIE_VARS["admfilter_service"])) { $admfilter_service = $HTTP_COOKIE_VARS["admfilter_service"]; }
if(isset($HTTP_COOKIE_VARS["admfilter_entity"])) { $admfilter_entity = $HTTP_COOKIE_VARS["admfilter_entity"]; }

if(isset($_SESSION["sessionAuth"])) { $sessionAuth = $_SESSION["sessionAuth"]; }
?>
