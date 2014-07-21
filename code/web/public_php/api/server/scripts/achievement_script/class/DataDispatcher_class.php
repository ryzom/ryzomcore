<?php
	/*
	 * The DataDispatcher is used to route data to atoms that requested it.
	 * At first atoms will be registered. Later, when data comes in, it will be passed on to them.
	 */

	class DataDispatcher {
		private $value;
		private $entity;
		private $event;

		function DataDispatcher() {
			$this->value = array();
			$this->entity = array();
			$this->event = array();
		}

		//registering atoms

		function registerValue($name,$callback) {
			if(!is_array($this->value[$name])) {
				$this->value[$name] = array();
			}
			$this->value[$name][] = $callback;
		}

		function registerEntity($name,$callback) {
			if(!is_array($this->entity[$name])) {
				$this->entity[$name] = array();
			}
			$this->entity[$name][] = $callback;
		}

		function registerEvent($name,$callback) {
			if(!is_array($this->event[$name])) {
				$this->event[$name] = array();
			}
			$this->event[$name][] = $callback;
		}

		//unregistering atoms

		function unregisterValue($name,$callback) {
			$res = array_search($callback,$this->value[$name],true);
			if($res !== false) {
				unset($this->value[$name][$res]);
			}
		}

		function unregisterEntity($name,$callback) {
			$res = array_search($callback,$this->entity[$name],true);
			if($res !== false) {
				unset($this->entity[$name][$res]);
			}
		}

		function unregisterEvent($name,$callback) {
			$res = array_search($callback,$this->event[$name],true);
			if($res !== false) {
				unset($this->event[$name][$res]);
			}
		}

		//dispatching data

		function dispatchValue($key,$val) {
			if(is_array($this->value[$key])) {
				foreach($this->value[$key] as $callback) {
					$callback->call($val);
				}
			}
		}

		function dispatchEntity($key,$val) {
			if(is_array($this->entity[$key])) {
				foreach($this->entity[$key] as $callback) {
					$callback->call($val);
				}
			}
		}

		function dispatchEvent($key,$val) {
			if(is_array($this->event[$key])) {
				foreach($this->event[$key] as $callback) {
					$callback->call($val);
				}
			}
		}
	}
?>