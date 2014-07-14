<?php

	function tool_ee_parse_getview($data)
	{
		$entity_data = array();

		reset($data);
		foreach($data as $ais_data)
		{
			$service_name 	= 'n/a';
			$answers		= 0;
			$values			= 0;
			$values_read	= 0;

			$entity = array();

			reset($ais_data);
			foreach($ais_data as $ais_line)
			{
				$ais_line = trim($ais_line);
				if (ereg("^===\[ Service ([^\ ]+) returned \]===$", $ais_line, $eregs))
				{
					$service_name = $eregs[1];
				}
				elseif (ereg("^have ([[:digit:]]+) answer$", $ais_line, $eregs))
				{
					$answers = $eregs[1];
				}
				elseif (ereg("^have ([[:digit:]]+) value$", $ais_line, $eregs))
				{
					$values = $eregs[1];
				}
				elseif (ereg("^service ->(.*)$", $ais_line, $eregs))
				{
					$values_read++;
					$entity['service_id'] = trim($eregs[1]);
				}
				elseif (ereg("^entity ->(.*)$", $ais_line, $eregs))
				{
					$values_read++;
					$entity['entity'] = trim($eregs[1]);
					$entity_string = str_replace(array('(',')','0x'), '', $entity['entity']);
					$entity_string = str_replace(':','_', $entity_string);
					$entity['entity_string'] = $entity_string;
				}
				elseif (ereg("^NamedEntityName ->(.*)$", $ais_line, $eregs))
				{
					$values_read++;
					$entity['entity_name'] = trim($eregs[1]);
				}
				elseif (ereg("^NamedEntityState ->(.*)$", $ais_line, $eregs))
				{
					$values_read++;
					$entity['entity_state'] = trim($eregs[1]);
				}
				elseif (ereg("^NamedEntityParam1 ->(.*)$", $ais_line, $eregs))
				{
					$values_read++;
					$entity['entity_param1'] = trim($eregs[1]);
				}
				elseif (ereg("^NamedEntityParam2 ->(.*)$", $ais_line, $eregs))
				{
					$values_read++;
					$entity['entity_param2'] = trim($eregs[1]);
				}

				if (sizeof($entity) && ($values_read == $values) && ($values > 0))
				{
					$entity['service'] = $service_name;
					$entity['service_code'] = md5($service_name);
					$entity_data[] = $entity;
					$entity = array();
					$values = 0;
					$values_read = 0;
				}

			}
		}

		return $entity_data;
	}

	function tool_ee_get_entities($data)
	{
		$entities = array();

		reset($data);
		foreach($data as $dkey => $dval)
		{
			$dkey_bits = explode('_', $dkey);
			$dkey_nums = sizeof($dkey_bits);

			if ($dkey_nums > 4)
			{
				$_entity_bit_4 		= array_pop($dkey_bits);
				$_entity_bit_3 		= array_pop($dkey_bits);
				$_entity_bit_2 		= array_pop($dkey_bits);
				$_entity_bit_1 		= array_pop($dkey_bits);

				$dkey_bits			= array_reverse($dkey_bits);
				$_entity_service	= array_pop($dkey_bits);
				$dkey_bits			= array_reverse($dkey_bits);

				$dkey_entity 		= $_entity_bit_1 .'_'. $_entity_bit_2 .'_'. $_entity_bit_3 .'_'. $_entity_bit_4;
				$dkey_name			= implode('_', $dkey_bits);

				if (!isset($entities[$_entity_service .'_'. $dkey_entity]))
				{
					$entities[$_entity_service .'_'. $dkey_entity] = array();
				}

				$entities[$_entity_service .'_'. $dkey_entity][$dkey_name] = trim($dval);
			}
		}

		return $entities;
	}

?>