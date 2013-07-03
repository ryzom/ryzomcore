<?php

function settings(){
    if(WebUsers::isLoggedIn()){
            //in case id-GET param set it's value as target_id, if no id-param is given, ue the session id.
            if(isset($_GET['id'])){
                if(($_GET['id'] != $_SESSION['id']) && (!WebUsers::isAdmin()) ){
                    //ERROR: No access!
                    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    exit;
                }else{
                    $result = WebUsers::getInfo($_GET['id']);
                    if(WebUsers::isAdmin() && ($_GET['id']!= $_SESSION['id'])){
                        $result['isAdmin'] = "TRUE";
                    }
                    $result['target_id'] = $_GET['id'];
                    $result['current_mail'] = WebUsers::getEmail($_GET['id']);
                }
            }else{
                $result = WebUsers::getInfo($_SESSION['id']);
                $result['target_id'] = $_SESSION['id'];
                $result['current_mail'] = WebUsers::getEmail($_SESSION['id']);
            }
            
            return $result;
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }
}