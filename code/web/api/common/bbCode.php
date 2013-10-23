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

// setup bbCode formatter

bbCode::$ig = RYZOM_IG;

/**
 * Image proxy
 */
if(!defined('IMG_PROXY')){
	$url = 'http://'.$_SERVER['HTTP_HOST'].'/app_forum/tools/imageproxy.php';
	define('IMG_PROXY', $url);
}
if (!function_exists('proxy_image_url')) {
	function proxy_image_url($href, $attrs=''){
		return IMG_PROXY.'?'.($attrs != '' ? $attrs.'&' : '').'url='.urlencode($href);
	}
}


abstract class bbCodeParser {

    /**
     * @var bool
     */
    private $_ig;

    /**
     * @var array
     */
    private $tags_ignore;
    private $tags_block_open;
    private $tags_block_close;
    private $tags_ignore_depth;

    /**
     * @var array
     */
    private $open_tags;

    /**
     * @var string
     */
    private $last_closed_tag;

    /**
     * @var int
     */
    private $current_tag;

    /**
     * @var array
     */
    private $state;

    /**
     * @param bool $ig if true, use ingame markup
     */
    function __construct($ig) {
        $this->_ig = $ig;

        // ignore bbcode between these tags
        $this->tags_ignore = array(
            'noparse', 'code',
            'url', 'img', 'mail', 'page', 'forum', 'topic', 'post', 'wiki', 'time', 'date'
        );

        // these create block level html code, so '\n' or ' ' or '\t' around them needs to be cleared
        $this->tags_block_open = array('h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'quote', 'list', 'p');
        $this->tags_block_close = $this->tags_block_open;
        if ($this->_ig) {
            // ingame <p> is not block level when closing, so dont strip there
            $key = array_search('p', $this->tags_block_close, true);
            unset($this->tags_block_close[$key]);
        }

        $this->state = array();

        // reset internals
        $this->reset();
    }

    /**
     * Format bbcode tag
     *
     * @param string $tag   tag name
     * @param string $open  open markup
     * @param string $close close markup
     * @param string $attr  tag attributes
     * @param string $text  text between tags
     */
    abstract function format($tag, $open, $close, $attr, $text);

    /**
     * Wrapper to call Child->format(...)
     *
     * @param array $tag assoc array with tag info
     * @return string
     */
    function handle_tag($tag) {
        return $this->format($tag['tag'], $tag['open'], $tag['close'], $tag['attr'], $tag['text']);
    }

    /**
     * Reset internals
     */
    function reset() {
        $this->current_tag = 0;
        $this->tags_ignore_depth = 0;

        // 0'th position is used as result
        $this->open_tags = array(
            0 => array('tag' => '', 'open' => '', 'close' => '', 'attr' => '', 'text' => '')
        );

        $this->last_closed_tag = false;
    }

    /**
     * Save working state
     */
    private function _state_save() {
        $this->state[] = array($this->current_tag, $this->tags_ignore_depth, $this->open_tags, $this->last_closed_tag);
        $this->reset();
    }

    /**
     * Restore working state
     */
    private function _state_restore() {
        if (!empty($this->state)) {
            list($this->current_tag, $this->tags_ignore_depth, $this->open_tags, $this->last_closed_tag) = array_pop($this->state);
        }
    }

    /**
     * Main worker. Parse $text for bbCode tags
     *
     * NOTE: Text must already be safe for HTML, ie. treated with htmlspecialchars()
     *
     * @param string $text
     * @return string formatted string
     */
    function bbcode($text) {
        $text = str_replace("\r\n", "\n", $text);

        $split = preg_split('/(\[[a-zA-Z0-9_\/]*?(?:[= ].*?)?\])/', $text, null, PREG_SPLIT_DELIM_CAPTURE | PREG_SPLIT_NO_EMPTY);

        foreach ($split as $chunk) {
            if (substr($chunk, 0, 1) == '[' && substr($chunk, -1, 1) == ']') {
                if (substr($chunk, 0, 2) == '[/') {
                    $this->close($chunk);
                } else {
                    $this->open($chunk);
                }
            } else {
                $this->text($chunk);
            }
        }

        return $this->result();
    }

    /**
     * Push tag with args to stack
     * Do not strip whitespace because tag might be invalid
     *
     * @param string $chunk full tag string, eg. [tag=attr]
     */
    function open($chunk) {
        list($tag, $attr) = $this->split_params($chunk);

        // test for [noparse]
        if ($this->tags_ignore_depth > 0) {
            $this->text($chunk);
        } else {
            $this->current_tag++;
            // remember tag, attributes and complete string that was used in markup
            $this->open_tags[$this->current_tag] = array('tag' => $tag, 'attr' => $attr, 'open' => $chunk, 'close' => '', 'text' => '');
        }

        if (in_array($tag, $this->tags_ignore)) {
            $this->tags_ignore_depth++;
        }
    }

    /**
     * Close tag and call tag handler to format output
     *
     * @param $chunk full tag string, eg. [/tag]
     */
    function close($chunk) {
        // extract tag name from [/name]
        $tag = strtolower(substr($chunk, 2, -1));

        if ($this->tags_ignore_depth > 0 && in_array($tag, $this->tags_ignore)) {
            $this->tags_ignore_depth--;
        }

        // stack underrun
        if ($this->current_tag < 0) {
            $this->text($chunk);
            return;
        }

        // ignore block
        if ($this->tags_ignore_depth > 0) {
            $this->text($chunk);
            return;
        }

        // tag mismatch
        if ($this->open_tags[$this->current_tag]['tag'] !== $tag) {
            // try to find first open tag for this
            $key = false;
            for ($i = $this->current_tag - 1; $i > 0; $i--) {
                if (isset($this->open_tags[$i]['tag']) && $this->open_tags[$i]['tag'] === $tag) {
                    $key = $i;
                    break;
                }
            }
            if ($key === false) {
                $this->text($chunk);
                return;
            }

            // tag is open so we need to 'rewind' a bit
            for ($i = $this->current_tag; $i > $key; $i--) {
                $tmp_tag = $this->pop_stack();
                $this->text($tmp_tag['open'] . $tmp_tag['text']);
            }
        }

        // close tag
        $open = $this->pop_stack();

        // handle bbcode
        $open['close'] = $chunk;

        $block_level = false;
        if (in_array($tag, $this->tags_block_open)) {
            $block_level = true;
            // for block level element, trim whitespace from inside tag
            // [tag]<ws>...text...<ws>[/tag]
            $open['text'] = $this->trim_ws($open['text']);
        }
        $result = $this->handle_tag($open);

        // strip whitespace from text before tag 'text...<ws>[tag]'
        if ($block_level) {
            $ts = $this->rtrim_ws($this->open_tags[$this->current_tag]['text']);
            $this->open_tags[$this->current_tag]['text'] = $ts;
        }

        $this->text($result);

        $this->last_closed_tag = $open['tag'];
    }

    function text($text) {
        // strip whitespace after closing '[/tag]<ws>...text'
        if (in_array($this->last_closed_tag, $this->tags_block_close)) {
            $text = $this->ltrim_ws($text);
        }
        $this->open_tags[$this->current_tag]['text'] .= $text;

        $this->last_closed_tag = false;
    }

    function result() {
        // close tags that are still open
        while ($this->current_tag > 0) {
            $open = $this->pop_stack();

            if ($this->tags_ignore_depth > 0) {
                $this->tags_ignore_depth--;
                // need to reparse text that's after ignore tag
                $this->_state_save();
                $text = $open['open'] . $this->bbcode($open['text']);
                $this->_state_restore();
            } else {
                // tag was not closed proprely, include start tag with result
                $text = $open['open'] . $open['text'];
            }

            $this->text($text);
        };

        return $this->open_tags[0]['text'];
    }

    /**
     * Pop tag and text from stack and return them
     *
     * @return array [0] = tag, [1] = text
     */
    function pop_stack() {
        // remove from stack
        $open = $this->open_tags[$this->current_tag];
        unset($this->open_tags[$this->current_tag]);
        $this->current_tag--;

        return $open;
    }

    /**
     * Trim from end of string
     * 'text...\s{0,}\n{1}\s{0,}'
     *
     * @param string $ts
     * @return string
     */
    function rtrim_ws($ts){
        // we want to get rid of all spaces/tabs, but only single \n, so rtrim($ts, " \t\n\r") would not work
        $ts = rtrim($ts, " \t");
        if (substr($ts, -1, 1) === "\n") {
            $ts = substr($ts, 0, -1);
            $ts = rtrim($ts, " \t");
        }
        return $ts;
    }

    /**
     * Trim from start of string
     * '\s{0,}\n{1}...text'
     *
     * @param string $ts
     * @return string
     */
    function ltrim_ws($ts){
        // we want to get rid of all spaces/tabs, but only single \n, so ltrim($ts, " \t\n\r") would not work
        $ts = ltrim($ts, " \t");
        if (substr($ts, 0, 1) === "\n") {
            $ts = substr($ts, 1);
        }
        return $ts;
    }

    /**
     * Trim from both sides
     * '\s{0,}\n{1}...text...\s{0,}\n{1}\s{0,}
     *
     * @param string $ts
     * @return string
     */
    function trim_ws($ts){
        $ts = $this->ltrim_ws($ts);
        $ts = $this->rtrim_ws($ts);
        return $ts;
    }

    /**
     * Extract tag parameters from [tag=params] or [tag key1=val1 key2=val2]
     *
     * @param type $tag
     * @return type
     */
    function split_params($chunk) {
        if (substr($chunk, 0, 1) == '[') {
            $b = '\[';
            $e = '\]';
        } else {
            $b = '';
            $e = '';
        }
        //                          [1]                   [2]       [3]
        if (preg_match('/^' . $b . '([\*a-zA-Z0-9]*?)' . '(=| )' . '(.*?)' . $e . '$/', $chunk, $match)) {
            $tagName = strtolower($match[1]);
            if ($match[2] == '=') {
                // = means single parameter
                $tagParam = $match[3];
            } else {
                // <space> means multiple parameters
                $tagParam = array();
                $args = preg_split('/[ ]/', $match[3], null, PREG_SPLIT_NO_EMPTY);
                foreach ($args as $arg) {
                    $pairs = explode('=', $arg);
                    // preg_replace will remove possible quotes around value
                    if (isset($pairs[1])) {
                        $tagParam[strtolower($pairs[0])] = preg_replace('@("|\'|)(.*?)\\1@', '$2', $pairs[1]);
                    } else {
                        $tagParam[] = preg_replace('@("|\'|)(.*?)\\1@', '$2', $pairs[0]);
                    }
                }
            }
        } else {
            if (substr($chunk, 0, 1) == '[' && substr($chunk, -1, 1) == ']') {
                $chunk = substr($chunk, 1, -1);
            }
            $tagName = strtolower($chunk);
            $tagParam = '';
        }
        return array($tagName, $tagParam);
    }

}

class bbCode extends bbCodeParser {
	static $legacy_sync = 1348956841;
	static $legacy_shard = array(
		'ani' => 2363920179,
		'lea' => 2437578274,
		'ari' => 2358620001,
	);

