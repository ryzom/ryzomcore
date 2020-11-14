<?php 

	include_once('../login/config.php');
	
	// LOG database
	$StatsDBHost =  $DBHost;
	$StatsDBUserName = $DBUserName;
	$StatsDBPassword = $DBPassword;
	$StatsDBName = "stats";

	error_reporting(E_ERROR | E_PARSE);
	set_error_handler('err_callback');

	// global var
	$link = NULL;
	$page_max = 100;
	$dev_ip="192.168.1.169"; //ip where sql error are displayed
	$private_network = "/192\.168\.1\./i"; //ip where the cmd=log&msg=dump function works
	$page_name = "stats.php";

	
	
	//get the ip of the viewer
	function getIp()
	{
		if (getenv("HTTP_CLIENT_IP"))
		{
			$ip = getenv("HTTP_CLIENT_IP");
		}
		elseif(getenv("HTTP_X_FORWARDED_FOR"))
		{
			$ip = getenv("HTTP_X_FORWARDED_FOR");
		}
		else
		{
			$ip = getenv("REMOTE_ADDR");
		}
		return $ip;
	}


	// if the player ip is the dev ip then the sql error is explain
	function die2($debug_str)
	{
		global $private_network;
		if ( preg_match($private_network, getIp()) )
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

	// log error in bdd
	function debug($str)
	{
		global $StatsDBHost;
		global $StatsDBUserName;
		global $StatsDBPassword;
		global $StatsDBName;
		global $link;

		$newConnection = 0;
		
		if ($link == NULL)
		{
			$link = mysql_connect($StatsDBHost, $StatsDBUserName, $StatsDBPassword) or die2 (__FILE__. " " .__LINE__." Can't connect to database host:$StatsDBHost user:$StatsDBUserName");
			$newConnection = 1;

			mysql_select_db ($StatsDBName, $link) or die2 (__FILE__. " " .__LINE__." Can't access to the table dbname:$StatsDBName");
		}
		
		
		$str = str_replace("'", "", $str);
		$str = str_replace( '"', "", $str);


		$query= "INSERT INTO `log` ( `log` )"
			. "VALUES ("
			. "'$str'"
			. ")";

		$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
		if ($newConnection == 1)
		{
			mysql_close($link);
			$link = NULL;
		}
	
	}
	function err_callback($errno, $errmsg, $filename, $linenum, $vars)
	{
		debug("$filename $linenum $errmsg");
	}
	

//	debug( getenv("QUERY_STRING") );

	// extract the cmd
	$cmd = getPost("cmd" ,"log");
	if ($cmd == "")
	{
		echo "0:Missing cmd";
		die2();
	}

	// check for 'clear password' tag
	switch ($cmd)
	{
	// log <=> display php page
	case "log":
		$date = date('Y-m-d H:i:s', time());
		$ip = getIp();
		$log = getenv("QUERY_STRING");
		$link = mysql_connect($StatsDBHost, $StatsDBUserName, $StatsDBPassword) or die2 (__FILE__. " " .__LINE__." Can't connect to database host:$StatsDBHost user:$StatsDBUserName");
		mysql_select_db ($StatsDBName, $link) or die2 (__FILE__. " " .__LINE__." Can't access to the table dbname:$StatsDBName");


		$msg = getPost("msg", "");
		switch ($msg)
		{
			//display php infos


			
			case "start_download":

				$query = "SELECT max(`session_id`) as `res` from `sessions`";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);				
				$session_id = 1000;
				if( mysql_num_rows($result) != 0)
				{
					$row = mysql_fetch_array($result);
					$session_id = $row["res"] + 1;
				}
		
				$now = date("Y-m-d H:i:s", time());
				$server = getPost("server", "");
				$application = getPost("application", "");
				$version = getPost("version", "");
				$lang = getPost("lang","");
				$type = getPost("application", "");
				$package = getPost("package", "");
				$protocol = getPost("protocol", "");
				$size_download =getPost("size_download", "");
				$size_install = getPost("size_install", "");
				$user_id = getPost("user_id", "0");
				$previous_download = getPost("previous_download", "0");
		
				
				$query= "INSERT INTO `sessions` ( `session_id`, `user_id` , `server`, `application`, `ip` , `lang`, `type`, `package`, `protocol`, `size_download`, `size_install`, `start_download`, `stop_download`, `previous_download` )"
				
				. "VALUES ("
				. "'$session_id', '$user_id' ,'$server', '$application', '$ip', '$lang', '$type', '$package', '$protocol', '$size_download', '$size_install', '$now', '$now', '$previous_download'"
				. ")";

				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);


				$query= "UPDATE `install_users` set install_count = install_count + 1, state='DU_DL', last_install='$now' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);

				echo "<session_id>".$session_id."</session_id>";	
			break;
			
			case "stop_download":	

				$session_id =getPost("session_id", "0");
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `stop_download` = '$now', `percent_download` ='100' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);

				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_IN' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);

			break;
			// update the percent of download
			case "update_download":	

				$session_id =getPost("session_id", "0");
				$percent = getPost("percent", "0");
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `percent_download` ='$percent', `stop_download` = '$now'  WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;

			// update the percent of finish
			case "update_install":	

				$now = date("Y-m-d H:i:s", time());
				$session_id =getPost("session_id", "0");
				$percent = getPost("percent", "0");
				$query = "UPDATE `sessions` SET `percent_install` ='$percent', `stop_download` = '$now' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;



			case "no_install":

				$session_id = getPost("session_id", "0");
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `size_download` = '0', `start_install` = '$now', `stop_install` = '$now', `percent_install` ='100' WHERE `session_id` = '$session_id' LIMIT 1";
				echo $query;
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;

			// install is finished
			case "stop_install":
				$session_id = getPost("session_id", "0");
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `stop_install` = '$now',  `stop_download` = '$now', `percent_install` ='100' WHERE `session_id` = '$session_id' LIMIT 1";
				echo $query;
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_FI' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;
			// addd user info to database	
			case "start_install":
				$session_id = getPost("session_id", "");
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `start_install` = '$now', `stop_download` = '$now' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);


				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_IN' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_video_mode_setup":
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_VMS' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_video_mode_setup_high_color":
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_VMSHS' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_login_screen":
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_AL' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_post_login":
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_PL' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_character_selection":
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_CS' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_game_entry":
				$user_id = getPost("user_id", "0");
				$query= "UPDATE `install_users` set state='DU_AG' where user_id='$user_id';";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;


			case "login_step_game_exit":
				$user_id = getPost("user_id", "0");
				$play_time = getPost("play_time", "0");
				// manualy estimate the duration of the previous session
				{
					$query = "SELECT `state` from install_users where `user_id` = '$user_id'";
					$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
					$state = "AG";
					if( mysql_num_rows($result) > 0)
					{
						$row = mysql_fetch_array($result);
						$state = $row["state"];				
					}

					if ($state == "DU_P1")
					{
						$play_time = $play_time + 30 *60;
					}
					else if ($state == "DU_P2")
					{
						$play_time = $play_time + 60* 60;
					}
					else if ($state == "DU_P3")
					{
						$play_time = $play_time + 2*60* 60; // P3 will stat P3
					}
				}


				if ($play_time > 2*60*60) // time played > 2 h
				{
					$query= "UPDATE `install_users` set state='DU_P3' where user_id='$user_id';";
					$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				}
				else if ($play_time > 60*60)	// time played > 2 h				
				{
					$query= "UPDATE `install_users` set state='DU_P2' where user_id='$user_id';";
					$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				}
				else if ($play_time > 30*60) // time played > 30 m
				{
					$query= "UPDATE `install_users` set state='DU_P1' where user_id='$user_id';";
					$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				}
				break;

			// addd user info to database	
			case "init":
				$query = "SELECT max(`user_id`) as max_id from `install_users`";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);				
				$user_id = 1;
				if( mysql_num_rows($result) != 0)
				{
					$row = mysql_fetch_array($result);
					$user_id = $row["max_id"] + 1;
				}
	
				$install_id = getPost("install_id", "0");	
				$os = getPost("os", "Unknown");
				$proc = getPost("proc", "Unknown");
				$memory = getPost("memory", "Unknown");
				$video_card = getPost("video_card", "Unknown");
				$driver_version = getPost("driver_version", "Unknown");

				$query = "INSERT INTO `install_users` SET `user_id` = '$user_id', `install_id`='$install_id', `os`='$os', `proc`='$proc', `memory`='$memory', `video_card`='$video_card', `driver_version`='$driver_version', `last_install`='".date('Y-m-d H:i:s', time()) . "', `first_install`=`last_install`";

								 
				
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				echo "<user_id>$user_id</user_id>";	
			break;

			// first log if empyt user_id is return then must init	
			case "login":
				$install_id = getPost("install_id", "0");	
				$query = "SELECT `user_id` from install_users where `install_id` = '$install_id'";
				$result = mysql_query ($query, $link) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				if( mysql_num_rows($result) == 0)
				{
					echo "<user_id></user_id>";
					break;
				}
				$row = mysql_fetch_array($result);
				$user_id = $row["user_id"];
				echo "<user_id>$user_id</user_id>";
			break;


	
			
			default:
			echo "unknown command: $msg $log";
		}
					

		mysql_close($link);
		unset($link);
	
		break;
			
	default:
		echo "0:Unknown command";
		die2();
	}
			

?>
