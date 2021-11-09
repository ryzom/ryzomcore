<?php

	function tool_mfs_HTTPOpen($url)
	{
		$ch = curl_init();

		$url = "http://su1:50000/admin.php";

		$uri_params  = 'user_login=support';
		$uri_params .= '&shard=103';
		$uri_params .= '&forum=Atrium Keepers';

		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $uri_params);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); // 0 = debug , 1 = normal
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1); // 0 = debug , 1 = normal
		curl_setopt($ch, CURLOPT_NOPROGRESS, 0);
		curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)");
		curl_setopt($ch, CURLOPT_HEADER, 1); // has to be 1 due to using redirections
		curl_setopt($ch, CURLOPT_TIMEOUT, 120);

		ob_start();
		$curlOutput	= curl_exec ($ch);
		ob_end_clean();

		$curlError	= curl_errno($ch);
		if ($curlError != 0)
		{
			$outp		= "CURL Error [ $curlError ] : ". curl_error($ch);
		}
		else
		{
			$curlData 	= tool_mfs_HTTPParseResponse($curlOutput);
			$outp		= $curlData[2];
		}

		curl_close ($ch);

		return $outp;
	}


	function tool_mfs_HTTPParseResponse($response)
	{
		/*
		returns an array in the following format which varies depending on headers returned

		[0] => the HTTP error or response code such as 404
		[1] => Array
		(
		[Date] => Wed, 28 Apr 2004 23:29:20 GMT
		[Set-Cookie] => COOKIESTUFF
		[Expires] => Thu, 01 Dec 1994 16:00:00 GMT
		[Content-Type] => text/html
		)
		[2] => Response body (string)
		*/

		do
		{
			list($response_headers,$response) = explode("\r\n\r\n",$response,2);
			$response_header_lines = explode("\r\n",$response_headers);

			// first line of headers is the HTTP response code
			$http_response_line = array_shift($response_header_lines);
			if (preg_match('@^HTTP/[0-9]\.[0-9] ([0-9]{3})@',$http_response_line, $matches))
			{
				$response_code = $matches[1];
			}
			else
			{
				$response_code = "Error";
			}
		}
		while ((substr($response_code, 0,1) == "1") || (substr($response_code, 0,1) == "3"));

		$response_body = $response;

		// put the rest of the headers in an array
		$response_header_array = array();
		foreach ($response_header_lines as $header_line)
		{
			list($header,$value) = explode(': ',$header_line,2);
			$response_header_array[$header] = $value;
		}

		return array($response_code,$response_header_array,$response_body);
	}


?>