    static $ig = false;
    static $timezone = 'UTC';
    static $clock12h = false;
    static $shardid = false;
    static $lang = 'en';
    static $disabledTags = array();
    //
    const COLOR_P = '#d0d0d0'; // normal text
    //
    const COLOR_BBCODE_TAG = '#444444';

    static function bbDisabled($tag) {
        return in_array(strtolower($tag), self::$disabledTags);
    }

    static function getFontSize($value) {
        $size = 16;
        switch (strtolower($value)) {
            case '1': case 'xx-small': $size = 9;   break;
            case '2': case 'x-small' : $size = 10;  break;
            case '3': case 'small'   : $size = 13;  break;
            case '4': case 'medium'  : $size = 16;  break;
            case '5': case 'large'   : $size = 18;  break;
            case '6': case 'x-large' : $size = 24;  break;
            case '7': case 'xx-large': $size = 32;  break;
            //case '8': case 'smaller' : break;
            //case '9': case 'larger'  : break;
        }
        return $size;
    }

    static function bb_noparse($code) {
        return preg_replace(array('/\[/', '/\]/'), array('&#91;', '&#93;'), $code);
    }

    static function bb_code($code) {
        return '<pre>' . self::bb_noparse($code) . '</pre>';
    }

    static function bb_list($list) {
        $result = '';
        $list = str_replace("\n[", '[', $list);
        $result = '<ul>' . preg_replace('/\s*\[\*\]\s*/is', "</li><li>", $list) . '</li></ul>';
        return preg_replace('#<ul>\s*</li>#is', '<ul>', $result);
    }

