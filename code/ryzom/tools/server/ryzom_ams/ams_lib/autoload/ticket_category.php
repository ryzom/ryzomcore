<?php

class Ticket_Category{
    
    private $tCategoryId;
    private $name;
    
    public function __construct($db_data) {
        $this->db = $db_data;
    }


    //Creates a ticket_Catergory in the DB
    public static function createTicketCategory( $name ,$db ) {
        $dbl = new DBLayer($db);
        $query = "INSERT INTO ticket_category (Name) VALUES (:name)";
        $values = Array('name' => $name);
        $dbl->execute($query, $values);

    }


    //return constructed element based on TCategoryId
    public static function constr_TCategoryId( $id, $db_data) {
        $instance = new self($db_data);
        $instance->setTCategoryId($id);
        return $instance;
    
    }
    
    //return constructed element based on TCategoryId
    public function load_With_TCategoryId( $id) {
        $dbl = new DBLayer($this->db);
        $statement = $dbl->execute("SELECT * FROM ticket_category WHERE TCategoryId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tCategoryId = $row['TCategoryId'];
        $this->name = $row['Name'];
    }
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer($this->db);
        $query = "UPDATE ticket_category SET Name = :name WHERE TCategoryId=:id";
        $values = Array('id' => $this->tCategoryId, 'name' => $this->name);
        $statement = $dbl->execute($query, $values);
    }
    
    //Getters
    public function getName(){
        if ($this->name == ""){
            $this->load_With_TCategoryId($this->tCategoryId);
        }
        return $this->name;
    }
    
    
    public function getTCategoryId(){
        return $this->tCategoryId;
    }
    
    
    //setters
    public function setName($n){
        $this->name = $n;
    }
   
    
}