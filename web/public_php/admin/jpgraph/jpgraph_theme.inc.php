<?php
//=======================================================================
// File:        JPGRAPH_THEME.INC.PHP
// Description: Class to define graph theme
// Created:     2010-09-29 
// Ver:         $Id: jpgraph_theme.inc.php 83 2010-10-01 11:24:19Z atsushi $
//
// Copyright (c) Asial Corporation. All rights reserved.
//========================================================================


// include Theme classes
foreach (glob(dirname(__FILE__) . '/themes/*.php') as $theme_class_script) {
  require_once($theme_class_script);
}

//===================================================
// CLASS 
// Description: 
//===================================================
abstract class Theme {
    protected $color_index;
    
    function __construct() {
        $this->color_index = 0;
    }
    /**
    * 
    */
    abstract function GetColorList();

    /**
    *
    */
    abstract function ApplyPlot($plot);


    /**
    *
    */   
    function SetupPlot($plot) {
        if (is_array($plot)) {
            foreach ($plot as $obj) {
                $this->ApplyPlot($obj);
            }
        } else {
            $this->ApplyPlot($plot);
        }
    }

    /**
    *
    */
    function ApplyGraph($graph) {

        $this->graph = $graph;
        $method_name = '';

        if (get_class($graph) == 'Graph') {
            $method_name = 'SetupGraph';
        } else {
            $method_name = 'Setup' . get_class($graph);
        }

        if (method_exists($this, $method_name)) {
            $this->$method_name($graph);
        } else {
            JpGraphError::RaiseL(30001, $method_name, $method_name); //Theme::%s() is not defined. \nPlease make %s(\$graph) function in your theme classs.
        }
    }

    /**
    *
    */
    function PreStrokeApply($graph) {
    }

    /**
    *
    */
    function GetThemeColors($num = 30) { 
        $result_list = array();

        $old_index = $this->color_index;
        $this->color_index = 0;
        $count = 0;
  
        $i = 0;
        while (true) {
            for ($j = 0; $j < count($this->GetColorList()); $j++) {
                if (++$count > $num) {
                    break 2;
                }
                $result_list[] = $this->GetNextColor();
            }
            $i++;
        }

        $this->color_index = $old_index;
        
        return $result_list;
    }

    /**
    *
    */
    function GetNextColor() {
        $color_list = $this->GetColorList();

        $color = null;
        if (isset($color_list[$this->color_index])) {
            $color = $color_list[$this->color_index];
        } else {
            $color_count = count($color_list);
            if ($color_count <= $this->color_index) {
                $color_tmp = $color_list[$this->color_index % $color_count];
                $brightness = 1.0 - intval($this->color_index / $color_count) * 0.2;
                $rgb = new RGB();
                $color = $color_tmp . ':' . $brightness;
                $color = $rgb->Color($color);
                $alpha = array_pop($color);
                $color = $rgb->tryHexConversion($color);
                if ($alpha) {
                    $color .= '@' . $alpha;
                }
            }
        }

        $this->color_index++;

        return $color;
    }

} // Class

?>