    static function bb_quote($author, $text) {
        if (self::$ig) {
            // prevents [color] tag to take over color
            $author = self::bb_color(self::COLOR_P, $author);
            $text = self::bb_color(self::COLOR_P, $text);
            // left/right border, top/bottom border height
            $l = '<td width="1" bgcolor="#888888" height="1"></td>';
            $r = '<td width="1" bgcolor="#888888" height="1"></td>';
            return // 98% gives bit padding on the right
                '<table width="98%" cellpadding="0" cellspacing="0" border="0">' .
                '<tr><td width="1" height="5"></td><td></td><td></td></tr>' . // top padding - no border
                '<tr>' . $l . '<td bgcolor="#888888"></td>' . $r . '</tr>' . // top border
                '<tr>' . $l . '<td bgcolor="#000000" l_margin="5" height="3"></td>' . $r . '</tr>' . // author top padding
                '<tr>' . $l . '<td bgcolor="#000000" l_margin="5">' . $author . '</td>' . $r . '</tr>' . // author
                '<tr>' . $l . '<td bgcolor="#000000" l_margin="5" height="2"></td>' . $r . '</tr>' . // author bottom padding
                '<tr>' . $l . '<td bgcolor="#555555" l_margin="10" height="3"></td>' . $r . '</tr>' . // quote top padding
                '<tr>' . $l . '<td bgcolor="#555555" l_margin="10">' . $text . '</td>' . $r . '</tr>' . // quote
                '<tr>' . $l . '<td bgcolor="#555555" l_margin="10" height="2"></td>' . $r . '</tr>' . // quote bottom padding
                '<tr>' . $l . '<td bgcolor="#888888"></td>' . $r . '</tr>' . // bottom border
                '<tr><td width="1" height="8"></td><td></td><td></td></tr>' . // bottom padding - no border
                '</table>';
        } else {
            return '' .
                '<div class="post-quote">' .
                '<cite>' . $author . '</cite>' .
                '<blockquote>' . $text . '</blockquote>' .
                '</div>';
        }
    }

