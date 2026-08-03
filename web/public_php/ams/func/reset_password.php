<?php

function reset_password(){
    // filter all data; passwords must not go through FULL_SPECIAL_CHARS —
    // that rewrites &, <, etc. and makes real passwords fail to match
    $email = filter_var($_GET["email"], FILTER_SANITIZE_EMAIL);
    $user = isset($_GET["user"]) && is_string($_GET["user"]) ? $_GET["user"] : '';
    $key = isset($_GET["key"]) && is_string($_GET["key"]) ? $_GET["key"] : '';

    $password = isset($_POST['NewPass']) ? (string)$_POST['NewPass'] : '';
    $confirmpass = isset($_POST['ConfirmNewPass']) ? (string)$_POST['ConfirmNewPass'] : '';

    $target_id = WebUsers::getId($user);
    if (!$target_id){
        global $WEBPATH;
        $_SESSION['error_code'] = "403";
        header("Cache-Control: max-age=1");
        header("Location: ".$WEBPATH."?page=error");
        throw new SystemExit();
    }
    $webUser = new WebUsers($target_id);
    $email_id = WebUsers::getIdFromEmail($email);
    if( ($email_id == $target_id) && WebUsers::verifyPasswordResetToken($target_id, $webUser->getHashedPass(), $key) ){
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
            throw new SystemExit();
        }
        // only the params the form needs, encoded for the action url
        $GETString = '?user=' . rawurlencode($user)
            . '&email=' . rawurlencode($email)
            . '&key=' . rawurlencode($key);
        $result['getstring'] = $GETString;
        $result['prevNewPass'] = '';
        $result['prevConfirmNewPass'] = '';
        $result['no_visible_elements'] = 'TRUE';
        helpers :: loadtemplate( 'reset_password', $result);
        throw new SystemExit();

    }
    global $WEBPATH;
    $_SESSION['error_code'] = "403";
    header("Cache-Control: max-age=1");
    header("Location: ".$WEBPATH."?page=error");
    throw new SystemExit();
}
