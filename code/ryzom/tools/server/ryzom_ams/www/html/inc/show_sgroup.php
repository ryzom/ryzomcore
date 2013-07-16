<?php

function show_sgroup(){
    //if logged in
    if(WebUsers::isLoggedIn()){
        if( WebUsers::isAdmin()){
            if( isset($_GET['id'])){
                
                $result['target_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
                $group = Support_Group::getGroup($result['target_id']);
               
                $result['groupsname'] = $group->getName();
                $result['userlist'] = Gui_Elements::make_table(Support_Group::getAllUsersOfSupportGroup($result['target_id']), Array("getTUserId","getPermission","getExternId"), Array("tUserId","permission","externId"));
                $i = 0;
                foreach( $result['userlist'] as $user){
                    $result['userlist'][$i]['name'] = WebUsers::getUsername($user['externId']);
                    $i++;
                }
                return $result;
            
            }else{
                
                //ERROR: No page specified!
                $_SESSION['error_code'] = "404";
                header("Location: index.php?page=error");
                exit;
            }
                
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