    static function bb_h($nr, $color, $text) {
        $tag = 'h' . $nr;

        if (self::$ig) {
            if ($color != '') {
                $text = '<font color="' . $color . '">' . $text . '</font>';
            }
            return '<' . $tag . '>' . $text . '</' . $tag . '>';
        } else {
            if ($color != '') {
                $style = ' style="color: ' . $color . ';"';
            } else {
                $style = '';
            }
            return '<' . $tag . $style . '>' . $text . '</' . $tag . '>';
        }
    }

    static function bb_url($href, $text) {
        // &quot;http://..../&quot; remove &quot; if present
        if (substr($href, 0, 6) == '&quot;') {
            if (substr($href, -6) == '&quot;') {
                $href = substr($href, 6, -6);
            } else {
                $href = substr($href, 6);
            }
        }

        if ($href == '')
            $href = $text;
        if ($text == '') {
            $text = $href;
            $text = wordwrap($text, 65, ' ', true);
        }

        $disable = self::bbDisabled('url');
        // if not disabled and in ryzom and is proper url (<scheme>://<host>/)
        if (!$disable && self::$ig) {
            $url = @parse_url(strtolower($href));
            $disable = true;
            if (!empty($url['scheme']) && !empty($url['host'])) {
                if (in_array($url['scheme'], array('http', 'https'))) {
                    if (in_array($url['host'], array('app.ryzom.com'))) {
                        if (empty($url['query']) || stripos($url['query'], 'redirect') === false) {
                            // http://atys.ryzom.com/
                            // and does not contain redirect
                            // - allow url in game browser
                            $disable = false;
                        }
                    }
                }
            } // !empty
        }// isRYZOM

        if ($disable) {
            // empty href will give proper link color without 'underline' - perfect for 'disabled' links
            if ($href == '') {
                $text = '<a href="">' . $text . '</a>';
            } else {
                $href = wordwrap($href, 65, ' ', true);
                $text = wordwrap($text, 65, ' ', true);
                $text = '<a href="">' . $text . '</a> ' . self::bb_color(self::COLOR_BBCODE_TAG, '(' . $href . ')');
            }
            return $text;
        }

        // make sure http:// (or ftp:// or mailto:// etc is present), if not, add it
        if (!preg_match('#://#', $href)) {
            $href = 'http://' . $href;
        }

        return sprintf('<a href="%s"' . (self::$ig ? '' : ' target="_blank"') . '>%s</a>', $href, $text);
    }

