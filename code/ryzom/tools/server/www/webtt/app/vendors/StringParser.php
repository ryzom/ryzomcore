<?php
/*
	Ryzom Core Web-Based Translation Tool
	Copyright (C) 2011 Piotr Kaczmarek <p.kaczmarek@openlink.pl>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
?>
<?php
class StringParser
{
	var $pipeline_directory = "/home/kaczorek/projects/webtt/distfiles/translation/";
	var $debug = false;

	function removeComments($str)
	{
/*		while (($cstart = mb_strpos($str, "/*", $offset)) !== false)
		{
			$cend = mb_strpos();
		}*/
//		var_dump($str);
//As a pertinent note, there's an issue with this function where parsing any string longer than 94326 characters long will silently return null. So be careful where you use it at.
//http://pl.php.net/manual/en/function.preg-replace.php#98843
		ini_set('pcre.backtrack_limit', 10000000);
		//$returnString = preg_replace('!/\*.*?\*/!s', '', $str); // /* .*? */ s
		// added [^/] because there was //******* in translation file
		$returnString = preg_replace('![^/]/\*.*?\*/!s', '', $str); // /* .*? */ s
		// PHP 5.2.0
		// if (PREG_NO_ERROR !== preg_last_error())
		if ($returnString === null)
		{
			$returnStr = $str;
			var_dump("PREG ERROR");
			// exception
		}
		return $returnString;
	}

	function removeBOM($str)
	{
//		mb_internal_encoding("ISO-8859-2");
//		var_dump(substr($str, 0, 3));
		if(($bom = substr($str, 0,3)) == pack("CCC",0xef,0xbb,0xbf))
		{
//			var_dump("jest bom");
			$bla = substr($str, 3);
//			var_dump($bla);
			return $bla;
		}
		else
		{
//			var_dump($bom);
			return $str;
		}
	}

	function addBOM($str)
	{
		if(($bom = substr($str, 0,3)) != pack("CCC",0xef,0xbb,0xbf))
			return pack("CCC",0xef,0xbb,0xbf) . $str;
		else
			return $str;
	}

	function parseLine($str)
	{
		$arr = array();
//		var_dump(mb_internal_encoding());
//		mb_internal_encoding("ISO-8859-2");
		if (mb_substr($str, 0, 2) == "//")
		{
			if (mb_substr($str, 0, 7) == "// DIFF")
			{
				list($j, $type, $command, $args) = explode(" ", $str);
				$command = mb_strtolower($command);
				if ($command == "add" || $command == "changed")
				{
					$index = intval($args);
					$command = mb_substr($str, 3);
				}
				else
					unset($type);
			}
			else if (mb_substr($str, 0, 8) == "// INDEX")
			{
				list($j, $type, $index) = explode(" ", $str);
				$type = "internal_index";
//				$arr = explode(" ", $str);
			}
			else if (mb_substr($str, 0, 13) == "// HASH_VALUE")
			{
				list($j, $type, $hash_value) = explode(" ", $str);
				$type = "hash_value";
			}
/*			if (!isset($type))
			{
				var_dump(isset($type));
				debug_print_backtrace();
			}
			var_dump($type);*/
			if (isset($type))
			{
				$type = mb_strtolower($type);
				$arr = compact("type","command","index","hash_value");
			}
		}
		else if (!(mb_substr($str, 0, 2) == "//") && mb_strlen($str))
		{
			//list($ident, $j
			$type = "string";
			$lBracket = mb_strpos($str, "[");
			$rBracket = mb_strrpos($str, "]");
			$sStart = $lBracket + 1;
			$sEnd = $rBracket - ($sStart);
			$identifier = trim(mb_substr($str, 0, $lBracket));
			if (!$rBracket)
				$sEnd = mb_strlen($str);
			$string = mb_substr($str, $sStart, $sEnd);
			$string = str_replace(
					array('\\\\', '\[','\]'),
					array('\\', '[',']'),				// '
					$string
				);
			$arr = compact("type", "identifier", "string");
		}
/*		echo "<pre>################################\n";
		var_dump($str);
		var_dump($arr);
		echo "</pre>\n";*/
		return $arr;
	}

	function parseFile($file)
	{
		$parsedEnt = array();
		$newEnt = false;
		$prevStringLine = false;
		$entities = array();

//		$file = file_get_contents($this->pipeline_directory . $file);
//		var_dump(mb_substr($file, 0,3));
//		var_dump(substr($file, 0,3));
//		var_dump($file);
		$file = $this->removeBOM($file);
//		var_dump($file);
		$file = $this->removeComments($file);
//		var_dump($file);
		$lines = explode("\n", $file);
		if ($this->debug)			
		{
			echo "<pre>\n\n";
		}
		$line_no=1;
		foreach ($lines as $line)
		{
			if ($this->debug)
			{
				echo "\n\t#################### LINE NUMBER " . $line_no++ . "\n\n";
			}

//			var_dump($line);
//			$line = rtrim($line);
//			$line = str_replace(array("\r\n","\n","\r"), '', $line);
			$line = rtrim($line, "\r\n");
			$parsedLine = $this->parseLine($line);

			if ($this->debug)
			{
				echo "%%%% parsedLine\n";
				var_dump($parsedLine);
				echo "\n";
			
				echo "%%%% prevStringLine\n";
				var_dump($prevStringLine);
				echo "\n";
			}

			if (!$parsedLine)
				continue;

			// if line start with diff (diff files) or hash_value (translated files) and before was line with translation, then we start new ent

			if ($prevStringLine && (
					($parsedLine["type"] == "diff" && $parsedEnt) || ($parsedLine["type"] == "hash_value" && $parsedEnt)
				))
			{
/*				echo "%%%% prevStringLine %%%%%\n";
				var_dump($parsedEnt);*/
				$newEnt = true;
			}

			if ($newEnt)
			{
				if ($this->debug)			
				{
					echo "\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
					echo "\t%%%% newEnt %%%%%%%%% newEnt %%%%%%%%% newEnt %%%%%%%%% newEnt %%%%%\n";
					echo "\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n";
					var_dump($parsedEnt);
				}
				if (!isset($parsedEnt["diff"]) && !isset($parsedEnt["index"]))
					$parsedEnt["index"] = $parsedEnt["internal_index"];

				$entities[] = $parsedEnt;
				$parsedEnt =array();
				$newEnt = false;
			}

			if ($parsedLine["type"] == "internal_index")
					$parsedEnt["internal_index"] = $parsedLine["index"];

			if ($parsedLine["type"] == "string")
			{
				$prevStringLine = true;

				if ($this->debug)			
				{
					echo "%%%% parsedEnt %%%%%\n";
					var_dump($parsedEnt);

//					echo "%%%% parsedLine %%%%%\n";
//					var_dump($parsedLine);
				}

				if (!$parsedLine['identifier'])
				{
					if ($this->debug) echo "ZLACZENIE \n";
					if ($this->debug && !isset($parsedEnt['string']))
					{
						echo "!isset parsedEnt['string']\n";
						var_dump($line);
						var_dump($parsedEnt);
						var_dump($parsedLine);
					}
//					$parsedEnt['string'] .= $parsedLine['string'] . "\n";
					$parsedEnt['string'] .= "\n" . $parsedLine['string'];
				}
				else
				{
					if ($this->debug) echo "DODANIE \n";
					$parsedEnt += $parsedLine;
//					$parsedEnt['string'] .= "\n";
				}

				if ($this->debug)			
				{
					echo "%%%% parsedEnt after %%%%%\n";
					var_dump($parsedEnt);
				}
			}
			else
				$prevStringLine = false;

			if ($parsedLine["type"] == "diff")
			{
				$parsedEnt["diff"] = $parsedEnt["command"] = $parsedLine["command"];
				$parsedEnt["index"] = $parsedLine["index"];
			}
		}
		if ($parsedEnt)
		{
			if (!isset($parsedEnt["diff"]) && !isset($parsedEnt["index"]))
				$parsedEnt["index"] = $parsedEnt["internal_index"];

			$entities[] = $parsedEnt;
		}

		if ($this->debug)			
		{
			echo "<pre>";
			var_dump($entities);
			echo "</pre>\n";
		}
		return $entities;
	}
	
	function CRLF($s)
	{
		$s = str_replace("\r\n", "\n", $s);
		$s = str_replace("\n", "\r\n", $s);
		return $s;
	}

	
	function buildFile($entities)
	{
		$content = '';
		foreach ($entities as $ent)
		{
			if (isset($ent['command']))
				$content .= '// ' . $ent['command'] . "\n";
			if (isset($ent['hash_value']))
				$content .= '// HASH_VALUE ' . $ent['hash_value'];
/*			if (isset($ent['command']))
				$content .= '// INDEX ' . $ent['internal_index'] . "\n";
			else*/
			if (!isset($ent['command']))
				$content .= '// INDEX ' . $ent['index'] . "\n";
			$content .= $ent['identifier'] . "\t" . '[' . $ent['string'] . ']' . "\n";
			$content .= "\n";
		}
		return $this->addBOM($this->CRLF($content));
	}
}
?>