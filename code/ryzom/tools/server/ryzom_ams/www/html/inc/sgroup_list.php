<?php

function sgroup_list(){
    //if logged in 
    if(WebUsers::isLoggedIn()){
        if( WebUsers::isAdmin()){
            
            if(isset($_GET['delete'])){
                $delete_id = filter_var($_GET['delete'], FILTER_SANITIZE_NUMBER_INT);
                $result['delete'] = Support_Group::deleteSupportGroup( $delete_id);
                header("Location: index.php?page=sgroup_list");
                exit;             
            }
            $result['grouplist'] = Gui_Elements::make_table(Support_Group::getGroups(), Array("getSGroupId","getName","getTag"), Array("sGroupId","name","tag"));
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