	static function bb_img($attr, $href) {
        if (self::bbDisabled('img')) {
            return self::bb_noparse('[img]' . $href . '[/img]');
        }
        // $href is treated with htmlspecialchars() so any & in url is &amp;
        $href = str_replace('&amp;', '&', $href);

	    // images from current server directly
        if ($attr=='src' || strstr($href, $_SERVER['HTTP_HOST']) !== false){
			return '<img src="' . $href . '" />';
        }
        $url = proxy_image_url($href);
        return '<a href="' . $url . '&no_proxy=1"><img src="' . $url . '" /></a>';
    }

    static function bb_banner($lang, $ckey) {
        // $lang and $ckey should already be escaped for HTML, so urlencode() in here would double escape them
        // - channel it thru image proxy. proxy does caching better and uses '304 Not Modified' status
        $src = 'http://atys.ryzom.com/api/banner.php?ckey=' . $ckey . '&langid=' . $lang . '&size=500';
        return self::bb_img('', $src);
    }

    static function bb_mail($user) {
        $url = 'http://' . $_SERVER['HTTP_HOST'] . '/app_mail/?page=compose/to/' . urlencode($user);
        return '<a href="' . $url . '">' . $user . '</a>';
    }

    static function bb_profile($ptype, $pname) {
        // types from app_profile
        $types = array('user', 'player', 'npc', 'fauna', 'entity', 'source');
        $ptype = array_search($ptype, $types, true);
        // if type not found, then fall back to player
        if ($ptype === false)
            $ptype = 1;

        $url = 'http://' . $_SERVER['HTTP_HOST'] . '/app_profile/?ptype=' . intval($ptype) . '&pname=' . urlencode($pname);
        return '<a href="' . $url . '">' . $pname . '</a>';
    }

    static function bb_color($color, $str) {
        if ($color == '') {
            return $str;
        }

        if (self::$ig) {
            return '<font color="' . $color . '">' . $str . '</font>';
        } else {
            return '<span style="color: ' . $color . ';">' . $str . '</span>';
        }
    }

    static function bb_size($size, $str) {
        $size = self::getFontSize($size);

        if (self::$ig) {
            return '<font size="' . $size . 'px">' . $str . '</font>';
        } else {
            return '<span style="font-size: ' . $size . 'px;">' . $str . '</span>';
        }
    }

    static function bb_pre($str) {
        return '<pre>' . $str . '</pre>';
    }

    static function bb_p($str) {
        return '<p>' . $str . '</p>';
    }

    // Added by ulukyn. WebIg compatibility.
    static function bb_center($str) {
        if (self::$ig) {
            return '<table width="100%" cellpadding="0" cellspacing="0"><tr><td align="center" valign="middle">' . $str . '</td></tr></table>';
        } else {
            return '<div style="text-align: center;">' . $str . '</div>';
        }
    }

	/** Table format : (added by ulukyn)
	 * A1| A2|A3 
	 * B1| B2 |B3
	 * C1|C2 |C3
	 */
    static function bb_table($attr, $content) {
		$width = isset($attr['width'])?$attr['width']:'100%';
		$border = isset($attr['border'])?$attr['border']:'0';
		$bgcolor = isset($attr['bgcolor'])?' bgcolor="'.$attr['bgcolor'].'" ':'';
		$ret = '<table width="'.$width.'" border="'.$border.'" cellpadding="0" cellspacing="0" '.$bgcolor.' >';
		$lines =  explode("\n", $content);
		foreach ($lines as $line) {
			if ($line) {
				$ret .= '<tr>';
				$cols = explode('|', $line);
				foreach ($cols as $text) {
					if (!$text)
						continue;
					$params = array('valign' => 'middle');
					if ($text[0] == '#') {
						$paramsdef = explode(' ', $text);
						$paramlist = substr(array_shift($paramsdef), 1);
						$paramlist = explode(',', $paramlist);
						foreach ($paramlist as $p) {
							list($name, $value) = explode('=', $p);
							$params[ _h(str_replace('"', '', $name))] = _h(str_replace('"', '', $value));
						}
						if ($paramsdef)
							$text = implode(' ', $paramsdef);
					}
					$param_html = '';
					foreach ($params as $name => $value)
						$param_html .= $name.'="'.$value.'" ';
						
					if ($text && $text[0] == ' ' && $text[strlen($text)-1] == ' ')
						$align = 'center';
					else if ($text && $text[0] == ' ')
						$align = 'right';
					else
						$align = 'left';
					
					$ret .= '<td '.$param_html.' align="'.$align.'">'.$text.'</td>';
				}
				$ret .= '</tr>';
			}
		}
		
		$ret .= '</table>';
        return $ret;
    }


