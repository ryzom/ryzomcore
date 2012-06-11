private function parseRuleset() {
			$this->ruleset_parsed = $this->ruleset;
			#WORKPAD:####
			/*
			Trigger:
				by value
				(by event)

			Sources:
				XML
				valuecache
				ring_open
				(Achievement Service)
					(Mirror Service)
			
			Keywords:
				VALUE
				GRANT:EVENT player_death
				DENY:TIMER 3600
				RESET
				RESET_ALL
				UNLOCK
				UNLOCK_ALL

				IF
				SCRIPT
				MSG
			
			VALUE dappers = c_money
			IF(dappers >= 5000) {
				GRANT
			}

			VALUE sum = c_cache:sum
			IF(sum > 1000) {
				GRANT
			}	
			
			VALUE tmp = c_fame[scorchers]
			IF(tmp == 0) {
				DENY:3600
			}
			
			VALUE x = c_pos_x
			VALUE y = c_pos_y
			SCRIPT inside(x,y) {
				IF(MSG == "Majestic Garden") {
					GRANT
				}
			}

			EVENT player_death
			ON player_death {
				UNLOCK
			}

			EVENT region_changed
			ON region_changed {
				IF(MSG == "Majestic Garden") {
					GRANT
				}
			}
			*/
			#############
			

			#VALUE var = name;
			$match = array();
			preg_match_all("#VALUE ([a-zA-Z0-9_]) ?= ?([a-zA-Z0-9_]);#", $this->ruleset_parsed, $match,PREG_PATTERN_ORDER);
			foreach($match[0] as $key=>$elem) {
				$tmp = '$'.$match[1][$key].' = $_DATA->getData("VALUE","'.$match[2][$key].'",$user);\n';
				$tmp .= 'if($'.$match[1][$key].' == ) {\n';
				$tmp .= 'ERROR\n';
				$tmp .= '}\n';
				$this->ruleset_parsed = str_replace($elem,$tmp,$this->ruleset_parsed);
			}

			
			#IF(statement) {	}
			$match = array();
			preg_match_all("#IF ?\(([^\)]*)\) ?{#", $this->ruleset_parsed, $match,PREG_PATTERN_ORDER);
			foreach($match[0] as $key=>$elem) {
				$tmp = 'if() {\n';
				$this->ruleset_parsed = str_replace($elem,$tmp,$this->ruleset_parsed);
			}


			SCRIPT script(a,r,g,s) {
				MSG
			}

			#EVENT name;
			$match = array();
			preg_match_all("#EVENT ([^;]*);#", $this->ruleset_parsed, $match,PREG_PATTERN_ORDER);
			foreach($match[0] as $key=>$elem) {
				$tmp = '';
				$this->ruleset_parsed = str_replace($elem,$tmp,$this->ruleset_parsed);
			}
			
			ON name {
				MSG
			}

			#GRANT;
			#GRANT:EVENT name;
			#GRANT:TIMER seconds;
			$match = array();
			preg_match_all("#GRANT:?([^;]*);#", $this->ruleset_parsed, $match,PREG_PATTERN_ORDER);
			foreach($match[0] as $key=>$elem) {
				$tmp = '$this->grant("'.$match[1][$key].'");';
				$this->ruleset_parsed = str_replace($elem,$tmp,$this->ruleset_parsed);
			}

			#DENY;
			#DENY:EVENT name;
			#DENY:TIMER seconds;
			$match = array();
			preg_match_all("#DENY:?([^;]*);#", $this->ruleset_parsed, $match,PREG_PATTERN_ORDER);
			foreach($match[0] as $key=>$elem) {
				$tmp = '$this->deny("'.$match[1][$key].'");';
				$this->ruleset_parsed = str_replace($elem,$tmp,$this->ruleset_parsed);
			}

			#RESET;
			#RESET_ALL;
			#UNLOCK;
			#UNLOCK_ALL;
			$this->ruleset_parsed = str_replace("RESET_ALL;",'$this->reset_all();',$this->ruleset_parsed);
			$this->ruleset_parsed = str_replace("RESET;",'$this->reset_();',$this->ruleset_parsed);
			$this->ruleset_parsed = str_replace("UNLOCK_ALL;",'$this->unlock_all();',$this->ruleset_parsed);
			$this->ruleset_parsed = str_replace("UNLOCK;",'$this->unlock();',$this->ruleset_parsed);
		}