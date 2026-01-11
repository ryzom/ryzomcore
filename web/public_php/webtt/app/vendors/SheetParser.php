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
class SheetParser
{
	var $debug = false;

	function parseLine($str)
	{
		$arr = str_getcsv($str, "\t");
		return $arr;
	}

	function parseFile($file)
	{
		$parsedEnt = array();
		$newEnt = false;
		$prevStringLine = false;
		$entities = array();
		$diffFile = false;

		$file = mb_convert_encoding($file, 'UTF-8', 'UTF-16');
//		$file = $this->removeBOM($file);
//		$file = $this->removeComments($file);
		$lines = explode("\n", $file);
		if ($this->debug)			
		{
			echo "<pre>\n\n";
		}
		$line_no=1;

/*		var_dump(setlocale(LC_ALL,NULL));
		var_dump(setlocale(LC_ALL,'pl_PL.UTF-8'));*/
		// Need to set UTF-8 locale to get str_getcsv to work with UTF-8 cyryllic
		setlocale(LC_ALL,'pl_PL.UTF-8');

		foreach ($lines as $line)
		{
			if ($this->debug)
			{
				echo "\n\t#################### LINE NUMBER " . $line_no . "\n\n";
			}

//			var_dump($line);
			$line = rtrim($line,"\r\n");

			$parsedLine = $this->parseLine($line);

			if (!$line || mb_strpos($line, "DIFF NOT") === 0 || mb_strpos($line, "REMOVE THE") === 0)
				continue;

			if ($line_no == 1)
			{
				$parsedEnt["type"] = "sheet_description";
				if ($parsedLine[0] == "DIFF_CMD")
					$diffFile = true;
				$parsedEnt["sheet_id_column"] = $parsedLine[2];
			}
			else
			{
				if ($diffFile)
				{
					$parsedEnt["diff"] = $parsedEnt["command"] = rtrim($parsedLine[0]);
					$parsedEnt["hash_value"] = $parsedLine[1];
					$parsedEnt["identifier"] = $parsedLine[2];
				}
				else
				{
					$parsedEnt["hash_value"] = $parsedLine[0];
					$parsedEnt["identifier"] = $parsedLine[1];
				}
				$parsedEnt["type"] = "sheet";
			}

			if ($diffFile)
				$columns = array_slice($parsedLine, 3);
			else
				$columns = array_slice($parsedLine, 2);

			if (!isset($columnsCount))
				$columnsCount = count($columns);

			$parsedEnt["columns"] = $columns;

			if ($this->debug)
			{
				echo "%%%% parsedEnt %%%%%\n";
				var_dump($parsedEnt);
			}

/*			if ($parsedLine["type"] == "internal_index")
					$parsedEnt["internal_index"] = $parsedLine["index"];

			if ($parsedLine["type"] == "diff")
			{
				$parsedEnt["diff"] = $parsedLine["command"];
				$parsedEnt["index"] = $parsedLine["index"];
			}*/

			$newEnt = true;

			if ($newEnt)
			{
				if ($this->debug && 0)
				{
					echo "\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
					echo "\t%%%% newEnt %%%%%%%%% newEnt %%%%%%%%% newEnt %%%%%%%%% newEnt %%%%%\n";
					echo "\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n";
					var_dump($parsedEnt);
				}
/*				if (!isset($parsedEnt["diff"]) && !isset($parsedEnt["index"]))
					$parsedEnt["index"] = $parsedEnt["internal_index"];*/

				$entities[] = $parsedEnt;
				$parsedEnt =array();
				$newEnt = false;
			}
			$line_no++;
		}

		if ($this->debug && 0)
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

	function addBOM($str)
	{
		if(($bom = substr($str, 0,3)) != pack("CCC",0xef,0xbb,0xbf))
			return pack("CCC",0xef,0xbb,0xbf) . $str;
		else
			return $str;
	}

	function buildFile($entities)
	{
		$content = '';
		foreach ($entities as $ent)
		{
			if ($ent['type'] == 'sheet_description')
			{
				$_columns = $ent['columns'];
				$_sheet_id_column = $ent['sheet_id_column'];
				
				if (isset($ent['diff']))
				{
					$content .= 'DIFF_CMD' . "\t" . '*HASH_VALUE' . "\t" . $_sheet_id_column . "\t";
					foreach ($ent['columns'] as $value)
					{
						$content .= $value . "\t";
					}
					$content = mb_substr($content, 0, -1);
					$content .= "\n";
				}
				continue;
			}

			if (isset($ent['command']))
				$content .= $ent['command'] . "\t";
			if (isset($ent['hash_value']))
				$content .= $ent['hash_value'] . "\t";
			else
				$content .= '_0000000000000000' . "\t";
			$content .= $ent['identifier'] . "\t";
			foreach ($ent['columns'] as $value)
			{
				$content .= $value . "\t";
			}
			$content = mb_substr($content, 0, -1);
			$content .= "\n";
		}
		return mb_convert_encoding($this->addBOM($this->CRLF($content)), 'UTF-16LE', 'UTF-8');
//		return mb_convert_encoding($this->CRLF($content), 'UTF-16LE', 'UTF-8');
	}
}
?>