    static function bb_page_link($page, $txt) {
        if ($page == '') {
            $page = $txt;
        }
        $tmp = explode('/', $page);
        foreach ($tmp as $k => $v) {
            $tmp[$k] = urlencode($v);
        }
        $url = 'http://' . $_SERVER['HTTP_HOST'] . '/app_forum/?page=' . join('/', $tmp);
        return '<a href="' . $url . '">' . $txt . '</a>';
    }

    static function bb_forum_link($page, $id, $txt) {
        $page = $page . '/view/' . $id;
        if ($id == '') {
            $page.= $txt;
        }
        return self::bb_page_link($page, $txt);
    }

    // Added by Ulukyn
    static function bb_wiki_link($page, $txt) {
        $need_new_txt = false;
        if ($page == '') {
            $page = $txt;
            $need_new_txt = true;
        }

        if (substr($page, 0, 22) == 'http://atys.ryzom.com/')
            $url = 'http://atys.ryzom.com/start/app_wiki.php?page=' . substr($page, 21);
        else {
            $tmp = explode('/', $page);
            if (count($tmp) != 2) {
                return 'Syntax: [wiki]/[page], ex: en/Chronicles';
            } else {
                $wiki = $tmp[0];
                $page = $tmp[1];
            }
            if (self::$ig) {
                $url = 'http://atys.ryzom.com/start/app_wiki.php?page=/projects/pub' . $wiki . '/wiki/' . $page;
            }
            else
                $url = 'http://atys.ryzom.com/projects/pub' . $wiki . '/wiki/' . $page;
            if ($need_new_txt)
                $txt = 'WIKI [' . $page . ']';
        }
        return '<a href="' . $url . '"' . (self::$ig ? '' : ' target="_blank"') . '>' . $txt . '</a>';
    }

    static function bb_biu($tag, $txt) {
        $tag = strtolower($tag);
        if (self::$ig) {
            switch ($tag) {
                // FIXME: darken/lighter or tint current color
                case 'b': $txt = self::bb_color('white', $txt);
                    break;
                case 'i': $txt = self::bb_color('#ffffd0', $txt);
                    break;
                case 'u': $txt = '<a href="ah:">' . self::bb_color(self::COLOR_P, $txt) . '</a>';
                    break;
                default : $txt = self::bb_color(self::COLOR_BBCODE_TAG, $txt);
                    break; // fallback
            }
            return $txt;
        }

        switch ($tag) {
            case 'b': $tag = 'strong';
                break;
            case 'i': $tag = 'em';
                break;
            case 'u': $tag = 'u';
                break;
            default: $tag = 'span'; // fallback
        }
        return '<' . $tag . '>' . $txt . '</' . $tag . '>';
    }

    static function bb_date($attr, $txt) {
		$time = strtotime($txt);

		$shardid = isset($attr['shard']) ? $attr['shard'] : self::$shardid;
        if ($time === false || $shardid === false)
            return 'ERR:[' . $txt . ']';

		if (isset(self::$legacy_shard[$shardid])) {
			$tick = self::$legacy_shard[$shardid];
			if (self::$legacy_sync > $time) {
				// only modify game cycle when asked time is before sync
				$tick = ($time - self::$legacy_sync) * 10 + $tick;
			}
		} else {
			$tick = ryzom_time_tick($shardid);
			// tick is for NOW, adjust it to match time given
			$now  = time();
			$tick = ($time - $now) * 10 + $tick;
		}

        $rytime = ryzom_time_array($tick, $shardid);
        $txt = ryzom_time_txt($rytime, self::$lang);

        return $txt;
    }

    static function bb_lang($attr, $txt) {
		if (_user()->lang == $attr)
			return $txt;
		else
			return '';
    }
    
