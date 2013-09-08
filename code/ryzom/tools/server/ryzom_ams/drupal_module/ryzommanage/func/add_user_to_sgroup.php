<?php

function add_user_to_sgroup(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    if(WebUsers::isLoggedIn()){
        
        if( Ticket_User::isAdmin(unserialize($_SESSION['ticket_user'])) &&  isset($_POST['target_id'])){
            $name = filter_var($_POST['Name'],FILTER_SANITIZE_STRING);
            $id = filter_var($_POST['target_id'],FILTER_SANITIZE_NUMBER_INT);
            $user_id = WebUsers::getId($name);
            if ($user_id != ""){
                if (Ticket_User::constr_ExternId($user_id)->getPermission()>1){
                    $result['RESULT_OF_ADDING'] = Support_Group::addUserToSupportGroup($user_id, $id);
                }else{
                    $result['RESULT_OF_ADDING'] = "NOT_MOD_OR_ADMIN";
                }
            
            }else{
                $result['RESULT_OF_ADDING'] = "USER_NOT_EXISTING";
            }
            $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
            $result['no_visible_elements'] = 'FALSE';
            $result['username'] = $_SESSION['user'];
            //global $SITEBASE;
            //require_once($SITEBASE . 'inc/show_sgroup.php');
            //$result= array_merge($result, show_sgroup());
            //helpers :: loadtemplate( 'show_sgroup', $result);
            if (Helpers::check_if_game_client()) {
                header("Location: ".$INGAME_WEBPATH."?page=show_sgroup&id=".$id);
            }else{
                header("Location: ".$WEBPATH."?page=show_sgroup&id=".$id);
            }
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