<?php

function userlist(){
    if(Ticket_User::isMod($_SESSION['ticket_user'])){
        $users = WebUsers::getUsers();
        $i = 0;
        $pageResult['userlist'] = Array();
        while($row = $users->fetch(PDO::FETCH_ASSOC)){
            $pageResult['userlist'][$i]['id'] = $row['UId'];
            $pageResult['userlist'][$i]['username'] = $row['Login'];
            $pageResult['userlist'][$i]['permission'] = $row['Permission'];
            $pageResult['userlist'][$i]['email'] = $row['Email'];
            $i++;
        }
        return $pageResult;
    }else{
        //ERROR: No access!
        $_SESSION['error_code'] = "403";
        header("Location: index.php?page=error");
        exit;
    }
}