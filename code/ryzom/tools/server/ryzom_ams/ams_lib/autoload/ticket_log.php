<?php

class Ticket_Log{
    
    private $tCategoryId;
    private $name;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    
    //Creates a ticket_Catergory in the DB
    public static function createTicketCategory( $name) {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_category (Name) VALUES (:name)";
        $values = Array('name' => $name);
        $dbl->execute($query, $values);

    }


    //return constructed element based on TCategoryId
    public static function constr_TCategoryId( $id) {
        $instance = new self();
        $instance->setTCategoryId($id);
        return $instance;
    }
    
    //returns list of all category objects
    public static function getAllCategories() {
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT * FROM ticket_category");
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $category){
            $instance = new self();
            $instance->tCategoryId = $category['TCategoryId'];
            $instance->name = $category['Name'];
            $result[] = $instance;
        }
        return $result; 
    }
     
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }

    //return constructed element based on TCategoryId
    public function load_With_TCategoryId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_category WHERE TCategoryId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tCategoryId = $row['TCategoryId'];
        $this->name = $row['Name'];
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_category SET Name = :name WHERE TCategoryId=:id";
        $values = Array('id' => $this->tCategoryId, 'name' => $this->name);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getName(){
        if ($this->name == ""){
            $this->load_With_TCategoryId($this->tCategoryId);
        }
        return $this->name;
    }
    
    
    public function getTCategoryId(){
        return $this->tCategoryId;
    }
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    public function setName($n){
        $this->name = $n;
    }
    
    public function setTCategoryId($id){
        $this->tCategoryId = $id;
    }
   
    
}