<?php
class Ticket_Queue_csr extends Ticket_Queue{
    
    public function loadAllNewCsrTickets(){
        $dbl = new DBLayer("lib");
        $statement = $dbl->executeWithoutParams("SELECT ticket . * FROM ticket LEFT JOIN assigned ON ticket.TId = assigned.Ticket WHERE assigned.Ticket IS NULL");
        $rows = $statement->fetchAll();
        $this->setQueue($rows);
    }
    
}