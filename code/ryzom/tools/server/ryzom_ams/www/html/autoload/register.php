<?php
require( '../config.php' );
// check if values exist
if ( isset( $_POST["Username"] ) and isset( $_POST["Password"] ) and isset( $_POST["Email"] ) )
    {
     // check values
    $user = users :: checkUser( $_POST["Username"] );
     $pass = users :: checkPassword( $_POST["Password"] );
     $cpass = users :: confirmPassword();
     $email = users :: checkEmail( $_POST["Email"] );
     }else{
     $user = "";
     $pass = "";
     $cpass = "";
     $email = "";
     }
 // if all are good then create user
if ( ( $user == "success" ) and ( $pass == "success" ) and ( $cpass == "success" ) and ( $email == "success" ) and ( isset( $_POST["TaC"] ) ) ){
     $edit = array( 
        'name' => $_POST["Username"],
         'pass' => $_POST["Password"],
         'mail' => $_POST["Email"],
         'init' => $_POST["Email"],
         'unhashpass' => $_POST["Password"],
         'status' => 1,
         'access' => REQUEST_TIME
         );
     user_save( NULL, $edit );
     header( 'Location: email_sent.php' );
     exit;
     }else{
     $pageElements = array( 
        'GAME_NAME' => $GAME_NAME,
         'WELCOME_MESSAGE' => $WELCOME_MESSAGE,
         'USERNAME' => $user,
         'PASSWORD' => $pass,
         'CPASSWORD' => $cpass,
         'EMAIL' => $email
         );
     if ( $user != "success" ){
         $pageElements['USERNAME_ERROR'] = 'TRUE';
         }else{
         $pageElements['USERNAME_ERROR'] = 'FALSE';
         }
    
     if ( $pass != "success" ){
         $pageElements['PASSWORD_ERROR'] = 'TRUE';
         }else{
         $pageElements['PASSWORD_ERROR'] = 'FALSE';
         }
     if ( $cpass != "success" ){
         $pageElements['CPASSWORD_ERROR'] = 'TRUE';
         }else{
         $pageElements['CPASSWORD_ERROR'] = 'FALSE';
         }
     if ( $email != "success" ){
         $pageElements['EMAIL_ERROR'] = 'TRUE';
         }else{
         $pageElements['EMAIL_ERROR'] = 'FALSE';
         }
     if ( isset( $_POST["TaC"] ) ){
         $pageElements['TAC_ERROR'] = 'FALSE';
         }else{
         $pageElements['TAC_ERROR'] = 'TRUE';
         }
     if ( helpers :: check_if_game_client() ){
        helpers :: loadtemplate( '../../ams_lib/ingame_templates/register.phtml', $pageElements );
         }else{
         helpers :: loadtemplate( 'templates/register.phtml', $pageElements );
         }
     }
