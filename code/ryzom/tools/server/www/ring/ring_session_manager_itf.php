<?php
	require_once('welcome_service_itf.php');
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_play_subscribed";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_play_invited";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_edit_invited";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_anim_invited";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_playing";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_editing";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter++] = "sps_animating";
	$RSMGR_TSessionPartStatus_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TSessionPartStatus_InvalidValue = $arrayCounter;

	class RSMGR_TSessionPartStatus
	{
		var $Value;
		
		function RSMGR_TSessionPartStatus()
		{
			global $RSMGR_TSessionPartStatus_InvalidValue;
			$this->Value = $RSMGR_TSessionPartStatus_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TSessionPartStatus_EnumValues;
			return $RSMGR_TSessionPartStatus_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TSessionPartStatus_EnumValues;
			foreach ($RSMGR_TSessionPartStatus_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TSessionPartStatus_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TSessionPartStatus_InvalidValue;
			global $RSMGR_TSessionPartStatus_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TSessionPartStatus_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TSessionPartStatus_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TSessionType_EnumValues[$arrayCounter++] = "st_edit";
	$RSMGR_TSessionType_EnumValues[$arrayCounter++] = "st_anim";
	$RSMGR_TSessionType_EnumValues[$arrayCounter++] = "st_outland";
	$RSMGR_TSessionType_EnumValues[$arrayCounter++] = "st_mainland";
	$RSMGR_TSessionType_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TSessionType_InvalidValue = $arrayCounter;

	class RSMGR_TSessionType
	{
		var $Value;
		
		function RSMGR_TSessionType()
		{
			global $RSMGR_TSessionType_InvalidValue;
			$this->Value = $RSMGR_TSessionType_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TSessionType_EnumValues;
			return $RSMGR_TSessionType_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TSessionType_EnumValues;
			foreach ($RSMGR_TSessionType_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TSessionType_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TSessionType_InvalidValue;
			global $RSMGR_TSessionType_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TSessionType_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TSessionType_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter++] = "so_newbie_training";
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter++] = "so_story_telling";
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter++] = "so_mistery";
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter++] = "so_hack_slash";
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter++] = "so_guild_training";
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter++] = "so_other";
	$RSMGR_TSessionOrientation_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TSessionOrientation_InvalidValue = $arrayCounter;

	class RSMGR_TSessionOrientation
	{
		var $Value;
		
		function RSMGR_TSessionOrientation()
		{
			global $RSMGR_TSessionOrientation_InvalidValue;
			$this->Value = $RSMGR_TSessionOrientation_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TSessionOrientation_EnumValues;
			return $RSMGR_TSessionOrientation_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TSessionOrientation_EnumValues;
			foreach ($RSMGR_TSessionOrientation_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TSessionOrientation_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TSessionOrientation_InvalidValue;
			global $RSMGR_TSessionOrientation_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TSessionOrientation_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TSessionOrientation_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TSessionState_EnumValues[$arrayCounter++] = "ss_planned";
	$RSMGR_TSessionState_EnumValues[$arrayCounter++] = "ss_open";
	$RSMGR_TSessionState_EnumValues[$arrayCounter++] = "ss_locked";
	$RSMGR_TSessionState_EnumValues[$arrayCounter++] = "ss_closed";
	$RSMGR_TSessionState_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TSessionState_InvalidValue = $arrayCounter;

	class RSMGR_TSessionState
	{
		var $Value;
		
		function RSMGR_TSessionState()
		{
			global $RSMGR_TSessionState_InvalidValue;
			$this->Value = $RSMGR_TSessionState_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TSessionState_EnumValues;
			return $RSMGR_TSessionState_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TSessionState_EnumValues;
			foreach ($RSMGR_TSessionState_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TSessionState_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TSessionState_InvalidValue;
			global $RSMGR_TSessionState_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TSessionState_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TSessionState_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TAnimMode_EnumValues[$arrayCounter++] = "am_dm";
	$RSMGR_TAnimMode_EnumValues[$arrayCounter++] = "am_autonomous";
	$RSMGR_TAnimMode_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TAnimMode_InvalidValue = $arrayCounter;

	class RSMGR_TAnimMode
	{
		var $Value;
		
		function RSMGR_TAnimMode()
		{
			global $RSMGR_TAnimMode_InvalidValue;
			$this->Value = $RSMGR_TAnimMode_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TAnimMode_EnumValues;
			return $RSMGR_TAnimMode_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TAnimMode_EnumValues;
			foreach ($RSMGR_TAnimMode_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TAnimMode_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TAnimMode_InvalidValue;
			global $RSMGR_TAnimMode_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TAnimMode_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TAnimMode_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TAccessType_EnumValues[$arrayCounter++] = "at_public";
	$RSMGR_TAccessType_EnumValues[$arrayCounter++] = "at_private";
	$RSMGR_TAccessType_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TAccessType_InvalidValue = $arrayCounter;

	class RSMGR_TAccessType
	{
		var $Value;
		
		function RSMGR_TAccessType()
		{
			global $RSMGR_TAccessType_InvalidValue;
			$this->Value = $RSMGR_TAccessType_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TAccessType_EnumValues;
			return $RSMGR_TAccessType_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TAccessType_EnumValues;
			foreach ($RSMGR_TAccessType_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TAccessType_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TAccessType_InvalidValue;
			global $RSMGR_TAccessType_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TAccessType_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TAccessType_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TRuleType_EnumValues[$arrayCounter++] = "rt_strict";
	$RSMGR_TRuleType_EnumValues[$arrayCounter++] = "rt_liberal";
	$RSMGR_TRuleType_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TRuleType_InvalidValue = $arrayCounter;

	class RSMGR_TRuleType
	{
		var $Value;
		
		function RSMGR_TRuleType()
		{
			global $RSMGR_TRuleType_InvalidValue;
			$this->Value = $RSMGR_TRuleType_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TRuleType_EnumValues;
			return $RSMGR_TRuleType_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TRuleType_EnumValues;
			foreach ($RSMGR_TRuleType_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TRuleType_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TRuleType_InvalidValue;
			global $RSMGR_TRuleType_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TRuleType_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TRuleType_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter++] = "lf_a";
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter++] = "lf_b";
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter++] = "lf_c";
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter++] = "lf_d";
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter++] = "lf_e";
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter++] = "lf_f";
	$RSMGR_TLevelFilter_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TLevelFilter_InvalidValue = $arrayCounter;

	class RSMGR_TLevelFilter
	{
		var $Value;
		
		function RSMGR_TLevelFilter()
		{
			global $RSMGR_TLevelFilter_InvalidValue;
			$this->Value = $RSMGR_TLevelFilter_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TLevelFilter_EnumValues;
			return $RSMGR_TLevelFilter_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TLevelFilter_EnumValues;
			foreach ($RSMGR_TLevelFilter_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TLevelFilter_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TLevelFilter_InvalidValue;
			global $RSMGR_TLevelFilter_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TLevelFilter_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TLevelFilter_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TEstimatedDuration_EnumValues[$arrayCounter++] = "et_short";
	$RSMGR_TEstimatedDuration_EnumValues[$arrayCounter++] = "et_medium";
	$RSMGR_TEstimatedDuration_EnumValues[$arrayCounter++] = "et_long";
	$RSMGR_TEstimatedDuration_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TEstimatedDuration_InvalidValue = $arrayCounter;

	class RSMGR_TEstimatedDuration
	{
		var $Value;
		
		function RSMGR_TEstimatedDuration()
		{
			global $RSMGR_TEstimatedDuration_InvalidValue;
			$this->Value = $RSMGR_TEstimatedDuration_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TEstimatedDuration_EnumValues;
			return $RSMGR_TEstimatedDuration_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TEstimatedDuration_EnumValues;
			foreach ($RSMGR_TEstimatedDuration_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TEstimatedDuration_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TEstimatedDuration_InvalidValue;
			global $RSMGR_TEstimatedDuration_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TEstimatedDuration_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TEstimatedDuration_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TRaceFilter_EnumValues[$arrayCounter++] = "rf_fyros";
	$RSMGR_TRaceFilter_EnumValues[$arrayCounter++] = "rf_matis";
	$RSMGR_TRaceFilter_EnumValues[$arrayCounter++] = "rf_tryker";
	$RSMGR_TRaceFilter_EnumValues[$arrayCounter++] = "rf_zorai";
	$RSMGR_TRaceFilter_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TRaceFilter_InvalidValue = $arrayCounter;

	class RSMGR_TRaceFilter
	{
		var $Value;
		
		function RSMGR_TRaceFilter()
		{
			global $RSMGR_TRaceFilter_InvalidValue;
			$this->Value = $RSMGR_TRaceFilter_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TRaceFilter_EnumValues;
			return $RSMGR_TRaceFilter_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TRaceFilter_EnumValues;
			foreach ($RSMGR_TRaceFilter_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TRaceFilter_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TRaceFilter_InvalidValue;
			global $RSMGR_TRaceFilter_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TRaceFilter_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TRaceFilter_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TReligionFilter_EnumValues[$arrayCounter++] = "rf_kami";
	$RSMGR_TReligionFilter_EnumValues[$arrayCounter++] = "rf_karavan";
	$RSMGR_TReligionFilter_EnumValues[$arrayCounter++] = "rf_neutral";
	$RSMGR_TReligionFilter_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TReligionFilter_InvalidValue = $arrayCounter;

	class RSMGR_TReligionFilter
	{
		var $Value;
		
		function RSMGR_TReligionFilter()
		{
			global $RSMGR_TReligionFilter_InvalidValue;
			$this->Value = $RSMGR_TReligionFilter_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TReligionFilter_EnumValues;
			return $RSMGR_TReligionFilter_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TReligionFilter_EnumValues;
			foreach ($RSMGR_TReligionFilter_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TReligionFilter_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TReligionFilter_InvalidValue;
			global $RSMGR_TReligionFilter_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TReligionFilter_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TReligionFilter_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TGuildFilter_EnumValues[$arrayCounter++] = "gf_only_my_guild";
	$RSMGR_TGuildFilter_EnumValues[$arrayCounter++] = "gf_any_player";
	$RSMGR_TGuildFilter_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TGuildFilter_InvalidValue = $arrayCounter;

	class RSMGR_TGuildFilter
	{
		var $Value;
		
		function RSMGR_TGuildFilter()
		{
			global $RSMGR_TGuildFilter_InvalidValue;
			$this->Value = $RSMGR_TGuildFilter_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TGuildFilter_EnumValues;
			return $RSMGR_TGuildFilter_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TGuildFilter_EnumValues;
			foreach ($RSMGR_TGuildFilter_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TGuildFilter_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TGuildFilter_InvalidValue;
			global $RSMGR_TGuildFilter_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TGuildFilter_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TGuildFilter_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard00";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard01";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard02";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard03";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard04";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard05";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard06";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard07";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard08";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard09";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard10";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard11";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard12";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard13";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard14";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard15";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard16";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard17";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard18";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard19";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard20";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard21";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard22";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard23";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard24";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard25";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard26";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard27";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard28";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard29";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard30";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter++] = "sf_shard31";
	$RSMGR_TShardFilter_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TShardFilter_InvalidValue = $arrayCounter;

	class RSMGR_TShardFilter
	{
		var $Value;
		
		function RSMGR_TShardFilter()
		{
			global $RSMGR_TShardFilter_InvalidValue;
			$this->Value = $RSMGR_TShardFilter_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TShardFilter_EnumValues;
			return $RSMGR_TShardFilter_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TShardFilter_EnumValues;
			foreach ($RSMGR_TShardFilter_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TShardFilter_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TShardFilter_InvalidValue;
			global $RSMGR_TShardFilter_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TShardFilter_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TShardFilter_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
	$RSMGR_TSessionEvent_EnumValues[$arrayCounter++] = "se_char_enter";
	$RSMGR_TSessionEvent_EnumValues[$arrayCounter++] = "se_char_leave";
	$RSMGR_TSessionEvent_EnumValues[$arrayCounter++] = "se_session_closing";
	$RSMGR_TSessionEvent_EnumValues[$arrayCounter] = "invalid";
	$RSMGR_TSessionEvent_InvalidValue = $arrayCounter;

	class RSMGR_TSessionEvent
	{
		var $Value;
		
		function RSMGR_TSessionEvent()
		{
			global $RSMGR_TSessionEvent_InvalidValue;
			$this->Value = $RSMGR_TSessionEvent_InvalidValue;
		}
			
		function toString()
		{
			global $RSMGR_TSessionEvent_EnumValues;
			return $RSMGR_TSessionEvent_EnumValues[$this->Value];
		}
		
		function fromString($strValue)
		{
			global $RSMGR_TSessionEvent_EnumValues;
			foreach ($RSMGR_TSessionEvent_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}
			
			$this->Value = $RSMGR_TSessionEvent_InvalidValue;
		}
		
		function toInt()
		{
			return $this->Value;
		}
		
		function fromInt($intValue)
		{
			global $RSMGR_TSessionEvent_InvalidValue;
			global $RSMGR_TSessionEvent_EnumValues;
			if (array_key_exists($intValue, $RSMGR_TSessionEvent_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $RSMGR_TSessionEvent_InvalidValue;
		}
	}	
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('../tools/nel_message.php');

	class CRingSessionManagerWeb extends CCallbackClient
	{

		function scheduleSession($charId, $sessionType, $sessionTitle, $sessionDesc, $sessionLevel, $ruleType, $estimatedDuration, $subscriptionSlot, $animMode, $raceFilter, $religionFilter, $guildFilter, $shardFilter, $levelFilter, $language, $orientation, $subscriptionClosed, $autoInvite)
		{
			$msg = new CMessage;
			$msg->setName("SSS");


			$msg->serialUint32($charId);
				$msg->serialEnum($sessionType);
				$msg->serialString($sessionTitle);
				$msg->serialString($sessionDesc);
				$msg->serialEnum($sessionLevel);
				$msg->serialEnum($ruleType);
				$msg->serialEnum($estimatedDuration);
				$msg->serialUint32($subscriptionSlot);
				$msg->serialEnum($animMode);
				$msg->serialEnum($raceFilter);
				$msg->serialEnum($religionFilter);
				$msg->serialEnum($guildFilter);
				$msg->serialEnum($shardFilter);
				$msg->serialEnum($levelFilter);
				$msg->serialString($language);
				$msg->serialEnum($orientation);
				$msg->serialUint32($subscriptionClosed);
				$msg->serialUint32($autoInvite);
	
			return parent::sendMessage($msg);


		}

		function setSessionStartParams($charId, $sessionId, $initialIslandLocation, $initialEntryPointLocation, $initialSeason)
		{
			$msg = new CMessage;
			$msg->setName("SSSP");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($initialIslandLocation);
				$msg->serialUint32($initialEntryPointLocation);
				$msg->serialUint32($initialSeason);
	
			return parent::sendMessage($msg);


		}

		function getSessionInfo($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("GSI");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function updateSessionInfo($charId, $sessionId, $sessionTitle, $plannedDate, $sessionDesc, $sessionLevel, $estimatedDuration, $subscriptionSlot, $raceFilter, $religionFilter, $guildFilter, $shardFilter, $levelFilter, $subscriptionClosed, $autoInvite, $language, $orientation)
		{
			$msg = new CMessage;
			$msg->setName("USS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialString($sessionTitle);
				$msg->serialUint32($plannedDate);
				$msg->serialString($sessionDesc);
				$msg->serialEnum($sessionLevel);
				$msg->serialEnum($estimatedDuration);
				$msg->serialUint32($subscriptionSlot);
				$msg->serialEnum($raceFilter);
				$msg->serialEnum($religionFilter);
				$msg->serialEnum($guildFilter);
				$msg->serialEnum($shardFilter);
				$msg->serialEnum($levelFilter);
				$msg->serialUint32($subscriptionClosed);
				$msg->serialUint32($autoInvite);
				$msg->serialString($language);
				$msg->serialEnum($orientation);
	
			return parent::sendMessage($msg);


		}

		function cancelSession($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("CANSS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function startSession($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("STSS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function closeSession($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("CLSS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function closeEditSession($charId)
		{
			$msg = new CMessage;
			$msg->setName("CLESS");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function addFriendCharacter($userId, $friendCharId)
		{
			$msg = new CMessage;
			$msg->setName("AFC");


			$msg->serialUint32($userId);
				$msg->serialUint32($friendCharId);
	
			return parent::sendMessage($msg);


		}

		function removeFriendCharacter($userId, $friendCharId)
		{
			$msg = new CMessage;
			$msg->setName("RFC");


			$msg->serialUint32($userId);
				$msg->serialUint32($friendCharId);
	
			return parent::sendMessage($msg);


		}

		function addBannedCharacter($userId, $bannedCharId)
		{
			$msg = new CMessage;
			$msg->setName("ABC");


			$msg->serialUint32($userId);
				$msg->serialUint32($bannedCharId);
	
			return parent::sendMessage($msg);


		}

		function removeBannedCharacter($userId, $bannedCharId)
		{
			$msg = new CMessage;
			$msg->setName("RBC");


			$msg->serialUint32($userId);
				$msg->serialUint32($bannedCharId);
	
			return parent::sendMessage($msg);


		}

		function addFriendDMCharacter($userId, $friendDMCharId)
		{
			$msg = new CMessage;
			$msg->setName("AFDC");


			$msg->serialUint32($userId);
				$msg->serialUint32($friendDMCharId);
	
			return parent::sendMessage($msg);


		}

		function removeFriendDMCharacter($userId, $friendDMCharId)
		{
			$msg = new CMessage;
			$msg->setName("RFDC");


			$msg->serialUint32($userId);
				$msg->serialUint32($friendDMCharId);
	
			return parent::sendMessage($msg);


		}

		function setKnownCharacterComments($userId, $charId, $relation, $comments)
		{
			$msg = new CMessage;
			$msg->setName("SKCC");


			$msg->serialUint32($userId);
				$msg->serialUint32($charId);
				$msg->serialString($relation);
				$msg->serialString($comments);
	
			return parent::sendMessage($msg);


		}

		function inviteCharacter($ownerCharId, $sessionId, $invitedCharId, $charRole)
		{
			$msg = new CMessage;
			$msg->setName("IC");


			$msg->serialUint32($ownerCharId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($invitedCharId);
				$msg->serialEnum($charRole);
	
			return parent::sendMessage($msg);


		}

		function removeInvitedCharacter($ownerCharId, $sessionId, $removedCharId)
		{
			$msg = new CMessage;
			$msg->setName("RIC");


			$msg->serialUint32($ownerCharId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($removedCharId);
	
			return parent::sendMessage($msg);


		}

		function subscribeSession($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("SBS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function unsubscribeSession($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("USBS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function joinSession($charId, $sessionId, $clientApplication)
		{
			$msg = new CMessage;
			$msg->setName("JSS");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialString($clientApplication);
	
			return parent::sendMessage($msg);


		}

		function joinMainland($charId, $clientApplication)
		{
			$msg = new CMessage;
			$msg->setName("JML");


			$msg->serialUint32($charId);
				$msg->serialString($clientApplication);
	
			return parent::sendMessage($msg);


		}

		function joinEditSession($charId, $clientApplication)
		{
			$msg = new CMessage;
			$msg->setName("JES");


			$msg->serialUint32($charId);
				$msg->serialString($clientApplication);
	
			return parent::sendMessage($msg);


		}

		function hibernateEditSession($charId)
		{
			$msg = new CMessage;
			$msg->setName("HES");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function getShards($charId)
		{
			$msg = new CMessage;
			$msg->setName("GSH");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function kickCharacter($ownerCharId, $sessionId, $kickedCharId)
		{
			$msg = new CMessage;
			$msg->setName("KC");


			$msg->serialUint32($ownerCharId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($kickedCharId);
	
			return parent::sendMessage($msg);


		}

		function unkickCharacter($ownerCharId, $sessionId, $unkickedCharId)
		{
			$msg = new CMessage;
			$msg->setName("UKC");


			$msg->serialUint32($ownerCharId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($unkickedCharId);
	
			return parent::sendMessage($msg);


		}

		function inviteGuild($charId, $sessionId, $guildId)
		{
			$msg = new CMessage;
			$msg->setName("IG");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($guildId);
	
			return parent::sendMessage($msg);


		}

		function removeInvitedGuild($charId, $sessionId, $guildId)
		{
			$msg = new CMessage;
			$msg->setName("RIG");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($guildId);
	
			return parent::sendMessage($msg);


		}

		function setScenarioInfo($charId, $sessionId, $title, $numPlayer, $playType)
		{
			$msg = new CMessage;
			$msg->setName("SSCI");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialString($title);
				$msg->serialUint32($numPlayer);
				$msg->serialString($playType);
	
			return parent::sendMessage($msg);


		}

		function addJournalEntry($charId, $sessionId, $entryType, $text)
		{
			$msg = new CMessage;
			$msg->setName("AJE");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialString($entryType);
				$msg->serialString($text);
	
			return parent::sendMessage($msg);


		}

		function setPlayerRating($charId, $sessionId, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection)
		{
			$msg = new CMessage;
			$msg->setName("SPR");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
				$msg->serialUint32($rateFun);
				$msg->serialUint32($rateDifficulty);
				$msg->serialUint32($rateAccessibility);
				$msg->serialUint32($rateOriginality);
				$msg->serialUint32($rateDirection);
	
			return parent::sendMessage($msg);


		}
	

		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
			case "RET":
				$this->invokeResult_skel($message);
				break;
			case "SSSR":
				$this->scheduleSessionResult_skel($message);
				break;
			case "SIR":
				$this->sessionInfoResult_skel($message);
				break;
			case "JSSR":
				$this->joinSessionResult_skel($message);
				break;
			case "JSSRE":
				$this->joinSessionResultExt_skel($message);
				break;
			case "GSHR":
				$this->getShardsResult_skel($message);
				break;
			default:
				return false;
			}

			return true;
		}
		

		function invokeResult_skel(&$message)
		{
			$message->serialUint32($userId);
				$message->serialUint32($resultCode);
				$message->serialString($resultString);
				
			$this->invokeResult($userId, $resultCode, $resultString);
		}

		function scheduleSessionResult_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($sessionId);
				$message->serialUInt8($result);
				$message->serialString($resultString);
				
			$this->scheduleSessionResult($charId, $sessionId, $result, $resultString);
		}

		function sessionInfoResult_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($sessionId);
				
			$raceFilter = new RSMGR_TRaceFilter;
	$message->serialEnum($raceFilter);
				
			$religionFilter = new RSMGR_TReligionFilter;
	$message->serialEnum($religionFilter);
				
			$guildFilter = new RSMGR_TGuildFilter;
	$message->serialEnum($guildFilter);
				
			$shardFilter = new RSMGR_TShardFilter;
	$message->serialEnum($shardFilter);
				
			$levelFilter = new RSMGR_TLevelFilter;
	$message->serialEnum($levelFilter);
				$message->serialUint32($subscriptionClosed);
				$message->serialUint32($autoInvite);
				$message->serialString($language);
				
			$orientation = new RSMGR_TSessionOrientation;
	$message->serialEnum($orientation);
				$message->serialString($description);
				
			$this->sessionInfoResult($charId, $sessionId, $raceFilter, $religionFilter, $guildFilter, $shardFilter, $levelFilter, $subscriptionClosed, $autoInvite, $language, $orientation, $description);
		}

		function joinSessionResult_skel(&$message)
		{
			$message->serialUint32($userId);
				$message->serialUint32($sessionId);
				$message->serialUint32($result);
				$message->serialString($shardAddr);
				
			$participantStatus = new RSMGR_TSessionPartStatus;
	$message->serialEnum($participantStatus);
				
			$this->joinSessionResult($userId, $sessionId, $result, $shardAddr, $participantStatus);
		}

		function joinSessionResultExt_skel(&$message)
		{
			$message->serialUint32($userId);
				$message->serialUint32($sessionId);
				$message->serialUint32($result);
				$message->serialString($shardAddr);
				
			$participantStatus = new RSMGR_TSessionPartStatus;
	$message->serialEnum($participantStatus);
				$message->serialUint32($securityCheckForFastDisconnection);
				
			$this->joinSessionResultExt($userId, $sessionId, $result, $shardAddr, $participantStatus, $securityCheckForFastDisconnection);
		}

		function getShardsResult_skel(&$message)
		{
			$message->serialUint32($userId);
				$message->serialString($result);
				
			$this->getShardsResult($userId, $result);
		}


		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class 
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////
		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error

		function invokeResult($userId, $resultCode, $resultString)
		{
		}

		// result is : 0 : session have been created fine
		//             1 : invalid session type
		//             2 : invalid level
		//             3 : unknown character
		//             4 : not used
		//             5 : invalid access type
		//             6 : invalid rule type
		//             7 : invalid duration
		//             8 : invalid user
		//             9 : free trial account can't create anim session
		//             10 : user is ban from ring anim session

		function scheduleSessionResult($charId, $sessionId, $result, $resultString)
		{
		}

		// session info result (anim)

		function sessionInfoResult($charId, $sessionId, $raceFilter, $religionFilter, $guildFilter, $shardFilter, $levelFilter, $subscriptionClosed, $autoInvite, $language, $orientation, $description)
		{
		}

		// Return the result of the session joining attempt
		// If join is ok, the shardAddr contain <ip:port> of the
		// Front end that waits for the player to come in and the.
		// participation mode for the character (editor, animator or player).
		// If ok, the web must return a page with a lua script.
		// that trigger the action handler 'on_connect_to_shard' :
		// <lua>runAH(nul, "on_connect_to_shard", "cookie=cookieValue|fsAddr=shardAddr|mode=participantStatus");<lua>
		// result : 0 : ok the client can join the session
		//          1 : char not found
		//          2 : session not found
		//          3 : no session participant for this character (not used for a mainland shard)
		//          4 : can't find session server (not used for a mainland shard)
		//          5 : shard hosting session is not reachable
		//          6 : nel user info not found
		//          7 : ring user not found
		//          8 : welcome service rejected connection request
		//          9 : session service shutdown (not used for a mainland shard)
		//         10 : no mainland shard found (joinMainland only)
		//         11 : internal error
		//         12 : failed to request for access permission
		//         13 : can't find access permission for user and domain
		//         14 : Welcome service is closed for you
		//         15 : Session is not open
		//         16 : User banned from ring
		//         17 : Newcomer flag missmatch
		//         18 : Can't find session log to validate session access
		//         19 : Can't find scenario info to validate session access
		//         20 : Scenario is not allowed to free trial players

		function joinSessionResult($userId, $sessionId, $result, $shardAddr, $participantStatus)
		{
		}

		// See joinSessionResult.
		// Adds a security code.

		function joinSessionResultExt($userId, $sessionId, $result, $shardAddr, $participantStatus, $securityCheckForFastDisconnection)
		{
		}

		// Return the list of online shards on which the user is allowed to connect,
		// and their current dynamic attributes. Other attributes (e.g. names)
		// can be queried from the database. Offline shards are the ones in the database
		// of the same domain but not listed in the result.
		// Then the client will have to call joinShard to connect on an online shard.

		function getShardsResult($userId, $result)
		{
		}

	}
?>
<?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('../tools/nel_message.php');

	class CSessionBrowserServerWeb extends CCallbackClient
	{

		function authenticate($userId, $cookie)
		{
			$msg = new CMessage;
			$msg->setName("AUTH");


			$msg->serialUint32($userId);
				$msg->serialUint32($cookie);
	
			return parent::sendMessage($msg);


		}

		function getSessionList($charId)
		{
			$msg = new CMessage;
			$msg->setName("GSL");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function getCharList($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("GCL");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function inviteCharacterByName($charId, $invitedCharName)
		{
			$msg = new CMessage;
			$msg->setName("ICBN");


			$msg->serialUint32($charId);
				$msg->serialUint32($invitedCharName);
	
			return parent::sendMessage($msg);


		}

		function getMyRatings($charId, $sessionId)
		{
			$msg = new CMessage;
			$msg->setName("GMSR");


			$msg->serialUint32($charId);
				$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function getSessionAverageScores($sessionId)
		{
			$msg = new CMessage;
			$msg->setName("GSAS");


			$msg->serialUint32($sessionId);
	
			return parent::sendMessage($msg);


		}

		function getScenarioAverageScores($md5)
		{
			$msg = new CMessage;
			$msg->setName("GSCAS");


			$msg->serialUint32($md5);
	
			return parent::sendMessage($msg);


		}

		function getRingRatings($charId)
		{
			$msg = new CMessage;
			$msg->setName("GRR");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function getRingPoints($charId)
		{
			$msg = new CMessage;
			$msg->setName("GRP");


			$msg->serialUint32($charId);
	
			return parent::sendMessage($msg);


		}

		function forwardToDss($charId, $msg)
		{
			$msg = new CMessage;
			$msg->setName("DSS_FW");


			$msg->serialUint32($charId);
				$msg->serialUint32($msg);
	
			return parent::sendMessage($msg);


		}
	

		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
			case "SL":
				$this->sessionList_skel($message);
				break;
			case "CL":
				$this->charList_skel($message);
				break;
			case "PR":
				$this->playerRatings_skel($message);
				break;
			case "SAS":
				$this->sessionAverageScores_skel($message);
				break;
			case "SCAS":
				$this->scenarioAverageScores_skel($message);
				break;
			case "RR":
				$this->ringRatings_skel($message);
				break;
			case "RP":
				$this->ringPoints_skel($message);
				break;
			default:
				return false;
			}

			return true;
		}
		

		function sessionList_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($sessions);
				
			$this->sessionList($charId, $sessions);
		}

		function charList_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($sessionId);
				$message->serialUint32($characters);
				
			$this->charList($charId, $sessionId, $characters);
		}

		function playerRatings_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($scenarioRated);
				$message->serialUint32($rateFun);
				$message->serialUint32($rateDifficulty);
				$message->serialUint32($rateAccessibility);
				$message->serialUint32($rateOriginality);
				$message->serialUint32($rateDirection);
				
			$this->playerRatings($charId, $scenarioRated, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection);
		}

		function sessionAverageScores_skel(&$message)
		{
			$message->serialUint32($scenarioRated);
				$message->serialUint32($rateFun);
				$message->serialUint32($rateDifficulty);
				$message->serialUint32($rateAccessibility);
				$message->serialUint32($rateOriginality);
				$message->serialUint32($rateDirection);
				$message->serialUint32($rrpTotal);
				
			$this->sessionAverageScores($scenarioRated, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection, $rrpTotal);
		}

		function scenarioAverageScores_skel(&$message)
		{
			$message->serialUint32($scenarioRated);
				$message->serialUint32($rateFun);
				$message->serialUint32($rateDifficulty);
				$message->serialUint32($rateAccessibility);
				$message->serialUint32($rateOriginality);
				$message->serialUint32($rateDirection);
				$message->serialUint32($rrpTotal);
				
			$this->scenarioAverageScores($scenarioRated, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection, $rrpTotal);
		}

		function ringRatings_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($authorRating);
				$message->serialUint32($AMRating);
				$message->serialUint32($masterlessRating);
				
			$this->ringRatings($charId, $authorRating, $AMRating, $masterlessRating);
		}

		function ringPoints_skel(&$message)
		{
			$message->serialUint32($charId);
				$message->serialUint32($ringPoints);
				$message->serialUint32($maxRingPoints);
				
			$this->ringPoints($charId, $ringPoints, $maxRingPoints);
		}


		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class 
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////
		// Return the list of available session

		function sessionList($charId, $sessions)
		{
		}

		// Return the list of player characters in the session

		function charList($charId, $sessionId, $characters)
		{
		}

		// Return current player rating of the current session scenario

		function playerRatings($charId, $scenarioRated, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection)
		{
		}

		// Return average scores of a session

		function sessionAverageScores($scenarioRated, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection, $rrpTotal)
		{
		}

		// Return average scores of a scenario

		function scenarioAverageScores($scenarioRated, $rateFun, $rateDifficulty, $rateAccessibility, $rateOriginality, $rateDirection, $rrpTotal)
		{
		}

		// Return the author rating, the AM rating and the Masterless rating

		function ringRatings($charId, $authorRating, $AMRating, $masterlessRating)
		{
		}

		// Return the ring points of the character

		function ringPoints($charId, $ringPoints, $maxRingPoints)
		{
		}

	}
?>
