<?php
/**
 * Handles the database connections. It uses PDO to connect to the different databases. It will use the argument of the constructor to setup a connection to the database 
 * with the matching entry in the $cfg global variable.
 * 
 * @author Daan Janssens, mentored by Matthew Lagoe 
 */
class DBLayer {
    
    private $PDO;
    /**
     * *< The PDO object, instantiated by the constructor
     */
    
    /**
     * The constructor.
     * Instantiates the PDO object attribute by connecting to the arguments matching database(the db info is stored in the $cfg global var)
     * 
     * @param  $db String, the name of the databases entry in the $cfg global var.
     */
     function __construct( $db, $dbn = null )
     {
        if ( $db != "install" ) {
            
            global $cfg;
             $dsn = "mysql:";
             $dsn .= "host=" . $cfg['db'][$db]['host'] . ";";
             $dsn .= "dbname=" . $cfg['db'][$db]['name'] . ";";
             $dsn .= "port=" . $cfg['db'][$db]['port'] . ";";
            
             $opt = array( 
                PDO :: ATTR_ERRMODE => PDO :: ERRMODE_EXCEPTION,
                 PDO :: ATTR_DEFAULT_FETCH_MODE => PDO :: FETCH_ASSOC
                 );
             $this -> PDO = new PDO( $dsn, $cfg['db'][$db]['user'], $cfg['db'][$db]['pass'], $opt );
             } else {
            global $cfg;
             $dsn = "mysql:";
             $dsn .= "host=" . $cfg['db'][$dbn]['host'] . ";";
             $dsn .= "port=" . $cfg['db'][$dbn]['port'] . ";";
            
             $opt = array( 
                PDO :: ATTR_ERRMODE => PDO :: ERRMODE_EXCEPTION,
                 PDO :: ATTR_DEFAULT_FETCH_MODE => PDO :: FETCH_ASSOC
                 );
             $this -> PDO = new PDO( $dsn, $_POST['Username'], $_POST['Password'], $opt );
             } 
        
        } 
    
    /**
     * execute a query that doesn't have any parameters
     * 
     * @param  $query the mysql query
     * @return returns a PDOStatement object
     */
    public function executeWithoutParams( $query ) {
        $statement = $this -> PDO -> prepare( $query );
         $statement -> execute();
         return $statement;
         } 
    
    /**
     * execute a query that has parameters
     * 
     * @param  $query the mysql query
     * @param  $params the parameters that are being used by the query
     * @return returns a PDOStatement object
     */
    public function execute( $query, $params ) {
        $statement = $this -> PDO -> prepare( $query );
         $statement -> execute( $params );
         return $statement;
         } 
    
    /**
     * execute a query (an insertion query) that has parameters and return the id of it's insertion
     * 
     * @param  $query the mysql query
     * @param  $params the parameters that are being used by the query
     * @return returns the id of the last inserted element.
     */
    public function executeReturnId( $tb_name, $data ) {
        $field_values = ':' . implode( ',:', array_keys( $data ) );
         $field_options = implode( ',', array_keys( $data ) );
         try {
            $sth = $this -> PDO -> prepare( "INSERT INTO $tb_name ($field_options) VALUE ($field_values)" );
             foreach ( $data as $key => $value )
             {
                $sth -> bindValue( ":$key", $value );
                 } 
            $this -> PDO -> beginTransaction();
            $sth -> execute();
             $lastId = $this -> PDO -> lastInsertId();
             $this -> PDO -> commit();        
             } 
        catch ( Exception $e )
         {
            // for rolling back the changes during transaction
            $this -> PDO -> rollBack();
             throw new Exception( "error in inseting" );
             } 
        return $lastId;
         } 
    
    /**
     * Select function using prepared statement
     * 
     * @param string $tb_name Table Name to Select
     * @param array $data Associative array
     * @param string $where where to select
     * @return statement object
     */
    public function selectWithParameter( $param, $tb_name, $data, $where )
     {
        try {
            $sth = $this -> PDO -> prepare( "SELECT $param FROM $tb_name WHERE $where" );
             $this -> PDO -> beginTransaction();
             $sth -> execute( $data );
             $this -> PDO -> commit();
             } 
        catch( Exception $e )
         {
            $this -> PDO -> rollBack();
             throw new Exception( "error selection" );
             return false;
             }         
        return $sth;
         } 
    
    /**
     * Select function using prepared statement
     * 
     * @param string $tb_name Table Name to Select
     * @param array $data Associative array
     * @param string $where where to select
     * @return statement object
     */
    public function select( $tb_name, $data , $where )
     {
        try {
            $sth = $this -> PDO -> prepare( "SELECT * FROM $tb_name WHERE $where" );
             $this -> PDO -> beginTransaction();
             $sth -> execute( $data );
             $this -> PDO -> commit();
             } 
        catch( Exception $e )
         {
            $this -> PDO -> rollBack();
             throw new Exception( "error selection" );
             return false;
             } 
        return $sth;
         } 
    
    /**
     * Update function with prepared statement
     * 
     * @param string $tb_name name of the table
     * @param array $data associative array with values
     * @param string $where where part
     * @throws Exception error in updating
     */
    public function update( $tb_name, $data, $where )
     {
        $field_option_values = null;
         foreach ( $data as $key => $value )
         {
            $field_option_values .= ",$key" . '=:' . $key;
             } 
        $field_option_values = ltrim( $field_option_values, ',' );
         try {
            $sth = $this -> PDO -> prepare( "UPDATE $tb_name SET $field_option_values WHERE $where " );
            
             foreach ( $data as $key => $value )
             {
                $sth -> bindValue( ":$key", $value );
                 } 
            $this -> PDO -> beginTransaction();
             $sth -> execute();
             $this -> PDO -> commit();
             } 
        catch ( Exception $e )
         {
            $this -> PDO -> rollBack();
             throw new Exception( 'error in updating' );
             return false;
             } 
        return true;
         }
 
    /**
     * insert function using prepared statements
     * 
     * @param string $tb_name Name of the table to insert in
     * @param array $data Associative array of data to insert
     */
    public function insert( $tb_name, $data )
     {
        $field_values = ':' . implode( ',:', array_keys( $data ) );
         $field_options = implode( ',', array_keys( $data ) );
         try {
            $sth = $this -> PDO -> prepare( "INSERT INTO $tb_name ($field_options) VALUE ($field_values)" );
             foreach ( $data as $key => $value )
             {
                
                $sth -> bindValue( ":$key", $value );
                 } 
            $this -> PDO -> beginTransaction();
             // execution
            $sth -> execute();
             $this -> PDO -> commit();
            
             } 
        catch ( Exception $e )
         {
            // for rolling back the changes during transaction
            $this -> PDO -> rollBack();
             throw new Exception( "error in inseting" );
             } 
        } 
    
    /**
     * Delete database entery using prepared statement
     * 
     * @param string $tb_name 
     * @param string $where 
     * @throws error in deleting
     */
    public function delete( $tb_name, $data, $where )
     {
        try {
            $sth = $this -> PDO -> prepare( "DELETE FROM $tb_name WHERE $where" );
             $this -> PDO -> beginTransaction();
             $sth -> execute( $data );
             $this -> PDO -> commit();
             } 
        catch ( Exception $e )
         {
            $this -> rollBack();
             throw new Exception( "error in deleting" );
             } 
        
        } 
    }
