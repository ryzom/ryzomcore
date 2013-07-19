<?php
class Ticket_User{
    
    private $tUserId;
    private $permission;
    private $externId;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //Creates a ticket_user in the DB
    public static function createTicketUser( $extern_id, $permission) {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_user (Permission, ExternId) VALUES (:perm, :ext_id)";
        $values = Array('perm' => $permission, 'ext_id' => $extern_id);
        $dbl->execute($query, $values);

    }
    
    public static function isMod($user){
        if(isset($user) && $user->getPermission() > 1){
            return true;
        }
        return false;
    }
    
    public static function isAdmin($user){
        if(isset($user) && $user->getPermission() == 3){
            return true;
        }
        return false;
    }
    
    //return constructed element based on TUserId
    public static function constr_TUserId( $id) {
        $instance = new self();
        $instance->setTUserId($id);
        return $instance;
    
    }
    
    //return constructed element based on ExternId
    public static function constr_ExternId( $id) {
        $instance = new self();
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_user WHERE ExternId=:id", array('id' => $id));
        $row = $statement->fetch();
        $instance->tUserId = $row['TUserId'];
        $instance->permission = $row['Permission'];
        $instance->externId = $row['ExternId'];
        return $instance;

    }
    
    public static function change_permission($user_id, $perm){
        $user = new Ticket_User();
        $user->load_With_TUserId($user_id);
        $user->setPermission($perm);
        $user->update();
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    public function __construct() {
    }
    
    //return constructed element based on TUserId
    public function load_With_TUserId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_user WHERE TUserId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tUserId = $row['TUserId'];
        $this->permission = $row['Permission'];
        $this->externId = $row['ExternId'];
    } 
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_user SET Permission = :perm, ExternId = :ext_id WHERE TUserId=:id";
        $values = Array('id' => $this->tUserId, 'perm' => $this->permission, 'ext_id' => $this->externId);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getPermission(){
        return $this->permission;
    }
   
   
    public function getExternId(){
        return $this->externId;
    }
    
    
    public function getTUserId(){
        return $this->tUserId;
    }
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    public function setPermission($perm){
        $this->permission = $perm;
    }
   
   
    public function setExternId($id){
        $this->externId = $id;
    }
    
    public function setTUserId($id){
        $this->tUserId= $id;
    }
    
    
}