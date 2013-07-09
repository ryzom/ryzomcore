<?php
class Ticket_User{
    
    private $tUserId;
    private $permission;
    private $externId;
    private $db;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //Creates a ticket_user in the DB
    public static function createTicketUser( $extern_id, $permission,$db ) {
        $dbl = new DBLayer($db);
        $query = "INSERT INTO ticket_user (Permission, ExternId) VALUES (:perm, :ext_id)";
        $values = Array('perm' => $permission, 'ext_id' => $extern_id);
        $dbl->execute($query, $values);

    }


    //return constructed element based on TUserId
    public static function constr_TUserId( $id, $db_data) {
        $instance = new self($db_data);
        $instance->setTUserId($id);
        return $instance;
    
    }
    
    //return constructed element based on ExternId
    public static function constr_ExternId( $id, $db_data ) {
        $instance = new self($db_data);
        $dbl = new DBLayer($instance->db);
        $statement = $dbl->execute("SELECT * FROM ticket_user WHERE ExternId=:id", array('id' => $id));
        $row = $statement->fetch();
        $instance->tUserId = $row['TUserId'];
        $instance->permission = $row['Permission'];
        $instance->externId = $row['ExternId'];
        return $instance;

    }
    
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    public function __construct($db_data) {
        $this->db = $db_data;
    }
    
    //return constructed element based on TUserId
    public function load_With_TUserId( $id) {
        $dbl = new DBLayer($this->db);
        $statement = $dbl->execute("SELECT * FROM ticket_user WHERE TUserId=:id", array('id' => $id));
        $row = $statement->fetch();
        $instance->tUserId = $row['TUserId'];
        $instance->permission = $row['Permission'];
        $instance->externId = $row['ExternId'];
        return $instance;
    } 
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer($this->db);
        $query = "UPDATE ticket_user SET Permission = :perm, ExternId = :ext_id WHERE TUserId=:id";
        $values = Array('id' => $this->tUserId, 'perm' => $this->permission, 'ext_id' => $this->externId);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getPermission(){
        if ($this->permission == ""){
            $this->load_With_TUserId($this->tUserId);
        }
        return $this->permission;
    }
   
   
    public function getExternId(){
        if ($this->ExternId == ""){
            $this->load_With_TUserId($this->tUserId);
        }
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
    
}