<?php
/**
* handles functions related to replies on tickets.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
class Ticket_Reply{
    private $tReplyId; /**< The id of the reply */
    private $ticket; /**< the ticket id related to the reply */
    private $content; /**< the content of the reply */
    private $author; /**< The id of the user that made the reply */
    private $timestamp; /**< The timestamp of the reply */
    private $hidden; /**< indicates if reply should be hidden for normal users or not */

    ////////////////////////////////////////////Functions////////////////////////////////////////////////////

    /**
    * return constructed element based on TReplyId.
    * @param $id the Id the reply we want to load.
    * @return the loaded object.
    */
    public static function constr_TReplyId( $id) {
        $instance = new self();
        $instance->setTReplyId($id);
        return $instance;
    }


    /**
    * return all replies on a specific ticket.
    * @param $ticket_id the id of the ticket of which we want the replies.
    * @param $view_as_admin if the browsing user is an admin/mod it should be 1, this will also show the hidden replies.
    * @return an array with ticket_reply objects (beware the author and content are objects on their own, not integers!)
    */
    public static function getRepliesOfTicket( $ticket_id, $view_as_admin) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_reply INNER JOIN ticket_content INNER JOIN ticket_user ON ticket_reply.Content = ticket_content.TContentId and ticket_reply.Ticket=:id and ticket_user.TUserId = ticket_reply.Author ORDER BY ticket_reply.TReplyId ASC", array('id' => $ticket_id));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $tReply){
            //only add hidden replies if the user is a mod/admin
            if(! $tReply['Hidden'] || $view_as_admin){
                //load author
                $instanceAuthor = Ticket_User::constr_TUserId($tReply['Author']);
                $instanceAuthor->setExternId($tReply['ExternId']);
                $instanceAuthor->setPermission($tReply['Permission']);

                //load content
                $instanceContent = new Ticket_Content();
                $instanceContent->setTContentId($tReply['TContentId']);
                $instanceContent->setContent($tReply['Content']);

                //load reply and add the author and content object in it.
                $instanceReply = new self();
                $instanceReply->setTReplyId($tReply['TReplyId']);
                $instanceReply->setTimestamp($tReply['Timestamp']);
                $instanceReply->setAuthor($instanceAuthor);
                $instanceReply->setTicket($ticket_id);
                $instanceReply->setContent($instanceContent);
                $instanceReply->setHidden($tReply['Hidden']);
                $result[] = $instanceReply;
            }
        }
        return $result;
    }

    /**
    * creates a new reply on a ticket.
    * Creates a ticket_content entry and links it with a new created ticket_reply, a log entry will be written about this.
    * In case the ticket creator replies on a ticket, he will set the status by default to 'waiting on support'.
    * @param $content the content of the reply
    * @param $author the id of the reply creator.
    * @param $ticket_id the id of the ticket of which we want the replies.
    * @param $hidden should be 0 or 1
    * @param $ticket_creator the ticket's starter his id.
    */
    public static function createReply($content, $author, $ticket_id , $hidden, $ticket_creator){
        $ticket_content = new Ticket_Content();
        $ticket_content->setContent($content);
        $ticket_content->create();
        $content_id = $ticket_content->getTContentId();

        $ticket_reply = new Ticket_Reply();
        $ticket_reply->set(Array('Ticket' => $ticket_id,'Content' => $content_id,'Author' => $author, 'Hidden' => $hidden));
        $ticket_reply->create();
        $reply_id = $ticket_reply->getTReplyId();

        if($ticket_creator == $author){
            Ticket::updateTicketStatus( $ticket_id, 1, $author);
        }

        Ticket_Log::createLogEntry( $ticket_id, $author, 4, $reply_id);
    }

    ////////////////////////////////////////////Methods////////////////////////////////////////////////////

    /**
    * A constructor.
    * Empty constructor
    */
    public function __construct() {
    }


    /**
    * sets the object's attributes.
    * @param $values should be an array.
    */
    public function set($values){
        $this->setTicket($values['Ticket']);
        $this->setContent($values['Content']);
        $this->setAuthor($values['Author']);
        if(isset($values['Timestamp'])){
            $this->setTimestamp($values['Timestamp']);
        }
        if(isset($values['Hidden'])){
            $this->setHidden($values['Hidden']);
        }
    }

    /**
    * creates a new 'ticket_reply' entry.
    * this method will use the object's attributes for creating a new 'ticket_reply' entry in the database (the now() function will create the timestamp).
    */
    public function create(){
        $dbl = new DBLayer("lib");
        $this->tReplyId = $dbl->executeReturnId("ticket_reply", Array('Ticket' => $this->ticket, 'Content' => $this->content, 'Author' => $this->author, 'Hidden' => $this->hidden), array('Timestamp'=>'now()'));
    }

    /**
    * loads the object's attributes.
    * loads the object's attributes by giving a ticket_reply's id.
    * @param $id the id of the ticket_reply that should be loaded
    */
    public function load_With_TReplyId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->select("ticket_reply", array('id' => $id), "TReplyId=:id");
        $row = $statement->fetch();
        $this->tReplyId = $row['TReplyId'];
        $this->ticket = $row['Ticket'];
        $this->content = $row['Content'];
        $this->author = $row['Author'];
        $this->timestamp = $row['Timestamp'];
        $this->hidden = $row['Hidden'];
    }

    /**
    * updates a ticket_reply entry based on the objects attributes.
    */
    public function update(){
        $dbl = new DBLayer("lib");
        $dbl->update("ticket", Array('Ticket' => $this->ticket, 'Content' => $this->content, 'Author' => $this->author, 'Timestamp' => $this->timestamp, 'Hidden' => $this->hidden), "TReplyId=$this->tReplyId, ");
    }

    ////////////////////////////////////////////Getters////////////////////////////////////////////////////

    /**
    * get ticket attribute of the object.
    */
    public function getTicket(){
        return $this->ticket;
    }

    /**
    * get content attribute of the object.
    */
    public function getContent(){
        return $this->content;
    }

    /**
    * get author attribute of the object.
    */
    public function getAuthor(){
        return $this->author;
    }

    /**
    * get timestamp attribute of the object.
    * The output format is defined by the Helpers class function, outputTime().
    */
    public function getTimestamp(){
        return Helpers::outputTime($this->timestamp);
    }

    /**
    * get tReplyId attribute of the object.
    */
    public function getTReplyId(){
        return $this->tReplyId;
    }

    /**
    * get hidden attribute of the object.
    */
    public function getHidden(){
        return $this->hidden;
    }

    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    /**
    * set ticket attribute of the object.
    * @param $t integer id of the ticket
    */
    public function setTicket($t){
        $this->ticket = $t;
    }

    /**
    * set content attribute of the object.
    * @param $c integer id of the ticket_content entry
    */
    public function setContent($c){
        $this->content = $c;
    }

    /**
    * set author attribute of the object.
    * @param $a integer id of the user
    */
    public function setAuthor($a){
        $this->author =  $a;
    }

    /**
    * set timestamp attribute of the object.
    * @param $t timestamp of the reply
    */
    public function setTimestamp($t){
        $this->timestamp = $t;
    }

    /**
    * set tReplyId attribute of the object.
    * @param $i integer id of the ticket_reply
    */
    public function setTReplyId($i){
        $this->tReplyId = $i;
    }

    /**
    * set hidden attribute of the object.
    * @param $h should be 0 or 1
    */
    public function setHidden($h){
        $this->hidden = $h;
    }
}
