<?php

function sgroup_list(){
    //if logged in 
    if(WebUsers::isLoggedIn()){
        if(Ticket_User::isMod(unserialize($_SESSION['ticket_user']))){
            
            if(isset($_GET['delete']) && Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                $delete_id = filter_var($_GET['delete'], FILTER_SANITIZE_NUMBER_INT);
                $result['delete'] = Support_Group::deleteSupportGroup( $delete_id);
                header("Location: ams?page=sgroup_list");
                exit;             
            }
            if(Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
                $result['isAdmin'] = "TRUE";
            }
            $result['grouplist'] = Gui_Elements::make_table(Support_Group::getGroups(), Array("getSGroupId","getName","getTag","getGroupEmail"), Array("sGroupId","name","tag","groupemail"));
            return $result;
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