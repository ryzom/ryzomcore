<?php

function forgot_password(){

        $email = filter_var($_POST["Email"], FILTER_SANITIZE_EMAIL);

        $target_id = WebUsers::getIdFromEmail($email);
        if ($target_id == "FALSE"){
            //the email address doesn't exist.
            $result['prevEmail'] = $email;
            $result['EMAIL_ERROR'] = 'TRUE';
            $result['no_visible_elements'] = 'TRUE';
            helpers :: loadtemplate( 'forgot_password', $result);
            throw new SystemExit();
        }
        $webUser = new WebUsers($target_id);
        $target_username = $webUser->getUsername();
        $target_hashedPass = $webUser->getHashedPass();
        $hashed_key = hash('sha512',$target_hashedPass);

        if ( isset( $_COOKIE['Language'] ) ) {
            $lang = $_COOKIE['Language'];
        }else{
            global $DEFAULT_LANGUAGE;
            $lang = $DEFAULT_LANGUAGE;
        }

        global $AMS_TRANS;
        $variables = parse_ini_file( $AMS_TRANS . '/' .  $lang . '.ini', true );
        $mailText = array();
        foreach ( $variables['email'] as $key => $value ){
            $mailText[$key] = $value;
        }

        //create the reset url
        global $WEBPATH;
        $resetURL = $WEBPATH . "?page=reset_password&user=". $target_username . "&email=" . $email . "&key=" . $hashed_key;
        //set email stuff
        $recipient = $email;
        $subject = $mailText['email_subject_forgot_password'];
        $body = $mailText['email_body_forgot_password_header'] . $resetURL . $mailText['email_body_forgot_password_footer'];
        Mail_Handler::send_mail($recipient, $subject, $body, NULL);
        $result['EMAIL_SUCCESS'] = 'TRUE';
        $result['prevEmail'] = $email;
        $result['no_visible_elements'] = 'TRUE';
        helpers :: loadtemplate( 'forgot_password', $result);
        throw new SystemExit();


}
