<?php
/**
* Handles the database connections. It uses PDO to connect to the different databases. It will use the argument of the constructor to setup a connection to the database 
* with the matching entry in the $cfg global variable.
* @author Daan Janssens, mentored by Matthew Lagoe
* 
*/
class DBLayer{
    
    private $PDO; /**< The PDO object, instantiated by the constructor */ 
    
    /**
    * The constructor.
    * Instantiates the PDO object attribute by connecting to the arguments matching database(the db info is stored in the $cfg global var)
    * @param String, the name of the databases entry in the $cfg global var.
    */
    function __construct($db)
    {
        global $cfg;
        $dsn  = "mysql:";
        $dsn .= "host=".   $cfg['db'][$db]['host'].";";
        $dsn .= "dbname=". $cfg['db'][$db]['name'].";";
        $dsn .= "port=".   $cfg['db'][$db]['port'].";";
    
        $opt = array(
            PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
        );
        $this->PDO = new PDO($dsn,$cfg['db'][$db]['user'],$cfg['db'][$db]['pass'], $opt);

    }
    
    /**
    * execute a query that doesn't have any parameters
    * @param $query the mysql query
    * @return returns a PDOStatement object 
    */
    public function executeWithoutParams($query){
        $statement = $this->PDO->prepare($query);
        $statement->execute();
        return $statement;
    }
    
    /**
    * execute a query that has parameters
    * @param $query the mysql query
    * @param $params the parameters that are being used by the query
    * @return returns a PDOStatement object 
    */
    public function execute($query,$params){
        $statement = $this->PDO->prepare($query);
        $statement->execute($params);
        return $statement;
    }
    
    /**
    * execute a query (an insertion query) that has parameters and return the id of it's insertion
    * @param $query the mysql query
    * @param $params the parameters that are being used by the query
    * @return returns the id of the last inserted element.
    */
    public function executeReturnId($query,$params){
        $statement = $this->PDO->prepare($query);
        $this->PDO->beginTransaction();
        $statement->execute($params);
        $lastId =$this->PDO->lastInsertId();
        $this->PDO->commit();
        return $lastId;
    }
    
}