    static function bb_time($options, $txt) {
        $time = strtotime($txt);

        if ($time == 0) {
            return $txt;
        }

        $timezone = self::$timezone;

        $show_time = '';
        $show_date = '';
        $show_timer = '';

        if (is_array($options)) {
            foreach ($options as $key => $val) {
                switch ($key) {
                    case 'timezone':
                        // fix some timezones for php
                        switch ($val) {
                            case 'pst': // fall thru
                            case 'pdt': $val = 'US/Pacific';
                                break;
                        }
                        $timezone = $val;
                        break;
                    case 'date' :
                        $show_date = $val == 'off' ? false : $val;
                        break;
                    case 'time' :
                        $show_time = $val == 'off' ? false : $val;
                        break;
                    case 'timer':
                        $show_timer = $val == 'off' ? false : $val;
                        break;
                }//switch
            }//foreach
        }

        $ret = array();

        $old_timezone = date_default_timezone_get();
        @date_default_timezone_set($timezone);
        if ($show_date !== false) {
            $date = ryzom_absolute_time($time);
            //ryzom_absolute_time does not have year, so we need to add it
            $current_y = date('Y', time());
            $y = date('Y', $time);
            if ($y != $current_y) {
                $date.= ' ' . $y;
            }
            $ret[] = self::bb_color($show_date, $date);
        }
        if ($show_time !== false) {
            $fmtTime = self::$clock12h ? 'g:i:s a T' : 'H:i:s T';
            $ret[] = self::bb_color($show_time, date($fmtTime, $time));
        }
        date_default_timezone_set($old_timezone);

        if ($show_timer !== false) {
            if ($show_time === false && $show_date === false) {
                $f = '%s';
            } else {
                $f = '(%s)';
            }
            $ret[] = self::bb_color($show_timer, sprintf($f, ryzom_relative_time($time)));
        }

        return join(' ', $ret);
    }

    /**
     * This function is called by bbCodeParser class
     *
     * @see bbCodeParser::format
     */
    public function format($tag, $open, $close, $attr, $text) {
        // silly functions all have different parameters
        switch ($tag) {
            case 'noparse' :
                $result = self::bb_noparse($text);
                break;
            case 'code' :
                $result = self::bb_code($text);
                break;
            case 'quote' :
                $result = self::bb_quote($attr, $text);
                break;
            case 'h1' : // fall thru
            case 'h2' : // fall thru
            case 'h3' : // fall thru
            case 'h4' : // fall thru
            case 'h5' : // fall thru
            case 'h6' :
                $nr     = (int) substr($tag, -1);
                $color  = isset($attr['color']) ? $attr['color'] : '';
                $result = self::bb_h($nr, $color, $text);
                break;
            case 'color' :
                $result = self::bb_color($attr, $text);
                break;
            case 'size' :
                $result = self::bb_size($attr, $text);
                break;
            case 'list' :
                $result = self::bb_list($text);
                break;
            case 'img' :
                $result = self::bb_img($attr, $text);
                break;
            case 'banner' :
                $result = self::bb_banner($attr, $text);
                break;
            case 'pre' :
                $result = self::bb_pre($text);
                break;
            case 'p' :
                $result = self::bb_p($text);
                break;
            case 'table' :
                $result = self::bb_table($attr, $text);
                break;
            case 'center' :
                $result = self::bb_center($text);
                break;
            case 'url' :
                $result = self::bb_url($attr, $text);
                break;
            case 'mail' :
                $result = self::bb_mail($text);
                break;
            case 'profile' :
                $result = self::bb_profile($attr, $text);
                break;
            case 'page' :
                $result = self::bb_page_link($attr, $text);
                break;
            case 'forum' : // fall thru
            case 'topic' : // fall thru
            case 'post' :
                $result = self::bb_forum_link($tag, $attr, $text);
                break;
            case 'wiki' :
                $result = self::bb_wiki_link($attr, $text);
                break;
            case 'b' : // fall thru
            case 'i' : // fall thru
            case 'u' :
                $result = self::bb_biu($tag, $text);
                break;
            case 'time' :
                $result = self::bb_time($attr, $text);
                break;
            case 'date' :
                $result = self::bb_date($attr, $text);
                break;
            case 'lang' :
                $result = self::bb_lang($attr, $text);
                break;
            default :
                $result = $open . $text . $close;
                break;
        }
        return $result;
    }

    /**
     * Replaces some BBcode with HTML code
     *
     * NOTE: $text should be already escaped for HTML
     *
     * @param string $text html escaped input text
     * @param array $disabledTags
     */
    static function parse($text, $disabledTags = array()) {
        static $parser = null;
        if ($parser === null) {
            $parser = new self(self::$ig);
        }
        $parser->reset();

        self::$disabledTags = $disabledTags;
        return $parser->bbcode($text);
    }

}
