<?php

function show_sgroup(){
    //if logged in
    if(WebUsers::isLoggedIn()){
        if( WebUsers::isAdmin()){
            if( isset($_GET['id'])){
                
                $id = filter_var($_GET['id'],FILTER_SANITIZE_STRING);
                
                $group = Support_Group::getGroup($id);
                $result['groupsname'] = $group->getName();
                $result['grouplist'] = Gui_Elements::make_table(Support_Group::getGroups(), Array("getSGroupId","getName","getTag"), Array("sGroupId","name","tag"));
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