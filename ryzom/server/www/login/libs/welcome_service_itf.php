<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$WS_TUserRole_EnumValues[$arrayCounter++] = "ur_player";
	$WS_TUserRole_EnumValues[$arrayCounter++] = "ur_editor";
	$WS_TUserRole_EnumValues[$arrayCounter++] = "ur_animator";
	$WS_TUserRole_EnumValues[$arrayCounter] = "invalid";
	$WS_TUserRole_InvalidValue = $arrayCounter;

	class WS_TUserRole
	{
		var $Value;
		
		function WS_TUserRole()
		{
			global $WS_TUserRole_InvalidValue;
			$this->Value = $WS_TUserRole_InvalidValue;
		}
			
		function toString()
		{
			global $WS_TUserRole_EnumValues;
			return $WS_TUserRole_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $WS_TUserRole_EnumValues;
			foreach ($WS_TUserRole_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $WS_TUserRole_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $WS_TUserRole_InvalidValue;
			global $WS_TUserRole_EnumValues;
			if (array_key_exists($intValue, $WS_TUserRole_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $WS_TUserRole_InvalidValue;
		}
	}	
?>
