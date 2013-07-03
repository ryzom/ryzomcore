<?php

function userlist(){
    if(WebUsers::isAdmin()){
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
        print('no permission');
        exit;
    }
}