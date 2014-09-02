<?php
/**
* handles basic user registration & management functions (shard related).
* The Users class is the basis class of WebUsers, this class provides functions being used by all CMS's and our own www version. The WebUsers class however needs to be reimplemented
* by using the CMS's it's funcionality. This class handles the writing to the shard db mainly, and in case it's offline: writing to the ams_querycache.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Users{

     /**
     * checks if entered values before registering are valid.
     * @param $values array with Username,Password, ConfirmPass and Email.
     * @return string Info: Returns a string, if input data is valid then "success" is returned, else an array with errors
     */
     public function check_Register($values){
          // check values
          if ( isset( $values["Username"] ) and isset( $values["Password"] ) and isset( $values["ConfirmPass"] ) and isset( $values["Email"] ) ){
               $user = Users::checkUser( $values["Username"] );
               $pass = Users::checkPassword( $values["Password"] );
               $cpass = Users::confirmPassword($pass,$values["Password"],$values["ConfirmPass"]);
               $email = Users::checkEmail( $values["Email"] );
          }else{
               $user = "";
               $pass = "";
               $cpass = "";
               $email = "";
          }

          if ( ( $user == "success" ) and ( $pass == "success" ) and ( $cpass == "success" ) and ( $email == "success" ) and (  isset( $_POST["TaC"] ) ) ){
               return "success";
          }else{
               global $TOS_URL;
               $pageElements = array(
               //'GAME_NAME' => $GAME_NAME,
               // 'WELCOME_MESSAGE' => $WELCOME_MESSAGE,
                'USERNAME' => $user,
                'PASSWORD' => $pass,
                'CPASSWORD' => $cpass,
                'EMAIL' => $email,
                'TOS_URL' => $TOS_URL
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
               return $pageElements;
          }

     }


     /**
     * checks if entered username is valid.
     * @param $username the username that the user wants to use.
     * @return string Info: Returns a string based on if the username is valid, if valid then "success" is returned
     */
     public function checkUser( $username )
     {
          if ( isset( $username ) ){
               if ( strlen( $username ) > 12 ){
                    return "Username must be no more than 12 characters.";
               }else if ( strlen( $username ) < 5 ){
                    return "Username must be 5 or more characters.";
               }else if ( !preg_match( '/^[a-z0-9\.]*$/', $username ) ){
                    return "Username can only contain numbers and letters.";
               }else if ( $username == "" ){
                    return "You have to fill in a username";
               }elseif ($this->checkUserNameExists($username)){
                    return "Username " . $username . " is in use.";
               }else{
                    return "success";
               }
          }
          return "fail";
     }

     /**
     * check if username already exists.
     * This is the base function, it should be overwritten by the WebUsers class.
     * @param $username the username
     * @return string Info: Returns true or false if the user is in the www db.
     */
     protected function checkUserNameExists($username){
          //You should overwrite this method with your own version!
          print('this is the base class!');

     }


    /**
     * checks if the password is valid.
     * @param $pass the password willing to be used.
     * @return string Info: Returns a string based on if the password is valid, if valid then "success" is returned
     */
     public function checkPassword( $pass )
    {
         if ( isset( $pass ) ){
             if ( strlen( $pass ) > 20 ){
                 return "Password must be no more than 20 characters.";
                 }elseif ( strlen( $pass ) < 5 ){
                 return "Password must be more than 5 characters.";
                 }elseif ( $pass == ""){
                 return "You have to fill in a password";
                 }else{
                 return "success";
                 }
             }
         return "fail";
         }


    /**
     * checks if the confirmPassword matches the original.
     * @param $pass_result the result of the previous password check.
     * @param $pass the original pass.
     * @param $confirmpass the confirmation password.
     * @return string Info: Verify's $_POST["Password"] is the same as $_POST["ConfirmPass"]
     */
     private function confirmPassword($pass_result,$pass,$confirmpass)
    {
          if ($confirmpass==""){
             return "You have to fill in the confirmation password.";
          }
          else if ( ( $pass ) != ( $confirmpass ) ){
             return "Passwords do not match.";
          }else if($pass_result != "success"){
               return;
          }else{
             return "success";
          }
          return "fail";
     }


    /**
     * wrapper to check if the email address is valid.
     * @param $email the email address
     * @return "success", else in case it isn't valid an error will be returned.
     */
     public function checkEmail( $email )
    {
         if ( isset( $email ) ){
               if ( !Users::validEmail( $email ) ){
                    return "Email address is not valid.";
               }else if($email == ""){
                    return "You have to fill in an email address";
               }else if ($this->checkEmailExists($email)){
                    return "Email is in use.";
               }else{
                    return "success";
               }
          }
          return "fail";
     }


     /**
     * check if email already exists.
     * This is the base function, it should be overwritten by the WebUsers class.
     * @param $email the email address
     * @return string Info: Returns true or false if the email is in the www db.
     */
     protected function checkEmailExists($email){
          //TODO: You should overwrite this method with your own version!
          print('this is the base class!');

     }


     /**
     * check if the emailaddress structure is valid.
     * @param $email the email address
     * @return true or false
     */
     public function validEmail( $email ){
          $isValid = true;
          $atIndex = strrpos( $email, "@" );
          if ( is_bool( $atIndex ) && !$atIndex ){
               $isValid = false;
          }else{
               $domain = substr( $email, $atIndex + 1 );
               $local = substr( $email, 0, $atIndex );
               $localLen = strlen( $local );
               $domainLen = strlen( $domain );
               if ( $localLen < 1 || $localLen > 64 ){
                    // local part length exceeded
                    $isValid = false;
               }else if ( $domainLen < 1 || $domainLen > 255 ){
                    // domain part length exceeded
                    $isValid = false;
               }else if ( $local[0] == '.' || $local[$localLen - 1] == '.' ){
                    // local part starts or ends with '.'
                    $isValid = false;
               }else if ( preg_match( '/\\.\\./', $local ) ){
                    // local part has two consecutive dots
                    $isValid = false;
               }else if ( !preg_match( '/^[A-Za-z0-9\\-\\.]+$/', $domain ) ){
                    // character not valid in domain part
                    $isValid = false;
               }else if ( preg_match( '/\\.\\./', $domain ) ){
                    // domain part has two consecutive dots
                    $isValid = false;
               }else if ( !preg_match( '/^(\\\\.|[A-Za-z0-9!#%&`_=\\/$\'*+?^{}|~.-])+$/', str_replace( "\\\\", "", $local ) ) ){
                    // character not valid in local part unless
                    // local part is quoted
                    if ( !preg_match( '/^"(\\\\"|[^"])+"$/', str_replace( "\\\\", "", $local ) ) ){
                         $isValid = false;
                     }
               }
               if ( $isValid && !( checkdnsrr( $domain, "MX" ) || checkdnsrr( $domain, "A" ) ) ){
                    // domain not found in DNS
                    $isValid = false;
               }
          }
          return $isValid;
     }



     /**
     * generate a SALT.
     * @param $length the length of the SALT which is by default 2
     * @return a random salt of 2 chars
     */
     public static function generateSALT( $length = 2 )
    {
         // start with a blank salt
        $salt = "";
         // define possible characters - any character in this string can be
        // picked for use in the salt, so if you want to put vowels back in
        // or add special characters such as exclamation marks, this is where
        // you should do it
        $possible = "2346789bcdfghjkmnpqrtvwxyzBCDFGHJKLMNPQRTVWXYZ";
         // we refer to the length of $possible a few times, so let's grab it now
        $maxlength = strlen( $possible );
         // check for length overflow and truncate if necessary
        if ( $length > $maxlength ){
             $length = $maxlength;
             }
         // set up a counter for how many characters are in the salt so far
        $i = 0;
         // add random characters to $salt until $length is reached
        while ( $i < $length ){
             // pick a random character from the possible ones
            $char = substr( $possible, mt_rand( 0, $maxlength - 1 ), 1 );
             // have we already used this character in $salt?
            if ( !strstr( $salt, $char ) ){
                 // no, so it's OK to add it onto the end of whatever we've already got...
                $salt .= $char;
                 // ... and increase the counter by one
                $i++;
                 }
             }
         // done!
        return $salt;
     }



     /**
     * creates a user in the shard.
     * incase the shard is offline it will place it in the ams_querycache. You have to create a user first in the CMS/WWW and use the id for this function.
     * @param $values with name,pass and mail
     * @param $user_id the extern id of the user (the id given by the www/CMS)
     * @return ok if it's get correctly added to the shard, else return lib offline and put in libDB, if libDB is also offline return liboffline.
     */
     public static function createUser($values, $user_id){
          try {
               //make connection with and put into shard db
               $dbs = new DBLayer("shard");
               $dbs->insert("user", $values);
               /*
               $dbr = new DBLayer("ring");
               $valuesRing['user_id'] =$user_id;
               $valuesRing['user_name'] = $values['Login'];
               $valuesRing['user_type'] = 'ut_pioneer';
               $dbr->insert("ring_users", $valuesRing);
               */
               ticket_user::createTicketUser( $user_id, 1);
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer("lib");
                    $dbl->insert("ams_querycache", array("type" => "createUser",
                    "query" => json_encode(array($values["Login"],$values["Password"],$values["Email"])), "db" => "shard"));
                    ticket_user::createTicketUser( $user_id , 1 );
                    return "shardoffline";
               }catch (PDOException $e) {
                    print_r($e);
                    return "liboffline";
               }
          }

     }

     /**
     * creates permissions in the shard db for a user.
     * incase the shard is offline it will place it in the ams_querycache.
     * @param $pvalues with username
     */
     public static function createPermissions($pvalues) {

          try {
               $values = array('username' =>  $pvalues[0]);
               $dbs = new DBLayer("shard");
               $sth = $dbs->selectWithParameter("UId", "user", $values, "Login= :username");
               $result = $sth->fetchAll();
               foreach ($result as $UId) {
                   $ins_values = array('UId' => $UId['UId'], 'clientApplication' => 'r2', 'AccessPrivilege' => 'OPEN');
                   $dbs->insert("permission", $ins_values);
                   $ins_values['clientApplication'] = 'ryzom_open';
                   $dbs->insert("permission", $ins_values);
               }
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put it in query queue at ams_lib db!
               $dbl = new DBLayer("lib");
               $dbl->insert("ams_querycache", array("type" => "createPermissions",
               "query" => json_encode(array($pvalues[0])), "db" => "shard"));
          }
          return true;
     }


     /**
     * check if username and password matches.
     * This is the base function, it should be overwritten by the WebUsers class.
     * @param $user the inserted username
     * @param $pass the inserted password
     */
     protected static function checkLoginMatch($user,$pass){
          print('This is the base class!');
     }

     /**
     * check if the changing of a password is valid.
     * a mod/admin doesn't has to fill in the previous password when he wants to change the password, however for changing his own password he has to fill it in.
     * @param $values an array containing the CurrentPass, ConfirmNewPass, NewPass and adminChangesOthers
     * @return if it is valid "success will be returned, else an array with errors will be returned.
     */
     public function check_change_password($values){
          //if admin isn't changing others
          if(!$values['adminChangesOther']){
               if ( isset( $values["user"] ) and isset( $values["CurrentPass"] ) and isset( $values["ConfirmNewPass"] ) and isset( $values["NewPass"] ) ){
                    $match = $this->checkLoginMatch($values["user"],$values["CurrentPass"]);
                    $newpass = $this->checkPassword($values["NewPass"]);
                    $confpass = $this->confirmPassword($newpass,$values["NewPass"],$values["ConfirmNewPass"]);
               }else{
                    $match = "";
                    $newpass = "";
                    $confpass = "";
               }
          }else{
               //if admin is indeed changing someone!
               if ( isset( $values["user"] ) and isset( $values["ConfirmNewPass"] ) and isset( $values["NewPass"] ) ){
                    $newpass = $this->checkPassword($values["NewPass"]);
                    $confpass = $this->confirmPassword($newpass,$values["NewPass"],$values["ConfirmNewPass"]);
               }else{
                    $newpass = "";
                    $confpass = "";
               }
          }
          if (  !$values['adminChangesOther'] and ( $match != "fail" ) and ( $newpass == "success" ) and ( $confpass == "success" ) ){
               return "success";
          }else if($values['adminChangesOther'] and ( $newpass == "success" ) and ( $confpass == "success" ) ){
               return "success";
          }else{
               $pageElements = array(
                'newpass_error_message' => $newpass,
                'confirmnewpass_error_message' => $confpass
                );
               if(!$values['adminChangesOther']){
                    $pageElements['match_error_message'] = $match;
                    if ( $match != "fail" ){
                         $pageElements['MATCH_ERROR'] = 'FALSE';
                    }else{
                         $pageElements['MATCH_ERROR'] = 'TRUE';
                    }
               }
               if ( $newpass != "success" ){
                    $pageElements['NEWPASSWORD_ERROR'] = 'TRUE';
               }else{
                    $pageElements['NEWPASSWORD_ERROR'] = 'FALSE';
               }
               if ( $confpass != "success" ){
                    $pageElements['CNEWPASSWORD_ERROR'] = 'TRUE';
               }else{
                    $pageElements['CNEWPASSWORD_ERROR'] = 'FALSE';
               }
               return $pageElements;
          }
     }

     /**
     * sets the shards password.
     * in case the shard is offline, the entry will be stored in the ams_querycache.
     * @param $user the usersname of the account of which we want to change the password.
     * @param $pass the new password.
     * @return ok if it worked, if the lib or shard is offline it will return liboffline or shardoffline.
     */
     protected static function setAmsPassword($user, $pass){

           $values = Array('Password' => $pass);

           try {
               //make connection with and put into shard db
               $dbs = new DBLayer("shard");
               $dbs->update("user", $values, "Login = $user");
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer("lib");
                    $dbl->insert("ams_querycache", array("type" => "change_pass",
                    "query" => json_encode(array($values["user"],$values["pass"])), "db" => "shard"));
                    return "shardoffline";
               }catch (PDOException $e) {
                    return "liboffline";
               }
          }
     }

     /**
     * sets the shards email.
     * in case the shard is offline, the entry will be stored in the ams_querycache.
     * @param $user the usersname of the account of which we want to change the emailaddress.
     * @param $mail the new email address
     * @return ok if it worked, if the lib or shard is offline it will return liboffline or shardoffline.
     */
     protected static function setAmsEmail($user, $mail){

           $values = Array('Email' => $mail);

           try {
               //make connection with and put into shard db
               $dbs = new DBLayer("shard");
               $dbs->update("user", $values, "Login = $user");
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer("lib");
                    $dbl->insert("ams_querycache", array("type" => "change_mail",
                    "query" => json_encode(array($values["user"],$values["mail"])), "db" => "shard"));
                    return "shardoffline";
               }catch (PDOException $e) {
                    return "liboffline";
               }
          }
     }
}
