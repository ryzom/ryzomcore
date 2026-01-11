<?php

/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

include_once (RYAPI_PATH.'/common/ryformBases.php');
 
class ryVar extends basicRyForm {
	public $formName = '';
	public $varName = '';
	public $varValue = '';
		
	function getFormDefs() {
		return array(
			new ryFormDef('varName', DEF_TYPE_TEXT),
			new ryFormDef('varValue', DEF_TYPE_TEXT),
		);
	}

	function getHtmlRepr() {
		return $this->varName.' =&gt; '.$this->varValue;
	}
}

class ryForm {

	private $name = '';
	private $defines = array();
	private $template = '';

	static private $forms;
	static private $ryformsIcons = array();
	
	function __construct($dir, $name) {
		$this->name = $name;
		self::$forms[$dir.'/'.$name] = $this;
	}

	static function addRyFormIcon($ryform_name, $icon) {
		self::$ryformsIcons[$ryform_name] = $icon;
	}

	function addDefine($def) {
		$this->defines[$def->name] = $def;
	}
	
	function setTemplate($template) {
		$this->template = $template;
	}
	
	function getTemplate() {
		return $this->template;
	}
		
	function addValue($name, $value) {
		if (array_key_exists($name, $this->defines))
			$this->defines[$name]->value = $value;
		else
			return false;
		return true;
	}

	function addExtraValues($name, $value) {
		if (array_key_exists($name, $this->defines))
			$this->defines[$name]->extraValues = $value;
		else
			return false;
		return true;
	}

