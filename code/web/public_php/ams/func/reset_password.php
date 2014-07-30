<?php

function reset_password(){
    //filter all data
    $email = filter_var($_GET["email"], FILTER_SANITIZE_EMAIL);
    $user = filter_var($_GET["user"], FILTER_SANITIZE_STRING);
    $key = filter_var($_GET["key"], FILTER_SANITIZE_STRING);
    
    $password = filter_var($_POST['NewPass'], FILTER_SANITIZE_STRING);
    $confirmpass = filter_var($_POST['ConfirmNewPass'], FILTER_SANITIZE_STRING);

    $target_id = WebUsers::getId($user);
    $webUser = new WebUsers($target_id);
    if( (WebUsers::getIdFromEmail($email) == $target_id) && (hash('sha512',$webUser->getHashedPass()) == $key) ){
        $params = Array( 'user' => $user, 'CurrentPass' => "dummy", 'NewPass' => $password, 'ConfirmNewPass' => $confirmpass, 'adminChangesOther' => true);
        $result = $webUser->check_change_password($params);
        if ($result == "success"){
            $result = array();
            $status = WebUsers::setPassword($user, $password);
            if($status == 'ok'){
                $result['SUCCESS_PASS'] = "OK";
            }else if($status == 'shardoffline'){
                $result['SUCCESS_PASS'] = "SHARDOFF";
            }
            $result['no_visible_elements'] = 'TRUE';
            helpers :: loadtemplate( 'reset_success', $result);
            exit;
        }    
        $GETString = "";
        foreach($_GET as $key => $value){
                $GETString = $GETString . $key . '=' . $value . "&";
        }		
        if($GETString != ""){
                $GETString = '?'.$GETString;
        }
        $result['getstring'] = $GETString;
        $result['prevNewPass'] = $password;
        $result['prevConfirmNewPass'] = $confirmpass;
        $result['no_visible_elements'] = 'TRUE';
        helpers :: loadtemplate( 'reset_password', $result);
        exit;
    
    }
}