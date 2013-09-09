<?php

class WebUsers extends Users{

       private $uId;
       private $login;
       private $email;
       private $firstname;
       private $lastname;
       private $gender;
       private $country;
       private $receiveMail;
       private $language;
       
       function __construct($UId = 0) {
              $this->uId = $UId;
       }
       
       public function set($values){
              $this->uId = $values['UId'];
              $this->login = $values['Login'];
              $this->email = $values['Email'];
              $this->firstname = $values['FirstName'];
              $this->lastname = $values['LastName'];
              $this->gender = $values['Gender'];
              $this->country = $values['Country'];
              $this->receiveMail = $values['ReceiveMail'];
              $this->language = $values['Language'];
       }
    
     /**
     * Function checkUserNameExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the web db.
     */
     protected function checkUserNameExists($username){
            $dbw = new DBLayer("web");
            return $dbw->execute("SELECT * FROM ams_user WHERE Login = :name",array('name' => $username))->rowCount();  
     }
    
    
    /**
     * Function checkEmailExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the www db.
     */
     protected function checkEmailExists($email){
            $dbw = new DBLayer("web");
            return $dbw->execute("SELECT * FROM ams_user WHERE Email = :email",array('email' => $email))->rowCount();
     }
     
     
    /**
     * Function checkUserPassMatch
     *
     * @takes $username,$password
     * @return string Info: Returns true or false if a login match is found in the web db
     */
     public function checkLoginMatch($username,$password){

        $dbw = new DBLayer("web");
        $statement = $dbw->execute("SELECT * FROM ams_user WHERE Login=:user", array('user' => $username));
        $row = $statement->fetch();
        $salt = substr($row['Password'],0,2);
        $hashed_input_pass = crypt($password, $salt);
        if($hashed_input_pass == $row['Password']){
              return $row;
        }else{
              return "fail";
        }	
     }
       
       //returns te id for a given username
       public static function getId($username){
         $dbw = new DBLayer("web");  
         $statement = $dbw->execute("SELECT * FROM ams_user WHERE Login=:username", array('username' => $username));
         $row = $statement->fetch();
         return $row['UId'];
       }
    
       //returns te id for a given username
       public static function getIdFromEmail($email){
          $dbw = new DBLayer("web");  
          $statement = $dbw->execute("SELECT * FROM ams_user WHERE Email=:email", array('email' => $email));
          $row = $statement->fetch();
          if(!empty($row)){
              return $row['UId'];
          }else{
              return "FALSE";
          }
       }
    
       public function getUId(){
              return $this->uId;
       }
    
    public function getUsername(){
       $dbw = new DBLayer("web");
       if(! isset($this->login) || $this->login == ""){
             $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
             $row = $statement->fetch();
             $this->set($row);
       }
       return $this->login;
    }
    
    public function getEmail(){
       $dbw = new DBLayer("web");
       if(! isset($this->email) || $this->email == ""){
              $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
              $row = $statement->fetch();
              $this->set($row);
       }
       return $this->email;
    }
    
    public function getInfo(){
       $dbw = new DBLayer("web");
       if(! (isset($this->firstname) && isset($this->lastname) && isset($this->gender) && isset($this->country) && isset($this->receiveMail) ) ||
          $this->firstname == "" || $this->lastname == "" || $this->gender == "" || $this->country == "" || $this->receiveMail == ""){
             $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
             $row = $statement->fetch();
             $this->set($row);
       }
       $result = Array('FirstName' => $this->firstname, 'LastName' => $this->lastname, 'Gender' => $this->gender, 'Country' => $this->country, 'ReceiveMail' => $this->receiveMail);
       return $result;
    }
    
       public function getReceiveMail(){
       $dbw = new DBLayer("web");
       if(! isset($this->receiveMail) || $this->receiveMail == ""){
              $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
              $row = $statement->fetch();
              $this->set($row);
       }
       return $this->receiveMail;
    }
    
       public function getLanguage(){
       $dbw = new DBLayer("web");
       if(! isset($this->language) || $this->language == ""){
              $statement = $dbw->execute("SELECT * FROM ams_user WHERE UId=:id", array('id' => $this->uId));
              $row = $statement->fetch();
              $this->set($row);
       }
       return $this->language;
    }
    
    public function isLoggedIn(){
        if(isset($_SESSION['user'])){
            return true;
        }
        return false;
    }
    
    public function setPassword($user, $pass){
        $reply = WebUsers::setAmsPassword($user, $pass);
        $values = Array('user' => $user, 'pass' => $pass);
         try {
               //make connection with and put into shard db
               $dbw = new DBLayer("web");
               $dbw->execute("UPDATE ams_user SET Password = :pass WHERE Login = :user ",$values);
          }
          catch (PDOException $e) {
            //ERROR: the web DB is offline
          }
        return $reply;
    }
    
     public function setEmail($user, $mail){
        $reply = WebUsers::setAmsEmail($user, $mail);
        $values = Array('user' => $user, 'mail' => $mail);
         try {
               //make connection with and put into shard db
               $dbw = new DBLayer("web");
               $dbw->execute("UPDATE ams_user SET Email = :mail WHERE Login = :user ",$values);
          }
          catch (PDOException $e) {
            //ERROR: the web DB is offline
          }
        return $reply;
    }
    
       public static function setReceiveMail($user, $receivemail){
        $values = Array('user' => $user, 'receivemail' => $receivemail);
         try {
               //make connection with and put into shard db
               $dbw = new DBLayer("web");
               $dbw->execute("UPDATE ams_user SET ReceiveMail = :receivemail WHERE UId = :user ",$values);
          }
          catch (PDOException $e) {
            //ERROR: the web DB is offline
          }
    }
    
      public static function setLanguage($user, $language){
        $values = Array('user' => $user, 'language' => $language);
         try {
               //make connection with and put into shard db
               $dbw = new DBLayer("web");
               $dbw->execute("UPDATE ams_user SET Language = :language WHERE UId = :user ",$values);
          }
          catch (PDOException $e) {
            //ERROR: the web DB is offline
          }
    }
    
    public function getUsers(){
        $dbl = new DBLayer("web");
        $data = $dbl->executeWithoutParams("SELECT * FROM ams_user");
        return $data;
    }
    
    public static function getAllUsersQuery(){
       return "SELECT * FROM ams_user";
    }
    
    public static function createWebuser($name, $pass, $mail){
       
       //register account with the correct language (check if cookie is already set)!
       if ( isset( $_COOKIE['Language'] ) ) { 
              $lang = $_COOKIE['Language'];
       }else{
              global $DEFAULT_LANGUAGE;
              $lang = $DEFAULT_LANGUAGE;
       }
       
       $values = Array('name' => $name, 'pass' => $pass, 'mail' => $mail, 'lang' => $lang);
       
       try {
          $dbw = new DBLayer("web");
          return $dbw->executeReturnId("INSERT INTO ams_user (Login, Password, Email, Language) VALUES (:name, :pass, :mail, :lang)",$values);
       }
       catch (PDOException $e) {
            //ERROR: the web DB is offline
       }
    }
    
}