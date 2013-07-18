<?php

function add_user_to_sgroup(){
    
    if(WebUsers::isLoggedIn()){
        
        if( Ticket_User::isAdmin($_SESSION['ticket_user']) &&  isset($_POST['target_id'])){
            $name = filter_var($_POST['Name'],FILTER_SANITIZE_STRING);
            $id = filter_var($_POST['target_id'],FILTER_SANITIZE_NUMBER_INT);
            $user_id = WebUsers::getId($name);
            if ($user_id != ""){
                $result['RESULT_OF_ADDING'] = Support_Group::addUserToSupportGroup($user_id, $id);
            }else{
                $result['RESULT_OF_ADDING'] = "USER_NOT_EXISTING";
            }
            $result['permission'] = $_SESSION['ticket_user']->getPermission();
            $result['no_visible_elements'] = 'FALSE';
            $result['username'] = $_SESSION['user'];
            global $SITEBASE;
            require_once($SITEBASE . 'inc/show_sgroup.php');
            $result= array_merge($result, show_sgroup());
            helpers :: loadtemplate( 'show_sgroup', $result);
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