<?php
namespace Ams_Tickets;

class Ticket{
    private $tId;
    private $timestamp;
    private $title;
    private $status;
    private $queue;
    private $ticket_category;
    private $author;
    private $db;
    
    public function __construct($db_data) {
        $this->db = $db_data;
    }


    //Set ticket object
    public function setTicket($ts,$t,$s,$q,$t_c,$a){
        $this->timestamp = $ts;
        $this->title = $t;
        $this->status = $s;
        $this->queue = $q;
        $this->ticket_category = $t_c;
        $this->author = $a;
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer($this->db);
        $query = "INSERT INTO ticket (Timestamp, Title, Status, Queue, Ticket_Category, Author) VALUES (:timestamp, :title, :status, :queue, :tcat, :author)";
        $values = Array('timestamp' => $this->timestamp, 'title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author);
        $dbl->execute($query, $values);
    }

    //return constructed element based on TId
    public function load_With_TId( $id) {
        $dbl = new DBLayer($this->db);
        $statement = $dbl->execute("SELECT * FROM ticket WHERE TId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tId = $row['TId'];
        $this->timestamp = $row['Timestamp'];
        $this->title = $row['Title'];
        $this->status = $row['Status'];
        $this->queue = $row['Queue'];
        $this->ticket_category = $row['Ticket_Category'];
        $this->author = $row['Author'];
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer($this->db);
        $query = "UPDATE ticket SET Timestamp = :timestamp, Title = :title, Status = :status, Queue = :queue, Ticket_Category = :tcat, Author = :author WHERE TId=:id";
        $values = Array('id' => $this->tId, 'timestamp' => $this->timestamp, 'title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author);
        $statement = $dbl->execute($query, $values);
    }
    
    //Getters
    public function getPermission(){
        return $this->permission;
    }
   
   
    public function getExternId(){
        return $this->externId;
    }
    
    
    public function getTUserId(){
        return $this->tUserId;
    }
    
    //setters
    public function setPermission($perm){
        $this->permission = $perm;
    }
   
   
    public function setExternId($id){
        $this->externId = $id;
    }
    
    
    public function setTUserId($id){
        $this->tUserId = $id;
    }
}