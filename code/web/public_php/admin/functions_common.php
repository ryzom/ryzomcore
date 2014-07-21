<?php

	/*
	 * THIS FILE SHOULD ONLY INCLUDE SMALL USEFUL FUNCTIONS
	 */

	/*
	 * pushes some data in the debug variable
	 */
	function nt_common_add_debug($data)
	{
		global $nel_debug;

		if (is_array($data))	$nel_debug[] = print_r($data,true);
		else					$nel_debug[] = $data;
	}

	/*
	 * redirects to a defined url
	 */
    function nt_common_redirect($url)
    {
        $url = trim($url);
        if (substr($url,0,1) == '/')    $url = substr($url,1);

        $redirect = NELTOOL_SITEBASE . $url;

        header("Location: ". $redirect);
        exit();
    }

	/*
	 * adds a user action log
	 */
	function nt_common_add_log($userinfo, $log_action, $log_desc)
	{
		global $db, $NELTOOL;

		$log_action = trim($log_action);
		$log_desc	= trim($log_desc);

		if (!is_array($userinfo) && (!isset($userinfo['user_id'])))	return false;
		if ($log_action == '') 										return false;
		if ($log_desc == '') 										return false;

		$sql = "INSERT INTO ". NELDB_LOG_TABLE ." ('log_user_id','log_action','log_description','log_date','log_ip') VALUES ('". $userinfo['user_id'] ."','". addslashes($log_action) ."','". addslashes($log_desc) ."','". time() ."','". $NELTOOL['SERVER_VARS']['REMOTE_ADDR'] ."')";
		$db->sql_query($sql);

		return true;
	}


	if (!function_exists('array_combine'))
	{
		function array_combine( $keys, $vals )
		{
			$keys = array_values( (array) $keys );
			$vals = array_values( (array) $vals );
			$n = max( count( $keys ), count( $vals ) );
			$r = array();
			for( $i=0; $i<$n; $i++ )
			{
				$r[ $keys[ $i ] ] = $vals[ $i ];
			}
			return $r;
		}
	}

	if (!function_exists('array_chunk')) {

		function array_chunk($input,$size,$preserve_keys=false)
		{
	       @reset($input);

	       $i = $j = 0;

	       while (@list($key,$value) = @each($input))
	       {
	           if( !( isset( $chunks[$i] ) ) )
	           {
	               $chunks[$i] = array();
	           }

	           if( count( $chunks[$i] ) < $size )
	           {
	               if( $preserve_keys )
	               {
	                   $chunks[$i][$key] = $value;
	                   $j++;
	               }
	               else
	               {
	                   $chunks[$i][] = $value;
	               }
	           }
	           else
	           {
	               $i++;

	               if( $preserve_keys )
	               {
	                   $chunks[$i][$key] = $value;
	                   $j++;
	               }
	               else
	               {
	                   $j = 0;
	                   $chunks[$i][$j] = $value;
	               }
	           }
	       }

	       return $chunks;
	   }
	}

	function array_natsort_list($array)
	{
		// for all arguments without the first starting at end of list
		for ($i=func_num_args();$i>1;$i--)
		{
			// get column to sort by
			$sort_by = func_get_arg($i-1);

			// clear arrays
			$new_array = array();
			$temporary_array = array();

			// walk through original array
			foreach($array as $original_key => $original_value)
			{
				// and save only values
				$temporary_array[] = $original_value[$sort_by];
			}

			// sort array on values
			natsort($temporary_array);

			// delete double values
			$temporary_array = array_unique($temporary_array);

			// walk through temporary array
			foreach($temporary_array as $temporary_value)
			{
				// walk through original array
				foreach($array as $original_key => $original_value)
				{
					// and search for entries having the right value
					if($temporary_value == $original_value[$sort_by])
					{
						// save in new array
						$new_array[$original_key] = $original_value;
					}
				}
			}

			// update original array
			$array = $new_array;
		}

		return $array;
	}

  	function nt_common_assert( $script, $line, $message )
  	{
  		nt_common_add_debug('ASSERT ('. $script .':'. $line .') : '. ereg_replace( '^.*//\*', '', $message ));
		//exit;
	}

	function nt_log($data)
	{
		global $db;
		global $nel_user;

		$log_user_name	= $nel_user['user_name'];
		$log_date		= time();
		$log_data		= addslashes(trim($data));

		$sql = "INSERT INTO ". NELDB_LOG_TABLE ." (`logs_user_name`,`logs_date`,`logs_data`) VALUES ('". $log_user_name ."','". $log_date ."','". $log_data ."')";
		$db->sql_query($sql);
	}

	function nt_sleep($delay)
	{
		if ($delay > 0)
		{
			sleep($delay);
		}
	}

	function nt_email($subject,$message,$emails=null)
	{
		if ($message !== '' && $subject !== '')
		{
			if ($emails === null)
			{
				$emails = 'vl@ryzom.com';
			}
			elseif (is_array($emails))
			{
				$emails = implode(', ', $emails);
			}

			$headers = "From: vl@ryzom.com\r\nReply-To: vl@ryzom.com\r\nX-Mailer: Shard Admin Tool\r\n";
			mail($emails, $subject, $message, $headers);
		}
	}

?>