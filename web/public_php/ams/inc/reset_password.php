<?php

function reset_password(){
    $email = filter_var($_GET["email"], FILTER_SANITIZE_EMAIL);
    $user = isset($_GET["user"]) && is_string($_GET["user"]) ? $_GET["user"] : '';
    $key = isset($_GET["key"]) && is_string($_GET["key"]) ? $_GET["key"] : '';

    $target_id = WebUsers::getId($user);
    if (!$target_id){
        global $WEBPATH;
        $_SESSION['error_code'] = "403";
        header("Cache-Control: max-age=1");
        header("Location: ".$WEBPATH."?page=error");
        throw new SystemExit();
    }
    $webUser = new WebUsers($target_id);

    if( (WebUsers::getIdFromEmail($email) == $target_id)
        && WebUsers::verifyPasswordResetToken($target_id, $webUser->getHashedPass(), $key) ){
        // you are allowed on the page

        $pageElements = array();
        $pageElements['getstring'] = '?user=' . rawurlencode($user)
            . '&email=' . rawurlencode($email)
            . '&key=' . rawurlencode($key);

        return $pageElements;

    }else{
        global $WEBPATH;
        $_SESSION['error_code'] = "403";
        header("Cache-Control: max-age=1");
        header("Location: ".$WEBPATH."?page=error");
        throw new SystemExit();
    }
}
