<?php

// Ryzom Core MMORPG framework - Error Reporter
//
// Copyright (C) 2015 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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

require_once( 'config.inc.php' );
require_once( 'log.inc.php' );

/// Example web application that takes bug reports from the bug reporter Qt app
class BugReportGatherApp
{
	private $db = NULL;
	private $logger = NULL;

	function __construct()
	{
		$this->logger = new Logger();
	}

	private function logPOSTVars()
	{
		$report = "";
		$descr = "";
		$email = "";

		if( isset( $_POST[ 'report' ] ) )
			$report = $_POST[ 'report' ];

		if( isset( $_POST[ 'descr' ] ) )
			$descr = $_POST[ 'descr' ];

		if( isset( $_POST[ 'email' ] ) )
			$email = $_POST[ 'email' ];

		$this->logger->log( 'report: ' . "\n" . $report );
		$this->logger->log( 'description: ' . "\n" . $descr );
		$this->logger->log( 'email: ' . "\n" . $email );
	}

	private function buildQuery()
	{
		$report = "";
		$descr = "";
		$email = "";

		if( isset( $_POST[ 'report' ] ) )
			$report = $_POST[ 'report' ];

		if( isset( $_POST[ 'descr' ] ) )
			$descr = $_POST[ 'descr' ];

		if( isset( $_POST[ 'email' ] ) )
			$email = $_POST[ 'email' ];

		$report = $this->db->real_escape_string( $report );
		$descr  = $this->db->real_escape_string( $descr );
		$email  = $this->db->real_escape_string( $email );


		$q = "INSERT INTO `bugs` (`report`,`description`,`email`) VALUES (";
		$q .= "'$report',";
		$q .= "'$descr',";
		$q .= "'$email')";

		return $q;
	}

	public function exec()
	{
		//$this->logPOSTVars();

		$this->db = new mysqli( BugReportConfig::$dbhost, BugReportConfig::$dbuser, BugReportConfig::$dbpw, BugReportConfig::$dbdb, BugReportConfig::$dbport );
		if( mysqli_connect_error() )
		{
			$this->logger->log( "Connection error :(" );
			$this->logger->log( mysqli_connect_error() );
			return;
		}

		$q = $this->buildQuery();
		$result = $this->db->query( $q );
		if( $result !== TRUE )
		{
			$this->logger->log( "Query failed :(" );
			$this->logger->log( 'Query: ' . $q );
			$this->logPOSTVars();
		}		

		$this->db->close();
	}
}


$app = new BugReportGatherApp();
$app->exec();

?>
