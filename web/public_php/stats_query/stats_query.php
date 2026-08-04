<?php

// Ryzom Core - MMORPG Framework <http://ryzom.dev/>
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

	include_once('../login/config.php');
	
	// LOG database
	$StatsDBHost =  $DBHost;
	$StatsDBUserName = $DBUserName;
	$StatsDBPassword = $DBPassword;
	$StatsDBName = "stats";

	error_reporting(E_ERROR | E_PARSE);
	set_error_handler('err_callback');

	// for error handling, buffer all output

	$link = NULL;
	$page_name="stats_query.php";
	$page_max = 50;
	// Access gate: REMOTE_ADDR must match $StatsPrivateNetwork (from
	// login/config.php / config_user.php), or the request must carry a
	// matching $StatsQuerySecret. Default network is the historical
	// 192.168.1.* allowlist — override it on real deployments.
	global $StatsPrivateNetwork, $StatsQuerySecret;
	$private_network = (isset($StatsPrivateNetwork) && $StatsPrivateNetwork !== '')
		? $StatsPrivateNetwork
		: '/^192\\.168\\.1\\./';
	$stats_query_secret = isset($StatsQuerySecret) ? (string)$StatsQuerySecret : '';


	function toHMS($time)
	{
		$ret = "";
		if ($time <= 0) { return "0 s";}
		if ($time > 60*60*24)
		{
			$days = floor($time / (60*60*24));
			$time = floor($time - $days * 60*60*24);			
			if ($days != 0)	{
				$ret = $ret . $days . "d ";
			}
		}

		if ($time > 60*60)
		{
			$hours = floor($time / (60*60));
			$time = floor($time - $hours * 60*60);			
			if ($hours != 0)	{
				$ret = $ret . $hours . "h ";
			}
		}

		if ($time > 60)
		{
			$mins = floor($time / 60);
			$time = floor($time - $mins * 60);			
			if ($mins != 0)	{
				$ret = $ret . $mins . "m ";
			}
		}
		if ($time != 0)	{
			$ret = $ret . $time . "s ";
		}
		return $ret;
	}

	// Peer address only. HTTP_CLIENT_IP / X-Forwarded-For are set by the
	// caller, so trusting them would let anyone claim a private address and
	// open the detailed error path and the whole view.
	function getIp()
	{
		if (!empty($_SERVER['REMOTE_ADDR']))
			return $_SERVER['REMOTE_ADDR'];
		$ip = getenv("REMOTE_ADDR");
		return ($ip !== false && $ip !== '') ? $ip : '';
	}


	// Detailed SQL errors only for addresses that would also get the view.
	function stats_query_authorized()
	{
		global $private_network, $stats_query_secret;
		if ($stats_query_secret !== '')
		{
			$token = '';
			if (isset($_GET['token']) && is_string($_GET['token']))
				$token = $_GET['token'];
			elseif (isset($_POST['token']) && is_string($_POST['token']))
				$token = $_POST['token'];
			elseif (isset($_SERVER['HTTP_X_STATS_TOKEN']) && is_string($_SERVER['HTTP_X_STATS_TOKEN']))
				$token = $_SERVER['HTTP_X_STATS_TOKEN'];
			if ($token !== '' && hash_equals($stats_query_secret, $token))
				return true;
		}
		$ip = getIp();
		if ($private_network !== '' && @preg_match($private_network, $ip))
			return true;
		return false;
	}

	// if the player ip is the dev ip then the sql error is explain
	function die2($debug_str = "")
	{
		// the failing query and the mysql error text are only for us
		if ( stats_query_authorized() )
		{
			die($debug_str);
		}
		else
		{
			die("GENERIC_ERROR");
		}
	}

	// get head or post infos return default if no valuees
	function getPost($value, $default=NULL)
	{
		if ( isSet($_GET[$value]) ) { return $_GET[$value]; }
		if ( isSet($_POST[$value]) ) { return $_POST[$value]; }
		return $default;
	}

	$cmd = getPost("cmd", "view");

	// all of these end up in the urls this page builds and in the date
	// arithmetic below, so keep them numeric
	$show_dl=(int)getPost("show_dl", "1");
	$show_ddl=(int)getPost("show_ddl", "1");
	$show_du=(int)getPost("show_du", "1");
	$show_hdu=(int)getPost("show_hdu", "1");
	$show_hdu2=(int)getPost("show_hdu2", "1");
	$show_hddetails=(int)getPost("show_hddetails", "0");

	function getHref()
	{

		global $show_dl;
		global $show_ddl;
		global $show_du;
		global $show_hdu;
		global $show_hdu2;
		global $selected;
		global $page_name;
		global $display;
		global $show_hddetails;
		return "$page_name?cmd=view&show_dl=$show_dl&show_ddl=$show_ddl&show_du=$show_du&show_hdu=$show_hdu&show_hdu2=$show_hdu2&selected=$selected&display=$display&show_hddetails=$show_hddetails";
	}


	// do a simple sql query return the result of the query
	function getSimpleQueryResult($query, &$link)
	{
		$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: $query<br/>".mysqli_error($link)."<br/>  ");
		
		if ( ($row = mysqli_fetch_array($result)) )
		{
			return $row["ret"];

		}
		return "";
	}

	// log error in bdd
	function debug($str)
	{
		global $StatsDBHost;
		global $DBPort;
		global $StatsDBUserName;
		global $StatsDBPassword;
		global $StatsDBName;
		global $link;

		$newConnection = 0;

		if ($link == NULL)
		{
			$link = mysqli_connect($StatsDBHost, $StatsDBUserName, $StatsDBPassword, NULL, $DBPort) or die2 (__FILE__. " " .__LINE__." can't connect to database host:$StatsDBHost user:$StatsDBUserName");
			if (function_exists('nel_mysqli_set_charset'))
				nel_mysqli_set_charset($link);
			$newConnection = 1;

			mysqli_select_db ($link, $StatsDBName) or die2 (__FILE__. " " .__LINE__." can't access to the table dbname:$StatsDBName");
		}


		// Strip-quotes was not enough (backslash, multibyte, etc.); escape
		// the same way stats.php already does for the same table.
		$str = mysqli_real_escape_string($link, $str);

		$query= "INSERT INTO `log` ( `log` )"
			. "VALUES ("
			. "'$str'"
			. ")";

		$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
		if ($newConnection == 1)
		{
			mysqli_close($link);
			$link = NULL;
		}

	}
	// $vars optional: php 8 calls error handlers with four arguments
	function err_callback($errno, $errmsg, $filename, $linenum, $vars = null)
	{
		// Do not echo PHP errors to the client; they go to the log table.
		debug("$filename $linenum $errmsg");
	}

	//extract infos FROM sessions
	function getStats($mode, $day, &$link)
	{

		$day2 = date('Y-m-d', $day);
		$day_first = date('Y-m-d H:i:s', strtotime( date('Y-m-d', $day) ) );
		$day_last = date('Y-m-d H:i:s', strtotime(date('Y-m-d', $day + 3600*24)) - 1);
		$condition1 = "install_users.first_install >= '$day_first' AND install_users.first_install <= '$day_last' ";
		$condition2 = "`sessions`.`start_download` >= '$day_first' AND `sessions`.`start_download` <= '$day_last' ";

		if (!isSet($condition))
		{
			$condition = "1";
		}
		$ret = array();

		if ($mode == 2)
		{

			// false = not optimized
			if (true)
			{
				//true == optimized: Request is optimized we use one query instead of many
				$query = "SELECT "
					."SUM(IF(install_users.state = 'DU_P3', 1, 0)) AS DU_P3, " 
					."SUM(IF(install_users.state = 'DU_P2', 1, 0)) AS DU_P2, "
					."SUM(IF(install_users.state = 'DU_P1', 1, 0)) AS DU_P1, "
					."SUM(IF(install_users.state = 'DU_AG', 1, 0)) AS DU_AG, "
					."SUM(IF(install_users.state = 'DU_CS', 1, 0)) AS DU_CS, "
					."SUM(IF(install_users.state IN ('DU_AL', 'DU_PL' ), 1, 0)) AS DU_AL, "
					."SUM(IF(install_users.state IN ('DU_FI', 'DU_VMS', 'DU_VMSHS' ), 1, 0)) AS DU_FI," 
					."SUM(IF(install_users.state = 'DU_IN', 1, 0)) AS DU_IN, "
					."SUM(IF(install_users.state = 'DU_DL', 1, 0)) AS DU_DL "
					."FROM install_users "
					."WHERE $condition1 "
					."AND install_users.user_id "
					."IN ( SELECT sessions.user_id FROM sessions WHERE $condition2 )";

				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
				if ( ($row = mysqli_fetch_array($result)) ) 
				{ 

					$ret["DU_P3"] = $row["DU_P3"];
					$ret["DU_P2"] = $row["DU_P2"] + $ret["DU_P3"];
					$ret["DU_P1"] = $row["DU_P1"] + $ret["DU_P2"];
					$ret["DU_AG"] = $row["DU_AG"] + $ret["DU_P1"];
					$ret["DU_CS"] = $row["DU_CS"] + $ret["DU_AG"];
					$ret["DU_AL"] = $row["DU_AL"] + $ret["DU_CS"];
					$ret["DU_FI"] = $row["DU_FI"] + $ret["DU_AL"];
					$ret["DU_IN"] = $row["DU_IN"] + $ret["DU_FI"];
					$ret["DU_DL"] = $row["DU_DL"] + $ret["DU_IN"];
				}
				
			} else {
		
				$ret["DU_P3"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`='DU_P3'", $link) ;
				$ret["DU_P2"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`='DU_P2'", $link) + $ret['DU_P3'];
				$ret["DU_P1"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`='DU_P1'", $link) + $ret['DU_P2'];
				$ret["DU_AG"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`='DU_AG'", $link) + $ret['DU_P1'];
				$ret["DU_CS"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`= 'DU_CS'", $link) + $ret['DU_AG'];
				$ret["DU_AL"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state` IN ('DU_AL', 'DU_PL' ) ", $link) + $ret['DU_CS'];
				$ret["DU_FI"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state` IN ('DU_FI', 'DU_VMS', 'DU_VMSHS' )", $link) + $ret['DU_AL'];
				$ret["DU_IN"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`='DU_IN'", $link) + $ret['DU_FI'];
				$ret["DU_DL"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(`sessions`.`user_id`)) AS ret FROM `sessions`, `install_users` WHERE $condition1 AND `install_users`.`user_id`=`sessions`.`user_id` AND `install_users`.`state`='DU_DL'", $link) + $ret['DU_IN'];
			}
		}		

		$ret["all_session"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2", $link);

		if ($mode == 1)
		{

			$ret["distinct_user"] = getSimpleQueryResult("SELECT COUNT(DISTINCT(sessions.user_id)) AS ret FROM `sessions` WHERE $condition2 ORDER BY sessions.user_id", $link);
			//true == optimized: Request is optimized we use one query instead of many
			if (true)
			{
				$query = "SELECT"
				. " COUNT(session_id) AS all_session,"
				. " SUM(IF(sessions.previous_download IN ('0', '0.00B', '0.000B'), 1, 0)) AS clean_start,"
				. " SUM(IF(sessions.percent_download = '100', 1, 0)) AS download_finished,"
				. " SUM(IF(sessions.percent_install = '100', 1, 0)) AS install_finished,"
				. " SUM(IF(sessions.package = 'full', 1, 0)) AS download_full,"
				. " SUM(IF(sessions.lang = 'fr', 1, 0)) AS fr,"
				. " SUM(IF(sessions.lang = 'en', 1, 0)) AS en,"
				. " SUM(IF(sessions.lang = 'de', 1, 0)) AS de"
				. " FROM sessions"
				. " WHERE $condition2";

				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
				if ( ($row = mysqli_fetch_array($result) ) )
				{
					$ret["all_session"] = $row["all_session"];
					$ret["clean_start"] = $row["clean_start"];
					$ret["download_finished"] = $row["download_finished"];
					$ret["install_finished"] = $row["install_finished"];
					$ret["download_full"] = $row["download_full"];
					$ret["fr"] = $row["fr"];
					$ret["en"] = $row["en"];
					$ret["de"] = $row["de"];
				}	


			} else {
				$ret["clean_start"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND previous_download IN ('0', '0.000b', '0.00B')", $link);
				$ret["download_finished"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND `percent_download` = '100'", $link);
				$ret["install_finished"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND `percent_install` = '100' ", $link);
				$ret["download_full"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND `package` = 'full'", $link);
				$ret["fr"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND `lang` = 'fr'", $link);
				$ret["en"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND `lang` = 'en'", $link);
				$ret["de"] = getSimpleQueryResult("SELECT COUNT(`session_id`) AS ret FROM `sessions` WHERE $condition2 AND `lang` = 'de'", $link);
			}
			$ret["restart"] = $ret['all_session'] - $ret['clean_start'];

		}
		return $ret;

	}



	// display download infos
	// condition is the condition for displaying users (time condition)
	// max_reslut is the number of result that could be displayed
	function displayDownload($link)
	{
		global $show_dl;
		global $selected;

		$date = getdate($selected);
		$date_first = date('Y-m-d H:i:s', strtotime( date('Y-m-1', $selected) ) );
		$date_first_int = strtotime($date_first);
		$date_last = date('Y-m-d H:i:s', strtotime(date('Y-m-1', $selected + 31*24*3600)) - 1);
		$date_last_int = strtotime($date_last);
		$nbday = round( ($date_last_int +1 - $date_first_int ) / (24*3600));

		//display sumary infos (if title clicked make apears or disapears the menu)
		$old_value = $show_dl;
		$show_dl = !$show_dl;
		echo '<h2><a href="'.getHref().'">downloads</a></h2>';
		$show_dl = $old_value;
		
		if ($show_dl == 1)
		{	


			echo '<table summary="texte">'."\n";
			echo "<tr>\n"
			.	"<th>week day</th>\n"
			.	"<th>".$date['month']. " ".$date['year']."</th>\n"
			.	"<th>distinct users(today)</th>\n"
			.	"<th>first start</th>\n"
			.	"<th>restart</th>\n"
			.	"<th>total download</th>\n"
			.	"<th>download finished</th>\n"
			.	"<th>install finished</th>\n"
			.	"<th>full version</th>\n"
			.	"<th>fr</th>\n"
			.	"<th>en</th>\n"
			.	"<th>de</th>\n"
			.	"</tr>\n";


			// for each day of the month with download display infos
			for ($i = 0; $i < $nbday; $i++)
			{
				$day = $date_first_int + $i*24*3600;

				if ($day >= time())// + 24*3600)
				{
					break;
				}
				$day_first = date('Y-m-d H:i:s', strtotime( date('Y-m-d', $day) ) );
				$day_last = date('Y-m-d H:i:s', strtotime(date('Y-m-d', $day + 3600*24)) - 1);

				$res = getStats(1, $day, $link);
				$date_today = getdate($day);
				// display only if active download
				if ($res['all_session'] != 0 )
				{

					echo "<tr>\n"

					. "<td>".$date_today['weekday']."</td>";


					if (date('Y-m-d', $day)==date('Y-m-d', $selected))
					{
						echo "<td>".($i+1)."</td>";
					}
					else
					{
						 echo "<td><a href=\"".getHref()."&selected=$day\">".($i+1)."</a></td>";
					}

					echo "<td>".$res['distinct_user']."</td>"
					//. "<td>".$res['all_session']."</td>"
					. "<td>".$res['clean_start']."</td>"
					. "<td>".$res['restart']."</td>"
					. "<td>".($res['restart'] + $res['clean_start'])." (". $res['clean_start']. "+" . $res['restart'] .")</td>"

					. "<td>".$res['download_finished']."</td>"
					. "<td>".$res['install_finished']."</td>"
					. "<td>".$res['download_full']."</td>"
					. "<td>".$res['fr']."</td>"
					. "<td>".$res['en']."</td>"
					. "<td>".$res['de']."</td>"
					. "</tr>\n";
				}

			}
			echo "</table>";

			//display next mont, prev month link
			echo '<a href="'.getHref().'&page_users=1&page_users2=1&page_download=1&selected='.strtotime(date('Y-m-1', $selected-1)).'">'." < last month". '</a> ';
			echo ", ";
			echo '<a href="'.getHref().'&page_users=1&page_users2=1&page_download=1&selected='.strtotime(date('Y-m-1', $selected+31*24*3600)).'">'."next month >". '</a> ';
		}
	}

	function displayDistinctUsers($link)		
	{
		global $show_du;
		global $selected;

		$date = getdate($selected);
		$date_first = date('Y-m-d H:i:s', strtotime( date('Y-m-1', $selected) ) );
		$date_first_int = strtotime($date_first);
		$date_last = date('Y-m-d H:i:s', strtotime(date('Y-m-1', $selected + 31*24*3600)) - 1);
		$date_last_int = strtotime($date_last);
		$nbday = round( ($date_last_int +1 - $date_first_int ) / (24*3600));

		// when click on title, the content apears or disapers
		$old_value = $show_du;
		$show_du = !$show_du;
		echo '<h2><a href="'.getHref().'">distinct new users</a></h2>';
		$show_du = $old_value;

		if ($show_du == 1)
		{
			echo '<table summary="texte">'."\n";
			echo "<tr>\n"
				.	"<th>week day</th>\n"
				.	"<th>".$date['month']. " ".$date['year']."</th>\n"
				.	"<th>started dl</th>\n"
				.	"<th>finished dl</th>\n"
				.	"<th>finished inst</th>\n"
				.	"<th>arrive login</th>\n"
				.	"<th>arrive char sel</th>\n"
				.	"<th>arrive in game</th>\n"
				.	"<th>play 30 min</th>\n"
				.	"<th>play 1hr</th>\n"
				.	"<th>play 2hr</th>\n"
				.	"</tr>\n";
			for ($i = 0; $i < $nbday; $i++)
			{

				$day = $date_first_int + $i*24*3600;
				if ($day >= time())// + 24*3600)
				{
					break;
				}
//				$row = getStats("`sessions`.`start_download` >= '$day_first' AND `sessions`.`start_download` <= '$day_last' ", $link);
				$row = getStats(2, $day, $link);

				$date_today = getdate($day);
				if ($row['all_session'] != 0)
				{
					echo "<tr>";

					echo "<td>".$date_today['weekday']."</td>";
					if (date('Y-m-d', $day)==date('Y-m-d', $selected))
					{
						echo "<td>".($i+1)."</td>";
					}
					else
					{
						 echo "<td><a href=\"".getHref()."&selected=$day\">".($i+1)."</a></td>";
					}

					echo "<td>".htmlspecialchars($row["DU_DL"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_IN"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_FI"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_AL"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_CS"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_AG"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_P1"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_P2"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["DU_P3"], ENT_QUOTES)."</td>\n";
					echo "</tr>\n";
				}

			 }

			echo "</table>\n";
			echo '<a href="'.getHref().'&page_users=1&page_users2=1&page_download=1&selected='.strtotime(date('Y-m-1', $selected-1)).'">'." < last month". '</a> ';
			echo ", ";
			echo '<a href="'.getHref().'&page_users=1&page_users2=1&page_download=1&selected='.strtotime(date('Y-m-1', $selected+31*24*3600)).'">'."next month >". '</a> ';

		}
	}

	function displayDownloadDetails($condition, $link)
	{
		global $page_max;
		global $show_ddl;
		global $page_download;
		global $page_users;
		global $page_users2;
		global $selected;


		$query = "SELECT COUNT(`session_id`) AS max"
		.	" FROM `sessions`"
		.	" WHERE $condition";
		$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
		$max_result = 0;
		if ( ($row = mysqli_fetch_array($result)) ) { $max_result = $row["max"];}

		//display sumary infos (if title clicked make apears or disapears the menu)
		$old_value = $show_ddl;
		$show_ddl = !$show_ddl;
		echo '<h2><a href="'.getHref().'">download details('.$max_result.')</a></h2>';
		$show_ddl = $old_value;
		if ($show_ddl == 1)
		{
			echo '<table summary="texte">'."\n";
			echo "<tr>\n"
			.	"<th>user_id</th>\n"
			.	"<th>ip</th>\n"
			.	"<th>lang</th>\n"
			.	"<th>sku</th>\n"
			.	"<th>protocol</th>\n"
			.	"<th>sz dl</th>\n"
			.	"<th>sz inst</th>\n"
			.	"<th>start</th>\n"
			.	"<th>start inst</th>\n"
			.	"<th>finish</th>\n"
			.	"<th>time</th>\n"
			.	"<th>% dl</th>\n"
			.	"<th>% inst</th>\n"
			.	"<th>prev dl</th>\n"
			.	"</tr>\n";

			$query = "SELECT COUNT(`session_id`) AS max FROM `sessions` WHERE $condition";
			$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
			$max_result=0;
			if ( ($row = mysqli_fetch_array($result)) ) { $max_result = $row["max"]; }



			$query = "SELECT `session_id` , `user_id` , `server` , `application` , `version` , `ip` , `lang` , `type` , `package` , `protocol` , `size_download` , `size_install` , `start_download` , `stop_download` , `start_install` , `stop_install` , `percent_download` , `percent_install` , `previous_download`"
			 .	" FROM `sessions`"
			 .	" WHERE $condition"
			 .	" ORDER by `user_id` desc, `start_download` desc"
			 .	" LIMIT ".($page_download * $page_max)." , ".$page_max;
			$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);

			while ( ($row = mysqli_fetch_array($result)) )
			{
				echo "<tr>";
				echo "<td>"
				.$row["user_id"]
				."</td>\n";
				echo "<td>".htmlspecialchars($row["ip"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["lang"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["package"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["protocol"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["size_download"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["size_install"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["start_download"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["start_install"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["stop_install"], ENT_QUOTES)."</td>\n";
				echo "<td>" .	toHMS(strtotime(htmlspecialchars($row["stop_download"], ENT_QUOTES)) - strtotime(htmlspecialchars($row["start_download"], ENT_QUOTES)) ) ."</td>\n";
				echo "<td>".htmlspecialchars($row["percent_download"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["percent_install"], ENT_QUOTES)."</td>\n";
				echo "<td>".htmlspecialchars($row["previous_download"], ENT_QUOTES)."</td>\n";
				echo "</tr>\n";
			}
			echo "</table>\n";

			for ($i = 1 ;$i < ($max_result / $page_max)+1; $i = $i+1)
			{
				if ($page_download + 1 == $i)
				{
					echo ($i)." ";
				}
				else
				{
					echo '<a href="'.getHref().'&page_users='.($page_users+1).'&page_users2='.($page_users2+1).'&page_download='.$i.'&selected='.$selected.'">'.($i). '</a> ';
				}
			}

		}
	}

	function lastSession($id, $condition, $link)
	{
		$query = "SELECT install_users.user_id, sessions.session_id, sessions.stop_download, sessions.start_download, sessions.percent_download, sessions.percent_install "
			.	" FROM install_users, sessions "
			.	" WHERE install_users.user_id='" . $id . "' AND install_users.user_id = sessions.user_id AND $condition"
			.	" ORDER BY sessions.session_id DESC";

		$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
		if ( ($row = mysqli_fetch_array($result)) )
		{
			$ret["session_id"] = $row["session_id"];
			$ret["time"] = strtotime($row["stop_download"]) -  strtotime($row["start_download"]) ;
			$ret["percent"] = $row["percent_download"] . "/". $row["percent_install"];
			return $ret;
		}
		$ret["time"] = "?";
		$ret["percent"] = "?";
		return $ret;
	}

		       

	// display infos on user hardware
	// condition is the condition for displaying users (time condition)
	function displayHardwareNewUserImpl($install_state, $title, $condition, $condition2,  $link)
	{

		global $page_max;
		global $page_users;
		global $page_users2;
		global $selected;
		global $show_hdu;
		global $show_hdu2;
		global $show_hddetails;

	
//		$query = "SELECT COUNT(install_users.user_id) AS max FROM install_users WHERE $condition2 AND install_users.user_id in (SELECT DISTINCT(sessions.user_id) from sessions where $condition)";
		$query = "SELECT COUNT(DISTINCT(sessions.user_id)) AS max FROM  `install_users`,`sessions` WHERE $condition2 AND install_users.user_id = sessions.user_id AND $condition";
		$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);
		$max_result = 0;
		if ( ($row = mysqli_fetch_array($result)) ) { $max_result = $row["max"];}

		if ($install_state==1)
		{
			$old_value = $show_hdu;
			$show_hdu = 1-$show_hdu;		
			echo '<h2><a href="'.getHref().'">Hardware users - Unfinished install ('.$max_result.')</a></h2>';
			$show_hdu = $old_value;
			$nbpages = $page_users;
		}
		else
		{
			$old_value = $show_hdu2;
			$show_hdu2 = 1-$show_hdu2;		
			echo '<h2><a href="'.getHref().'">Hardware users - Finished install ('.$max_result.')</a></h2>';
			$show_hdu2 = $old_value;
			$nbpages = $page_users2;
		}

		// Show / Hide os column	
		{	
			$old_value = $show_hddetails;
			$show_hddetails = 1 - $show_hddetails;
			if ($show_hddetails == 1)
			{
				echo '<h3><a href="'.getHref().'">Show details</a></h3>';
			}
			else
			{
				echo '<h3><a href="'.getHref().'">Hide details</a></h3>';
			}
			$show_hddetails = $old_value;
		}


	//display sumary infos (if title clicked make apears or disapears the menu)

		if (($install_state==1 && $show_hdu == 1) || ($install_state==2 && $show_hdu2 == 1) )
		{


			echo '<table summary="texte">'."\n";
			echo "<tr>\n"
				.	"<th>user_id</th>\n";
			if ($show_hddetails == 0)
			{

			echo	"<th>first_install</th>\n"
			.	"<th>last_install</th>\n"
			.	"<th>install_count</th>\n"
			.	"<th>install time</th>\n"
			.	"<th>last install time</th>\n";
			}
			if ($install_state) { echo "<th>last install state</th>\n"; }
			echo	"<th>memory</th>\n";
			if ($show_hddetails == 1)
			{
				echo	"<th>os</th>\n"
				.	"<th>proc</th>\n"

				.	"<th>video_card</th>\n"
				.	"<th>driver_version</th>\n";
			}

			echo	"<th>current state</th>\n"
			.	"</tr>\n";

			$query = "SELECT `install_users`.`user_id` , `install_users`.`first_install`, `install_users`.`last_install`, `install_users`.`install_count`, `install_users`.`os` , `install_users`.`proc` , `install_users`.`memory` , `install_users`.`video_card` , `install_users`.`driver_version`, `install_users`.`state`, "
				."SUM( IF(sessions.stop_download >sessions.start_download,  UNIX_TIMESTAMP(sessions.stop_download) - UNIX_TIMESTAMP(sessions.start_download) , 0))AS waiting_time, count(sessions.session_id) AS install_count2"
			.	" FROM `install_users`, `sessions`"
			.	" WHERE `sessions`.`user_id` = `install_users`.`user_id` AND $condition AND $condition2" 
			.	" GROUP by `sessions`.`user_id`"
			.	" ORDER by `sessions`.`user_id` desc"
			.	" LIMIT ".($nbpages * $page_max)." , ".$page_max;
			$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." can't execute the query: ".$query);


			while ( ($row = mysqli_fetch_array($result)) )
			{
				$ret = lastSession($row["user_id"], $condition, $link);
				echo "<tr>";
				echo "<td>".htmlspecialchars($row["user_id"], ENT_QUOTES)."</td>\n";
				if ($show_hddetails == 0)
				{
					echo "<td>".htmlspecialchars($row["first_install"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["last_install"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["install_count2"], ENT_QUOTES)."</td>\n";
					echo "<td>".toHMS(htmlspecialchars($row["waiting_time"], ENT_QUOTES))."</td>\n";
					echo "<td>".toHMS($ret["time"])."</td>\n";
				}
				if ($install_state) { echo "<td>".$ret["percent"]."</td>\n"; }
				echo "<td>".htmlspecialchars($row["memory"], ENT_QUOTES)."</td>\n";
				if ($show_hddetails == 1)
				{
					echo "<td>".htmlspecialchars($row["os"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["proc"], ENT_QUOTES)."</td>\n";

					echo "<td>".htmlspecialchars($row["video_card"], ENT_QUOTES)."</td>\n";
					echo "<td>".htmlspecialchars($row["driver_version"], ENT_QUOTES)."</td>\n";
				}
				echo "<td>".htmlspecialchars($row["state"], ENT_QUOTES)."</td>\n";
				echo "</tr>\n";
			}
			echo "</table>\n";

		}
		if (($install_state==1	&& $show_hdu == 1) || ($install_state==2 && $show_hdu2 == 1) )
		{
			for ($i = 1 ;$i < ($max_result / $page_max)+1; $i = $i+1)
			{
				if ($nbpages + 1 == $i)
				{
					echo ($i)." ";
				}
				else
				{
					if ($install_state==1)
					{
						echo '<a href="'.getHref().'&page_users='.($i).'&page_users2='.($page_users2+1).'">'.($i). '</a> ';
					}
					else
					{
						echo '<a href="'.getHref().'&page_users='.($page_users+1).'&page_users2='.($i).'">'.($i). '</a> ';
					}
				}
			}
		}
	}

	function displayHardwareNewUser($condition, $link)
	{

		global $display;
		if ($display == 4)
		{			
			displayHardwareNewUserImpl(1, "unfinished Install", $condition,  " install_users.state IN ('DU_DL', 'DU_IN')", $link);
		}
		if ($display == 5)
		{
			displayHardwareNewUserImpl(2, "finished Install", $condition,  " NOT (install_users.state IN ('DU_DL', 'DU_IN') )", $link);
		}


	
	}



	// extract the cmd

	if ($cmd == "")
	{
		echo "0:missing cmd";
		die2();
	}

	// check for 'clear password' tag
	switch ($cmd)
	{
	// log <=> display php pabe
	case "view":
		$dt = gettimeofday();
		$date = date('Y-m-d H:i:s', time());
		$ip = getIp();
		$log = getenv("query_string");
		$link = mysqli_connect($StatsDBHost, $StatsDBUserName, $StatsDBPassword, NULL, $DBPort) or die2 (__FILE__. " " .__LINE__." can't connect to database host:$StatsDBHost user:$StatsDBUserName");
		if (function_exists('nel_mysqli_set_charset'))
			nel_mysqli_set_charset($link);
		mysqli_select_db ($link, $StatsDBName) or die2 (__FILE__. " " .__LINE__." can't access to the table dbname:$StatsDBName");


		// verify network allowlist and/or shared secret
		if (!stats_query_authorized())
			die("GENERIC_ERROR");
		{
			//xhtml header + style
			echo '<!doctype html public "-//w3c//dtd html 4.01 transitional//en>'."\n"
			.	'<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="fr" lang="fr">'."\n";


			echo "<head><style =\"text/css\">\n";

			echo "table {"
			.	"border: medium solid #000000;"
			.	"width: 100%;"

			.	"	}";
			echo "th {"

			.	"	border: thin solid #6495ed;"
			//.	"	width: 20%;"
			.	"	background-color:#fcc"
			.	"	}";

			echo "td {"

			.	"	border: thin solid #6495ed;"
			//.	"	width: 20%;"
			.	"	background-color:#eee;"
			.	"	}";



			echo "a {"
				." color : #0033bb;"
				."	text-decoration : none; "
				." }"
				."a:hover {"
				." text-decoration : underline;"
				."}";

			echo "body {"
			.	"padding-left: 5em;"
			.	"margin-right:100px;"
//				.	"width:850px;"
			.	" }";

			echo "h1 {"

			.	"text-align: center;"
			//.	"padding-left: 10em;"
			.	" }";


			echo "</style>\n";
			echo "</head>\n";
			echo "<body>\n";

			//display summary stat by day for current month

			$selected = (int)getPost("selected", strtotime(date('Y-m-d', time())));
			$page_download = (int)getPost("page_download", "1");
			$page_download = $page_download -1;
			if ($page_download < 0) { $page_download = 0;}
			$page_users = (int)getPost("page_users", "1");
			$page_users = $page_users -1;
			$page_users2 = (int)getPost("page_users2", "1");
			$page_users2 = $page_users2 -1;
			if ($page_users < 0) { $page_users = 0;}
			if ($page_users2 < 0) { $page_users2 = 0;}


			$date_first = date('Y-m-d H:i:s', strtotime( date('Y-m-1', $selected) ) );
			$date_first_int = strtotime($date_first);
			$date_last = date('Y-m-d H:i:s', strtotime(date('Y-m-1', $selected + 31*24*3600)) - 1);
			$date_last_int = strtotime($date_last);
			$nbday = round( ($date_last_int +1 - $date_first_int ) / (24*3600));

			$display = (int)getPost("display", "0");

			$date = getdate($selected);
			// display server time
			echo "<h4>server time: ".date("Y-m-d H:i:s", time())."</h4>\n";



			// display title
			echo '<h1>'.$date['weekday'].' '.$date['mday'].' '.$date['month'].' '.$date['year'].'</h1>';

			echo '<h2><a href="'.getHref().'&display=1">downloads</a></h2>';
			echo '<h2><a href="'.getHref().'&display=2">distinct new users</a></h2>';
			echo '<h2><a href="'.getHref().'&display=3">download details</a></h2>';
			echo '<h2><a href="'.getHref().'&display=4">Hardware users - Unfinished install</a></h2>';			
			echo '<h2><a href="'.getHref().'&display=5">Hardware users - Finished install</a></h2>';			

			echo "<hr/><br/>";
			/*
			to add display function copy a display* function (add a global variable to indicates if the content must be display
			add this variable to the getHref function
			 */
			if ($display==1)
			{
				displayDownload($link);
			}
			if ($display ==2)
			{
				displayDistinctUsers($link);
			}

			//display current session

			$current_day = date("Y-m-d", $selected);
			$next_day = date("Y-m-d", $selected + 24*3600);
			$condition = "`sessions`.`start_download` >= '$current_day' AND `sessions`.`start_download` < '$next_day'";
			if ($display == 3)
			{
				displayDownloadDetails($condition, $link);
			}
			$condition2 = "`install_users`.`first_install` >= '$current_day' AND `install_users`.`first_install` < '$next_day'";
			displayHardwareNewUser($condition2, $link);
			
			$dt2= gettimeofday();
			echo "<br/><br/>computed in " . ( $dt2["sec"] - $dt["sec"] +  ($dt2["usec"] - $dt["usec"]) / 1000000  ). "s";
			echo "</body>\n</html>\n";
	
			mysqli_close($link);
			unset($link);

		} //end check ip

		break;

	default:
		echo "0:unknown command";
		die2();
	}



?>

