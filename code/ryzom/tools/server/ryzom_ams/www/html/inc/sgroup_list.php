<?php

function sgroup_list(){
    //if logged in  & queue id is given
    if(WebUsers::isLoggedIn()){
        if( WebUsers::isAdmin()){
            
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