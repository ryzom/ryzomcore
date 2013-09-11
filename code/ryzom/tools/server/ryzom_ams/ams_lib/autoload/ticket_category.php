<?php
/**
* Class related to the ticket categories.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_Category{
    
    private $tCategoryId; /**< The id of the category */ 
    private $name; /**< The name of the category */ 
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /**
    * creates a ticket_Catergory in the DB.   
    * @param $name name we want to give to the new category.
    */
    public static function createTicketCategory( $name) {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_category (Name) VALUES (:name)";
        $values = Array('name' => $name);
        $dbl->execute($query, $values);

    }


    /**
    * construct a category object based on the TCategoryId.
    * @return constructed element based on TCategoryId
    */
    public static function constr_TCategoryId( $id) {
        $instance = new self();
        $instance->setTCategoryId($id);
        return $instance;
    }
    

    /**
    * return a list of all category objects.
    * @return an array consisting of all category objects.
    */
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
     
    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }


    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a categories id.
    * @param $id the id of the ticket_category that should be loaded
    */
    public function load_With_TCategoryId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_category WHERE TCategoryId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tCategoryId = $row['TCategoryId'];
        $this->name = $row['Name'];
    }
    
    
    /**
    * update object attributes to the DB.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket_category SET Name = :name WHERE TCategoryId=:id";
        $values = Array('id' => $this->tCategoryId, 'name' => $this->name);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get name attribute of the object.
    */
    public function getName(){
        if ($this->name == ""){
            $this->load_With_TCategoryId($this->tCategoryId);
        }
        return $this->name;
    }
    
    /**
    * get tCategoryId attribute of the object.
    */
    public function getTCategoryId(){
        return $this->tCategoryId;
    }
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    /**
    * set name attribute of the object.
    * @param $n name of the category
    */
    public function setName($n){
        $this->name = $n;
    }
    
    /**
    * set tCategoryId attribute of the object.
    * @param $id integer id of the category
    */
    public function setTCategoryId($id){
        $this->tCategoryId = $id;
    }
   
    
}