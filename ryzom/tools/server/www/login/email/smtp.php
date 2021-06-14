<?php
/**
* Filename.......: class.smtp.inc
* Project........: SMTP Class
* Version........: 1.0.5
* Last Modified..: 21 December 2001
*/

	define('SMTP_STATUS_NOT_CONNECTED', 1, TRUE);
	define('SMTP_STATUS_CONNECTED', 2, TRUE);

	class smtp{

		var $authenticated;
		var $connection;
		var $recipients;
		var $headers;
		var $timeout;
		var $errors;
		var $status;
		var $body;
		var $from;
		var $host;
		var $port;
		var $helo;
		var $auth;
		var $user;
		var $pass;

		/**
        * Constructor function. Arguments:
		* $params - An assoc array of parameters:
		*
		*   host    - The hostname of the smtp server		Default: localhost
		*   port    - The port the smtp server runs on		Default: 25
		*   helo    - What to send as the HELO command		Default: localhost
		*             (typically the hostname of the
		*             machine this script runs on)
		*   auth    - Whether to use basic authentication	Default: FALSE
		*   user    - Username for authentication			Default: <blank>
		*   pass    - Password for authentication			Default: <blank>
		*   timeout - The timeout in seconds for the call	Default: 5
		*             to fsockopen()
        */

		function smtp($params = array()){

			if(!defined('CRLF'))
				define('CRLF', "\r\n", TRUE);

			$this->authenticated	= FALSE;			
			$this->timeout			= 5;
			$this->status			= SMTP_STATUS_NOT_CONNECTED;
			$this->host				= 'localhost';
			$this->port				= 25;
			$this->helo				= 'localhost';
			$this->auth				= FALSE;
			$this->user				= '';
			$this->pass				= '';
			$this->errors   		= array();

			foreach($params as $key => $value){
				$this->$key = $value;
			}
		}

		/**
        * Connect function. This will, when called
		* statically, create a new smtp object, 
		* call the connect function (ie this function)
		* and return it. When not called statically,
		* it will connect to the server and send
		* the HELO command.
        */

		function &connect($params = array()){

			if(!isset($this->status)){
				$obj = new smtp($params);
				if($obj->connect()){
					$obj->status = SMTP_STATUS_CONNECTED;
				}

				return $obj;

			}else{
				$this->connection = fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout);
				if(function_exists('socket_set_timeout')){
					@socket_set_timeout($this->connection, 5, 0);
				}

				$greeting = $this->get_data();
				if(is_resource($this->connection)){
					return $this->auth ? $this->ehlo() : $this->helo();
				}else{
					$this->errors[] = 'Failed to connect to server: '.$errstr;
					return FALSE;
				}
			}
		}

		/**
        * Function which handles sending the mail.
		* Arguments:
		* $params	- Optional assoc array of parameters.
		*            Can contain:
		*              recipients - Indexed array of recipients
		*              from       - The from address. (used in MAIL FROM:),
		*                           this will be the return path
		*              headers    - Indexed array of headers, one header per array entry
		*              body       - The body of the email
		*            It can also contain any of the parameters from the connect()
		*            function
        */

		function send($params = array()){

			foreach($params as $key => $value){
				$this->set($key, $value);
			}

			if($this->is_connected()){

				// Do we auth or not? Note the distinction between the auth variable and auth() function
				if($this->auth AND !$this->authenticated){
					if(!$this->auth())
						return FALSE;
				}

				$this->mail($this->from);
				if(is_array($this->recipients))
					foreach($this->recipients as $value)
						$this->rcpt($value);
				else
					$this->rcpt($this->recipients);

				if(!$this->data())
					return FALSE;

				// Transparency
				$headers = str_replace(CRLF.'.', CRLF.'..', trim(implode(CRLF, $this->headers)));
				$body    = str_replace(CRLF.'.', CRLF.'..', $this->body);
				$body    = $body[0] == '.' ? '.'.$body : $body;

				$this->send_data($headers);
				$this->send_data('');
				$this->send_data($body);
				$this->send_data('.');

				$result = (substr(trim($this->get_data()), 0, 3) === '250');
				//$this->rset();
				return $result;
			}else{
				$this->errors[] = 'Not connected!';
				return FALSE;
			}
		}
		
		/**
        * Function to implement HELO cmd
        */

		function helo(){
			if(is_resource($this->connection)
					AND $this->send_data('HELO '.$this->helo)
					AND substr(trim($error = $this->get_data()), 0, 3) === '250' ){

				return TRUE;

			}else{
				$this->errors[] = 'HELO command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}
		
		/**
        * Function to implement EHLO cmd
        */

		function ehlo(){
			if(is_resource($this->connection)
					AND $this->send_data('EHLO '.$this->helo)
					AND substr(trim($error = $this->get_data()), 0, 3) === '250' ){

				return TRUE;

			}else{
				$this->errors[] = 'EHLO command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}
		
		/**
        * Function to implement RSET cmd
        */

		function rset(){
			if(is_resource($this->connection)
					AND $this->send_data('RSET')
					AND substr(trim($error = $this->get_data()), 0, 3) === '250' ){

				return TRUE;

			}else{
				$this->errors[] = 'RSET command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}
		
		/**
        * Function to implement QUIT cmd
        */

		function quit(){
			if(is_resource($this->connection)
					AND $this->send_data('QUIT')
					AND substr(trim($error = $this->get_data()), 0, 3) === '221' ){

				fclose($this->connection);
				$this->status = SMTP_STATUS_NOT_CONNECTED;
				return TRUE;

			}else{
				$this->errors[] = 'QUIT command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}
		
		/**
        * Function to implement AUTH cmd
        */

		function auth(){
			if(is_resource($this->connection)
					AND $this->send_data('AUTH LOGIN')
					AND substr(trim($error = $this->get_data()), 0, 3) === '334'
					AND $this->send_data(base64_encode($this->user))			// Send username
					AND substr(trim($error = $this->get_data()),0,3) === '334'
					AND $this->send_data(base64_encode($this->pass))			// Send password
					AND substr(trim($error = $this->get_data()),0,3) === '235' ){

				$this->authenticated = TRUE;
				return TRUE;

			}else{
				$this->errors[] = 'AUTH command failed: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}

		/**
        * Function that handles the MAIL FROM: cmd
        */
		
		function mail($from){

			if($this->is_connected()
				AND $this->send_data('MAIL FROM:<'.$from.'>')
				AND substr(trim($this->get_data()), 0, 2) === '250' ){

				return TRUE;

			}else
				return FALSE;
		}

		/**
        * Function that handles the RCPT TO: cmd
        */
		
		function rcpt($to){

			if($this->is_connected()
				AND $this->send_data('RCPT TO:<'.$to.'>')
				AND substr(trim($error = $this->get_data()), 0, 2) === '25' ){

				return TRUE;

			}else{
				$this->errors[] = trim(substr(trim($error), 3));
				return FALSE;
			}
		}

		/**
        * Function that sends the DATA cmd
        */

		function data(){

			if($this->is_connected()
				AND $this->send_data('DATA')
				AND substr(trim($error = $this->get_data()), 0, 3) === '354' ){
 
				return TRUE;

			}else{
				$this->errors[] = trim(substr(trim($error), 3));
				return FALSE;
			}
		}

		/**
        * Function to determine if this object
		* is connected to the server or not.
        */

		function is_connected(){

			return (is_resource($this->connection) AND ($this->status === SMTP_STATUS_CONNECTED));
		}

		/**
        * Function to send a bit of data
        */

		function send_data($data){

			if(is_resource($this->connection)){
				return fwrite($this->connection, $data.CRLF, strlen($data)+2);
				
			}else
				return FALSE;
		}

		/**
        * Function to get data.
        */

		function &get_data(){

			$return = '';
			$line   = '';
			$loops  = 0;

			if(is_resource($this->connection)){
				while((strpos($return, CRLF) === FALSE OR substr($line,3,1) !== ' ') AND $loops < 100){
					$line    = fgets($this->connection, 512);
					$return .= $line;
					$loops++;
				}
				return $return;

			}else
				return FALSE;
		}

		/**
        * Sets a variable
        */
		
		function set($var, $value){

			$this->$var = $value;
			return TRUE;
		}

	} // End of class
?>