<?php

function show_sgroup(){
    //if logged in
    if(WebUsers::isLoggedIn()){
        if(Ticket_User::isAdmin($_SESSION['ticket_user'])){
            if( isset($_GET['id'])){
                
                //['target_id'] holds the id of the group!
                $result['target_id'] = filter_var($_GET['id'], FILTER_SANITIZE_NUMBER_INT);
                
                if(isset($_GET['delete'])){
                    $delete_id = filter_var($_GET['delete'], FILTER_SANITIZE_NUMBER_INT);
                    $result['delete'] = Support_Group::deleteUserOfSupportGroup( $delete_id, $result['target_id']  );
                    header("Location: index.php?page=show_sgroup&id=" . $result['target_id']);
                    exit;
                    
                }
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