	function getForm($params) {
		
		if (isset($params['ryform_action']) && $params['ryform_action']) {
			$res = $this->doAction($params);
			return $res;
		}

		if (@$params['validate'] == $this->name)
			return array(DATA_FORM_VALUES, $_POST);
		
		if (isset($params['ryform_parent']))
			$parent_ryform_name = $params['ryform_parent'].'/';
		else
			$parent_ryform_name = '';
		
		$action =_url(ryzom_get_params(), array('validate' => $this->name));
		$ret = '';
		$ret .= '<form action="'.$action.'" method="POST">'."\n";

		if (!$this->getTemplate()) {
			$ret .= '	<table width="100%" cellpadding="0" cellspacing="0">'."\n";
			$ret .= '	'._s('t header', '<td height="24px">'._t('parameter').'</td><td>'._t('value').'</td><td></td>')."\n";
			$tmpl = '';
		} else {
			$tmpl = $this->getTemplate();
		}
		
		$i = 0;

		foreach ($this->defines as $def_id => $def) {
			if ($def->name == 'name')
				$def->name = '_name';
					
			$deffullname = $def->name;
			$url_params = ryzom_get_params();
			$type = $def->type;
			$infos = $def->infos;
			$value = ($def->value !== NULL)?$def->value:$def->defaultValue;
			
			if (!is_object($value) && !is_array($value))
				$str_value = _h(strval($value));
			else
				$str_value = '';
			
			if ($def->hidden)
				$type = DEF_TYPE_HIDDEN;

			$hidden = false;
			$input = '';
			switch ($type) {
								
				case DEF_TYPE_HIDDEN:
					$input = '<input type="hidden" name="'.$def->name.'" value="'.$str_value.'" />'."\n";
					$hidden = true;
					break;
					
				case DEF_TYPE_TEXT:
					$input = '<input style="width:250px" type="text" name="'.$def->name.'" value="'.$str_value.'" size="25'.(_user()->ig?'0':'').'" />';
					break;
				case DEF_TYPE_NAMEID:
					$input = '<input style="width:250px" type="text" name="'.$def->name.'" value="'.getNameId($str_value).'" size="25'.(_user()->ig?'0':'').'" />';
					break;
					
				case DEF_TYPE_ID:
				case DEF_TYPE_INT:
				case DEF_TYPE_FLOAT:
					$input = '<input style="width:100px"  type="text" name="'.$def->name.'" value="'.$str_value.'" size="10'.(_user()->ig?'0':'').'" />';
					break;
					
				case DEF_TYPE_BOOL:
					$input = '<select name="'.$def->name.'">'."\n";
					if ($value)
						$input .= '<option selected="selected" value="on">'._t('yes').'</option>'."\n".'<option value="off">'._t('no').'</option>';
					else
						$input .= '<option value="on">'._t('yes').'</option>'."\n".'<option selected="selected" value="off">'._t('no').'</option>';
					$input .= '</select>';
					break;
				
				case DEF_TYPE_OPTION_FUNCTION:
				case DEF_TYPE_OPTION:
					if ($type == DEF_TYPE_OPTION)
						$options = $def->params;
					else {
						if (is_array($def->defaultValue))
							$options = call_user_func_array($def->params, $def->defaultValue);
						else
							$options = call_user_func($def->params);
					}
					$input = '<select name="'.$def->name.'">'."\n";						
					$options_html = '';
					foreach ($options as $option => $text) {
						$option = strval($option);
						if ($option && $option[0] === '<' && $option[1] === '/')
							$options_html .=  '</optgroup>';
						else if ($option && $option[0] === '<')
							$options_html .=  '<optgroup label="'.$text.'">';
						else {
							if ($value !== NULL  and (strval($value) == $option))
								$options_html .= '<option selected="selected" value="'.$option.'">'.$text.'</option>'."\n";
							else
								$options_html .= '<option value="'.$option.'">'.$text.'</option>'."\n";
						}
					}
					$input .= $options_html;
					$input .= '</select>';
					break;

				case DEF_TYPE_COMBO_FUNCTION:
				case DEF_TYPE_COMBO:
					if ($type == DEF_TYPE_COMBO)
						$options = $def->params;
					else {
						if (is_array($def->defaultValue))
							$options = call_user_func_array($def->params, $def->defaultValue);
						else
							$options = call_user_func($def->params);
					}
					if (_user()->ig) {
						// TODO : try to do it with lua
					} else { // HTML 4
						$input .= '<input style="width:400px"  type="text" id="'.$def->name.'" name="'.$def->name.'"  value="'.$str_value.'" />
							<select onChange="combo(this, \''.$def->name.'\')" onMouseOut="comboInit(this, \''.$def->name.'\')" >';
						$options_html = '';
						$have_selected = false;
						foreach ($options as $option => $text) {
							if ($option && $option[0] === '<' && $option[1] === '/')
								$options_html .=  '</optgroup>';
							else if ($option && $option[0] === '<')
								$options_html .=  '<optgroup label="'.$text.'">';
							else {

								if ($value and ($value == $option)) {
									$have_selected = true;
									$options_html .= '<option selected="selected" value="'.$option.'">'.$text.'</option>'."\n";
								} else
									$options_html .= '<option value="'.$option.'">'.$text.'</option>'."\n";
							}
						}
						if ($have_selected)
							$input .=  '<option value=""></option>';
						else
							$input .=  '<option selected="selected"  value=""></option>';
						$input .= $options_html;
						$input .= '</select>';
					}
					break;
				
				case DEF_TYPE_TEXTAREA:
					if (!$value)
						$value = "";
					$input = '<pre>'.($type == DEF_TYPE_BBCODE?'<font color="orange">- BBCode -</font><br />':'').'<textarea name="'.$def->name.'" rows="3">'._h($value).'</textarea></pre>';
					break;
					
				case DEF_TYPE_TRAD:
					$base = '';
					$param = $def->name;
					$value = array_merge(array('en' => '', 'fr' => '', 'de' => '', 'ru' => '', 'es' => ''), $value);
					$base = ryzom_get_param('select_base', '');
					$edit = $display = $input_header = '';
					foreach (array('en', 'fr', 'de', 'ru', 'es') as $lang) {
						if (_user()->lang == $lang)
							$edit =  _i($lang == 'en'?API_URL.'data/img/lang/us.png':API_URL.'data/img/lang/'.$lang.'.png').' <textarea style="width: 90%" rows="3" name="'.$param.'['.$lang.']">'._h($value[$lang]).'</textarea>';
						if ((!$base && $value[$lang]) || $base == $lang) {
							$base = $lang;
							$display = strtoupper($lang).' = <font color="orange">'.str_replace("\n", '<br />', _h($value[$lang])).'</font>';
						}
						$input .= '<input type="hidden" name="'.$param.'['.$lang.']" value="'.$value[$lang].'" />';						
						$input_header .= _l(_i($lang == 'en'?API_URL.'data/img/lang/us.png':API_URL.'data/img/lang/'.$lang.'.png'), $url_params, array('select_base' => $lang)).'&nbsp;&nbsp;';
					}
					
					$input = $input_header.$input.' &nbsp; '.$display.'<br />'.$edit;
					break;

				case DEF_TYPE_RYFORM:
				case DEF_TYPE_RYFORMS_ARRAY:
						$savedRyform = $value;
						if (is_array($savedRyform)) {
							$to_clean = array();
							foreach ($savedRyform as $id => $ryform) {
								if (!is_object($ryform))
									$to_clean[] = $id;
							}
							foreach ($to_clean as $id)
								unset($savedRyform[$id]);
							$savedRyform = array_values($savedRyform);
						} else if (is_object($savedRyform)) {
							$savedRyform = array($savedRyform);
						} else
							$savedRyform = array();

						$input .= '<table width="100%" cellspacing="0" cellpadding="0" >';
						if ($savedRyform) {
							foreach ($savedRyform as $id => $ryform) {
								if (!is_object($ryform)) {
									p('!!! ERROR !!!', $ryform);
									continue;
								}
								$ryform->id = $id+1;
								if (!isset($ryform->formName) || !$ryform->formName)
									$ryform->formName = 'Element '.$id;
								if (count($savedRyform) > 1)
									$display_id = '<font size="12px" style="font-weight: bold; font-size: 14px" color="#FFAA55">'.strval(intval($id)+1).'</font>';
								else
									$display_id = '';
									
								$script_up = ($id != 0)?_l(_i('16/arrow_up', _t('up')), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.$id, 'ryform_action' => 'up')).' ':'';
								$script_down = ($id != count($savedRyform)-1)?_l(_i('16/arrow_down', _t('down')), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.$id, 'ryform_action' => 'down')).' ':'';
								
								$icon = (isset(self::$ryformsIcons[get_class($ryform)]))?self::$ryformsIcons[get_class($ryform)]:_i('32/brick');
								$input .= _s('t row '.($id%2), 
									'<td width="36px">'._l(($def->type == DEF_TYPE_RYFORM?_i('16/arrow_redo', _t('change')):_i('16/add', _t('add'))), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.strval(intval($id)+1), 'ryform_action' => 'list')).' '.$display_id.'</td>'.
									'<td width="10px">'.$script_up.$script_down.'</td>'.
									'<td ><table width="100%"><tr>
										<td width="40px">'.$icon.'</td>
										<td valign="middle" width="300px"><font size="12px" style="font-size: 13px;font-weight: bold;"  color="#FFAA55">'.
									_l($ryform->formName, $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.$id, 'ryform_action' => 'edit')).' '.
									'</font><br />'._t(get_class($ryform).'_short_description').'</td>
										<td align="left" valign="middle" bgcolor="#000000">'.$ryform->getHtmlRepr().'</td>
									</tr></table><td width="70px" align="right">'.
									_l(_i('16/script_edit', _t('edit')), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.$id, 'ryform_action' => 'edit')).' '.
									_l(_i('16/script_code', _t('edit_source')), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.$id, 'ryform_action' => 'source')).'&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'.
									_l(_i('16/script_delete', _t('del')), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':'.$id, 'ryform_action' => 'del')).'</td><td>&nbsp;</td>');
							}
						}
						$input .= '</table>';
						if (count($savedRyform) == 0 || $def->type != DEF_TYPE_RYFORM) {
							if (is_string($def->params))
								$infos = _l(_i('16/add', _t('add')), $url_params, array('new_ryform' => $def->params, 'ryform_name' => $parent_ryform_name.$deffullname.':0', 'ryform_action' => 'add'));
							else if (count($def->params) == 1)
								$infos = _l(_i('16/add', _t('add')), $url_params, array('new_ryform' => $def->params[0], 'ryform_name' => $parent_ryform_name.$deffullname.':0', 'ryform_action' => 'add'));
							else
							
								$infos = _l(_i('16/add', _t('add')), $url_params, array('ryform_name' => $parent_ryform_name.$deffullname.':0', 'ryform_action' => 'list'));
							
							if ($type == DEF_TYPE_RYFORMS_ARRAY)
								$infos .= '&nbsp;&nbsp;&nbsp;'._l(_i('16/application_form_add', _t('multiadd')), $url_params, array('ryform_name' => $deffullname, 'ryform_action' => 'list_multiadd'));
						}
					break;
							
				case DEF_TYPE_FUNCTION:
					if (is_array($def->defaultValue))
						list($result_type, $value) = call_user_func_array($def->params, $def->defaultValue);
					else
						list($result_type, $value) = call_user_func($def->params);
					if ($result_type == DATA_HTML_FORM) {
						return array(DATA_HTML_FORM, $value);
					} else {
						unset($url_params[$deffullname.'_action']);
						$input = $value;
					}
					break;
				
				default:
					$input = '<input type="hidden" name="'.$def->name.'" value="'.$value.'" />'.$value."\n";
					$hidden = true;
				
			}
							
			if ($hidden)
				$ret .= $input;
			else
			{
				if ($tmpl) {
					$tmpl = str_replace('{'.$def->name.'}', '<font '.(_user()->ig?'color="orange" size="11"':'style="color:orange;"').'>'._t($def->prefixTrad.$def->name).'</font>', $tmpl);
					$tmpl = str_replace('{'.$def->name.'.input}', $input, $tmpl);
					$tmpl = str_replace('{'.$def->name.'.infos}', $infos, $tmpl);
				} else
					$ret .= _s('t row '.strval($i % 2), '<td height="32px" width="200px">&nbsp;'.(!$def->optional?'*':'').($def->superAdmin?'##':'').($def->admin?'#':'')._t($def->prefixTrad.$def->name).'</td><td valign="center">'.$input.'</td><td>'.$infos.'</td>')."\n";
				$i++;
			}
		}
		
		if ($tmpl) {
			$tmpl = str_replace('{submit.input}', '<input type="submit" value="'._t('submit').'" />', $tmpl);
			$ret .= $tmpl;
			$ret .= '<table width="100%" cellspacing="0" cellpadding="0" ><tr>'._s('t row '.strval($i % 2), '<td height="32px">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'._t('required_fields').'</td><td></td><td align="middle"><input type="submit" value="'._t('submit').'" /></td>').'</tr></table>';
		} else {
			$ret .= _s('t row '.strval($i % 2), '<td height="32px">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'._t('required_fields').'</td><td></td><td><input type="submit" value="'._t('submit').'" /></td>');
			$ret .= '</table>';
		}
		$ret .= '</form><br />';
		return array(DATA_HTML_FORM, $ret."\n");
	}
	
	
	function doAction($url_params) {
		if (!$url_params['ryform_name'])
			return array(DATA_RYFORM_VALUE, array());
		$ret = '';
		$ryforms = explode('/', $url_params['ryform_name']);
		$this_ryform_name = array_shift($ryforms);
		list($ryform_name,$ryform_pos) = explode(':', $this_ryform_name);
		if (!isset($this->defines[$ryform_name]))
			return 'Bad ryform name';
		$def = $this->defines[$ryform_name];
		
		if ($ryforms) {
			$action = 'edit';
			$next_action = $url_params['ryform_action'];
		} else {
			$action = $url_params['ryform_action'];
			$next_action = '';
		}	

		switch ($action) {
			case 'list':
				if (isset($url_params['ryform_parent']) && $url_params['ryform_parent'])
					$ryform_parent = $url_params['ryform_parent'].'/';
				else
					$ryform_parent = '';
				$ret .= '<table cellpadding="0" cellspacing="0" width="100%">';
				foreach ($def->params as $id => $ryform) {
					if (is_array($ryform)) {
						$ret .= _s('t row 1', '<td height="28px" width="22px" bgcolor='.ryzom_get_color_style('section').'></td><td bgcolor='.ryzom_get_color_style('section').'>'.ryzom_font(_t('ryform_cat_'.$id), '', '12').'</td><td bgcolor='.ryzom_get_color_style('section').'></td>');
						foreach ($ryform as $subid => $subryform) {
							$ret .= _s('t row '.($subid%2), '<td width="22px" height="34px">'.self::$ryformsIcons[$subryform].'</td><td>&nbsp;&nbsp;'.
							_l(_t($subryform.'_short_description'), $url_params, array('ryform_name' => $ryform_parent.$url_params['ryform_name'], 'ryform_action' => 'add', 'new_ryform' => $subryform)).'</td><td>'.$subryform.'</td>');
						}
					} else 
						$ret .= _s('t row '.($id%2), '<td width="22px" height="34px">'.self::$ryformsIcons[$ryform].'</td><td>&nbsp;&nbsp;'.
						_l(_t($ryform.'_short_description'), $url_params, array('ryform_name' => $ryform_parent.$url_params['ryform_name'], 'ryform_action' => 'add', 'new_ryform' => $ryform)).'</td><td>'.$ryform.'</td>');
				}
				$ret .= '</table>';
				return array(DATA_HTML_FORM, $ret);
			break;

			case 'list_multiadd':
				// TODO 
				/*
				unset($url_params[$deffullname.'_action']);
				$ret .= '<table cellpadding="5"><tr>';
				foreach ($def->params as $ryform) {
					$ret .= '<td bgcolor="#000000">'._l($ryform, $url_params, array('ryform_action' => 'multiadd', 'new_ryform' => $ryform)).'</td>';
				}
				$ret .= '</tr></table>';
				return array(DATA_HTML_FORM, $ret);*/
			break;
			
			case 'add':
				$new_ryform = ryzom_get_param('new_ryform');
				$valid_ryform = false;
				if ($new_ryform) {
					
					if ((is_string($def->params) && $new_ryform == $def->params) || in_array($new_ryform, $def->params))
						$valid_ryform = true;
					else {
						foreach ($def->params as $param) {
							if (is_array($param) && in_array($new_ryform, $param))
								$valid_ryform = true;
						}
					}
					if (!$valid_ryform)
						return array(DATA_HTML_FORM, 'Bad ryform');
					
				} else {
					$new_ryform = $def->params;
				}
				$ryform = new $new_ryform($new_ryform, '');
				$ryform->preSerialization();
				if ($def->type != DEF_TYPE_RYFORM) {
					p($def->value);
					if (!is_array($def->value))
						$savedRyform = array($def->value);
					else
						$savedRyform = $def->value;
					if ($ryform_pos === 0) {
						$value = array_values(array_merge(array($ryform), $savedRyform));
					} else if ($ryform_pos !== NULL) {
						$begin = array_slice($savedRyform, 0, $ryform_pos);
						$end = array_slice($savedRyform, $ryform_pos, count($savedRyform)-$ryform_pos);
						$value = array_values(array_merge($begin, array($ryform), $end));
					} else
						$value[] = $ryform;
					p($ryform_name, $value);
					//return array(DATA_RYFORM_VALUE, array('stages' => array()));
					return array(DATA_RYFORM_VALUE, array($ryform_name => $value));
				} else {
					p($ryform_name, $ryform);
					return array(DATA_RYFORM_VALUE, array($ryform_name => $ryform));
				}
			break;
						
			case 'edit':
				$a_ryforms = $def->value;
				if (is_array($a_ryforms))
					$ryform = $a_ryforms[$ryform_pos];
				else
					$ryform = $a_ryforms;
				$ryform->postSerialization();
				$validate = isset($url_params['validate']) && $url_params['validate'];

				$form = new ryForm('', $def->name);
				foreach ($ryform->getFormDefs() as $form_def) {
					$form->addDefine($form_def);
					$name = $form_def->name;
					// Init form with ryfom values
					if (property_exists($ryform, $name)) {
						$form->addValue($form_def->name, $ryform->$name);
					}
				}
				foreach ($ryform->getFormDefsExtraValues() as $def_name => $extra_values)
					$form->addExtraValues($def_name, $extra_values);
				$form->setTemplate($ryform->getTemplate());

				list($result_type, $value) = $form->getForm(array('action' => $url_params['action'], 'script' => $url_params['script'], 'ryform_action' => $next_action, 'ryform_parent' => $this_ryform_name, 'ryform_name' => implode('/', $ryforms), 'validate' => $validate));
				if ($result_type == DATA_HTML_FORM) {
					return array(DATA_HTML_FORM, $value);
				} else {
					if ($result_type == DATA_FORM_VALUES)
						$value = $form->validateFormPost($value);
					$ryform->setFormParams($value);
					$ryform->preSerialization();

				}
				if (is_array($a_ryforms))
					$a_ryforms[$ryform_pos] = $ryform;
				else
					$a_ryforms = $ryform;
					
				$value = array($ryform_name => $a_ryforms);
				return array(DATA_RYFORM_VALUE, $value);
			break;

				
			case 'del':
				$id = $ryform_pos;
				p($def->value);
				if (!is_array($def->value))
					$def->value = array();
				else
					unset($def->value[$id]);
				$value = array_values($def->value);
				return array(DATA_RYFORM_VALUE, array($ryform_name => $value));
			break;
			
			case 'up':
				$a_ryforms = $def->value;
				if (!is_array($a_ryforms))
					ryzom_redirect(_url($url_params, array('ryform_action' => '')));

				$temp_ryform = $a_ryforms[$ryform_pos-1];
				$a_ryforms[$ryform_pos-1] = $a_ryforms[$ryform_pos];
				$a_ryforms[$ryform_pos] = $temp_ryform;
				$a_ryforms = array_values($a_ryforms);
				p($ryform_name, $a_ryforms);
				return array(DATA_RYFORM_VALUE, array($ryform_name => $a_ryforms));		
			break;
				
			case 'down':
				$a_ryforms = $def->value;
				if (!is_array($a_ryforms))
					ryzom_redirect(_url($url_params, array('ryform_action' => '')));

				$temp_ryform = $a_ryforms[$ryform_pos+1];
				$a_ryforms[$ryform_pos+1] = $a_ryforms[$ryform_pos];
				$a_ryforms[$ryform_pos] = $temp_ryform;
				$a_ryforms = array_values($a_ryforms);
				return array(DATA_RYFORM_VALUE, array($ryform_name => $a_ryforms));		
			break;
			
			case 'source':
				$a_ryforms = $def->value;
				if (is_array($a_ryforms))
					$ryform = $a_ryforms[$ryform_pos];
				else
					$ryform = $a_ryforms;
				$ryform->postSerialization();
								
				$form = new ryForm('', $def->name);
				$form->addDefine(new ryFormDef('ryform_source', DEF_TYPE_TEXTAREA, '', base64_encode(serialize($ryform))));
				$validate = isset($url_params['validate']) && $url_params['validate'];
				list($result_type, $value) = $form->getForm(array('ryform_action' => $next_action, 'ryform_parent' => $this_ryform_name, 'ryform_name' => implode('/', $ryforms), 'validate' => $validate));
				if ($result_type == DATA_HTML_FORM) {
					return array(DATA_HTML_FORM, $value);
				} else {
					if ($result_type == DATA_FORM_VALUES)
						$params = $form->validateFormPost($value);
					else
						$params = $value;
					$ryform = unserialize(base64_decode($params['ryform_source']));
					if (!is_object($ryform)) {
						unset($url_params['validate']);
						ryzom_redirect(_url($url_params, array('ryform_action' => '', 'message' => 'bad_paste')));
					}
					
					$is_valid = false;
					p($def->params);
					foreach ($def->params as $id => $ryform_class) {
						if (is_array($ryform_class)) {
							if (in_array(get_class($ryform), array_values($ryform_class)))
								$is_valid = true;
						} else if (get_class($ryform) == $ryform_class)
							$is_valid = true;
					}
					if (!$is_valid)  {
						p(get_class($ryform), $def->params);
						ryzom_redirect(_url($url_params, array($deffullname.'_action' => '', 'message' => 'not_valid_stage')));
						return;
					}					
					if (is_array($a_ryforms))
						$a_ryforms[$ryform_pos] = $ryform;
					else
						$a_ryforms = $ryform;
					$value = array($ryform_name => $a_ryforms);
					return array(DATA_RYFORM_VALUE, $value);
				}
				return;
			break;
		}
	
		return $ret;
	}

	function validateFormPost($params, $use_default=true) {
		$final = array();
		foreach ($this->defines as $def) {
			$name = $def->name;
			if ($def->name == 'name')
				$def->name = '_name';
			
			$type = $def->type;
			if ($def->hidden)
				$type = DEF_TYPE_HIDDEN;

			if (isset($params[$def->name])) {
				$value = $params[$def->name];
			} else if (!$use_default) {
				continue;
			} else {
				$value = '';
			}

			
			switch ($type) {
								
				case DEF_TYPE_HIDDEN:
				case DEF_TYPE_TEXT:
				case DEF_TYPE_OPTION:
				case DEF_TYPE_TEXTAREA:
					$final[$name] = $value;
					break;

				case DEF_TYPE_TRAD:
					if (!$value[_user()->lang] && $value['europeanunion'])
						$value[_user()->lang] = $value['europeanunion'];
					$final[$name] = $value;
					break;
				
				case DEF_TYPE_NAMEID:
					$final[$name] = cleanNameID($value);
					break;
					
				case DEF_TYPE_COMBO:
					$final[$name] = $value;
					break;
				
				case DEF_TYPE_ID:
				case DEF_TYPE_INT: // TODO
					$final[$name] = intval($value);
					break;
				case DEF_TYPE_FLOAT: // TODO
					$final[$name] = floatval($value);
					break;
					
				case DEF_TYPE_BOOL:
					$final[$name] = $value == 'on';
					break;
					
				case DEF_TYPE_RYFORM:
					if (is_array($value))
						$final[$name] = $value[0];
					break;
				case DEF_TYPE_RYFORMS_ARRAY:
					break;

				default:
					$final[$name] = $value;
				
			}
		}
		return $final;
	}

/*
	function reset() {
	
		// Clean all temp files
		$userDatas = _tools()->listAppDataFiles($this->dir);
		foreach ($userDatas as $userData) {
			if (substr($userData, 0, strlen($this->name)) == $this->name)
				_tools()->saveAppData($this->dir.'/'.$userData, NULL);
		}
	}*/
}

?>
