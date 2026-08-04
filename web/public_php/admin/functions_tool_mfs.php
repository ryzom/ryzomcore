<?php

	function tool_mfs_HTTPOpen($url)
	{
		// This tool was never finished: it ignored $url and always POSTed a
		// hard-coded support login to a fixed internal host. Any operator with
		// tool_mfs access would then trigger a credentialed server-side probe.
		// Refuse until it is wired through domain_mfs_web with proper auth.
		unset($url);
		return "Mails &amp; Forums tool is not configured. "
			."(Previously it contacted a hard-coded internal host as user_login=support.)";
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