<?php
	/*
	 * Just to make sure every datasource has a drive() function...
	 */

	abstract class SourceDriver {
		abstract function drive($cdata);
	}
?>