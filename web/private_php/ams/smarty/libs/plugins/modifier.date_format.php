<?php
/**
 * Smarty plugin
 *
 * @package    Smarty
 * @subpackage PluginsModifier
 */

if (!function_exists('_smarty_strftime_compat')) {
    /**
     * strftime() replacement for PHP 8.1+ compatibility.
     * Converts a strftime()-style format string and returns a formatted date using date().
     *
     * @param string   $format    strftime format string
     * @param int|null $timestamp Unix timestamp (default: current time)
     * @return string
     */
    function _smarty_strftime_compat($format, $timestamp = null)
    {
        if ($timestamp === null) {
            $timestamp = time();
        }
        $date_format = '';
        $len = strlen($format);
        $i = 0;
        while ($i < $len) {
            if ($format[$i] === '%' && $i + 1 < $len) {
                $spec = $format[$i + 1];
                switch ($spec) {
                    case 'a': $date_format .= 'D'; break;
                    case 'A': $date_format .= 'l'; break;
                    case 'b': $date_format .= 'M'; break;
                    case 'B': $date_format .= 'F'; break;
                    case 'h': $date_format .= 'M'; break;
                    case 'd': $date_format .= 'd'; break;
                    case 'e': $date_format .= sprintf('%2d', date('j', $timestamp)); break;
                    case 'G': $date_format .= 'o'; break;
                    case 'H': $date_format .= 'H'; break;
                    case 'I': $date_format .= 'h'; break;
                    case 'k': $date_format .= sprintf('%2d', date('G', $timestamp)); break;
                    case 'l': $date_format .= sprintf('%2d', date('g', $timestamp)); break;
                    case 'm': $date_format .= 'm'; break;
                    case 'M': $date_format .= 'i'; break;
                    case 'n': $date_format .= "\n"; break;
                    case 'p': $date_format .= 'A'; break;
                    case 'P': $date_format .= 'a'; break;
                    case 'r': $date_format .= 'h:i:s A'; break;
                    case 'R': $date_format .= 'H:i'; break;
                    case 'S': $date_format .= 's'; break;
                    case 't': $date_format .= "\t"; break;
                    case 'T': $date_format .= 'H:i:s'; break;
                    case 'u': $date_format .= 'N'; break;
                    case 'V': $date_format .= 'W'; break;
                    case 'w': $date_format .= 'w'; break;
                    case 'y': $date_format .= 'y'; break;
                    case 'Y': $date_format .= 'Y'; break;
                    case 'z': $date_format .= 'O'; break;
                    case 'Z': $date_format .= 'T'; break;
                    case 'D': $date_format .= 'm/d/y'; break;
                    case 'F': $date_format .= 'Y-m-d'; break;
                    case 'c': $date_format .= 'D M j H:i:s Y'; break;
                    case 'x': $date_format .= 'm/d/y'; break;
                    case 'X': $date_format .= 'H:i:s'; break;
                    case '%': $date_format .= '%'; break;
                    default: break;
                }
                $i += 2;
            } else {
                $ch = $format[$i];
                if (ctype_alpha($ch)) {
                    $date_format .= '\\' . $ch;
                } else {
                    $date_format .= $ch;
                }
                $i++;
            }
        }
        return date($date_format, $timestamp);
    }
}

/**
 * Smarty date_format modifier plugin
 * Type:     modifier
 * Name:     date_format
 * Purpose:  format datestamps via strftime
 * Input:
 *          - string: input date string
 *          - format: strftime format for output
 *          - default_date: default date if $string is empty
 *
 * @link   http://www.smarty.net/manual/en/language.modifier.date.format.php date_format (Smarty online manual)
 * @author Monte Ohrt <monte at ohrt dot com>
 *
 * @param string $string       input date string
 * @param string $format       strftime format for output
 * @param string $default_date default date if $string is empty
 * @param string $formatter    either 'strftime' or 'auto'
 *
 * @return string |void
 * @uses   smarty_make_timestamp()
 */
function smarty_modifier_date_format($string, $format = null, $default_date = '', $formatter = 'auto')
{
    if ($format === null) {
        $format = Smarty::$_DATE_FORMAT;
    }
    /**
     * require_once the {@link shared.make_timestamp.php} plugin
     */
    static $is_loaded = false;
    if (!$is_loaded) {
        if (!is_callable('smarty_make_timestamp')) {
            include_once SMARTY_PLUGINS_DIR . 'shared.make_timestamp.php';
        }
        $is_loaded = true;
    }
    if (!empty($string) && $string !== '0000-00-00' && $string !== '0000-00-00 00:00:00') {
        $timestamp = smarty_make_timestamp($string);
    } elseif (!empty($default_date)) {
        $timestamp = smarty_make_timestamp($default_date);
    } else {
        return;
    }
    if ($formatter === 'strftime' || ($formatter === 'auto' && strpos($format, '%') !== false)) {
        if (Smarty::$_IS_WINDOWS) {
            $_win_from = array(
                '%D',
                '%h',
                '%n',
                '%r',
                '%R',
                '%t',
                '%T'
            );
            $_win_to = array(
                '%m/%d/%y',
                '%b',
                "\n",
                '%I:%M:%S %p',
                '%H:%M',
                "\t",
                '%H:%M:%S'
            );
            if (strpos($format, '%e') !== false) {
                $_win_from[] = '%e';
                $_win_to[] = sprintf('%\' 2d', date('j', $timestamp));
            }
            if (strpos($format, '%l') !== false) {
                $_win_from[] = '%l';
                $_win_to[] = sprintf('%\' 2d', date('h', $timestamp));
            }
            $format = str_replace($_win_from, $_win_to, $format);
        }
        return _smarty_strftime_compat($format, $timestamp);
    } else {
        return date($format, $timestamp);
    }
}
