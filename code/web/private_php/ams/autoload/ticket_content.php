<?php

/**
* Class that handles the content of a reply.
* The Ticket_Content has a one-to-one relation with a ticket_reply, it contains the content of a reply, this way the content doesn't always have to be loaded when
* we query the database when we only need information regarding to the replies basic information.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_Content{
    
    private $tContentId; /**< The id of ticket_content entry */
    private $content; /**< The content of an entry */
    
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    
    /**
    * return constructed element based on TContentId.
    * @param $id the id of ticket_content entry.
    * @return a constructed ticket_content object by specifying the TContentId. 
    */
    public static function constr_TContentId( $id) {
        $instance = new self();
        $instance->setTContentId($id);
        return $instance;
    } 

    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    
    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }

    
    /**
    * creates a new 'tickt_content' entry.
    * this method will use the object's attributes for creating a new 'ticket_content' entry in the database.
    */
    public function create() {
        $dbl = new DBLayer("lib");
    	$this->tContentId = $dbl->executeReturnId("ticket_content", Array('Content' => $this->content));
    }
    
    
    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a ticket_content's id,
    * @param $id the id of the ticket_content entry that should be loaded
    */
    public function load_With_TContentId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("ticket_content", array('id' => $id), "TContentId=:id");
        $row = $statement->fetch();
        $this->tContentId = $row['TContentId'];
        $this->content = $row['Content'];
    }
    
    /**
    * update the object's attributes to the database.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $dbl->update("ticket_content", Array('Content' => $this->content), "TContentId = $this->tContentId");
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    /**
    * get content attribute of the object.
    */
    public function getContent(){
        if ($this->content == ""){
            $this->load_With_TContentId($this->tContentId);
        }
        return $this->content;
    }
    
    /**
    * get tContentId attribute of the object.
    */
    public function getTContentId(){
        return $this->tContentId;
    }
    
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    /**
    * set content attribute of the object.
    * @param $c content of a reply
    */
    public function setContent($c){
        $this->content = $c;
    }
   
    /**
    * set tContentId attribute of the object.
    * @param $c integer id of ticket_content entry
    */
    public function setTContentId($c){
        $this->tContentId = $c;
    }
    
}
