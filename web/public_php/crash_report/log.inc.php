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

/// Simple file logger class
class Logger
{
	private $lf = NULL;

	function __construct()
	{
		// This log holds whatever was posted to submit.php, and this
		// directory is served: writing log.txt next to submit.php publishes
		// it. Keep it under the private path instead.
		global $PRIVATE_PHP_PATH;
		$dir = ( isset( $PRIVATE_PHP_PATH ) && $PRIVATE_PHP_PATH != '' ) ? rtrim( $PRIVATE_PHP_PATH, '/' ) : sys_get_temp_dir();

		$this->lf = fopen( $dir . '/crash_report.log', 'a' );
		if( $this->lf === FALSE )
			exit( 1 );
	}

	function __destruct()
	{
		fclose( $this->lf );
	}

	public function log( $msg )
	{
		$date = date( "[M d, Y H:i:s] " );
		fwrite( $this->lf, $date . $msg . "\n" );
	}
}

?>
