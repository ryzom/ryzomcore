<?php

function add_sgroup(){
    
    if(WebUsers::isLoggedIn()){
        
        if( Ticket_User::isAdmin($_SESSION['ticket_user'])){
            $name = filter_var($_POST['Name'],FILTER_SANITIZE_STRING);
            $inner_tag = filter_var($_POST['Tag'], FILTER_SANITIZE_STRING);
            $tag = "[" . $inner_tag . "]";
            
            $result['RESULT_OF_ADDING'] = Support_Group::createSupportGroup($name, $tag);
            $result['permission'] = $_SESSION['permission'];
            $result['no_visible_elements'] = 'FALSE';
            $result['username'] = $_SESSION['user'];
            global $SITEBASE;
            require_once($SITEBASE . 'inc/sgroup_list.php');
            $result= array_merge($result, sgroup_list());
            helpers :: loadtemplate( 'sgroup_list', $result);
            exit;
            
        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            exit;
        }
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }

}