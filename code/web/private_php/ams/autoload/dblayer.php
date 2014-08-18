<?php
/**
 * Handles the database connections. It uses PDO to connect to the different databases. It will use the argument of the constructor to setup a connection to the database 
 * with the matching entry in the $cfg global variable.
 * 
 * --> First create an object of dblayer --> $db = new DBLayer('short database name used in config')
 * 
 * --> Insert -->  $db->insert( $tb_name, $data )
 * 				   $tb_name = table name in which we want to insert data
 * 				   $data = array of data that needs to be inserted in format('fieldname' => $value) where fieldname must be a field in that table. 	
 * 
 * --> select -->  $db->select( $tb_name, $data, $where )
 *                 $tb_name = table name which we want to select
 * 				   $data = array of data which is then required in WHERE clause in format array('fieldname'=>$value) fieldname must be a field in that table.
 *				   $where = string in format ('fieldname=:fieldname') where :fieldname takes it's value from $data array. 
 * 
 * --> update -->  $db->update( $tb_name, $data, $where )
 * 				   $tb_name = table name which we want to update
 * 				   $data = array of data which contains the filelds that need to be updated with their values in the format('fieldname' => $value,...) where fieldname must be a field in that table.
 * 				   $where = string contains the filename with a value at that field in the format ('fieldname = $value') where fieldname must be a field in that table and $value is value respect to that field.		
 * 
 * --> delete -->  $db->delete( $tb_name, $data, $where )
 * 				   $tb_name = table name where we want to delete.
 * 				   $data = array of data which is then required in WHERE clause in format array('fieldname'=> $value) where fieldname must be a field in that table.	
 * 				   $where = string in format ('fieldname=:fieldname') where :fieldname takes it's value from $data array.
 * 
 * 
 * @author Daan Janssens, mentored by Matthew Lagoe 
 * 
 */
class DBLayer {
    
    private $PDO;
    /**
     *  The PDO object, instantiated by the constructor
     */
    
    /**
     * The constructor.
     * Instantiates the PDO object attribute by connecting to the arguments matching database(the db info is stored in the $cfg global var)
     * 
     * @param  $db String, the name of the databases entry in the $cfg global var.
     * @param  $dbn String, the name of the databases entry in the $cfg global var if $db referenced to an action(install etc).
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
     * Execute a query that doesn't have any parameters.
     * 
     * @param  $query the mysql query.
     * @return returns a PDOStatement object.
     */
    public function executeWithoutParams( $query ) {
        $statement = $this -> PDO -> prepare( $query );
         $statement -> execute();
         return $statement;
         } 
    
    /**
     * Execute a query that has parameters.
     * 
     * @param  $query the mysql query.
     * @param  $params the parameters that are being used by the query.
     * @return returns a PDOStatement object.
     */
    public function execute( $query, $params ) {
        $statement = $this -> PDO -> prepare( $query );
         $statement -> execute( $params );
         return $statement;
         } 
    
    /**
     * Insert function which returns id of the inserting field. 
     * 
     * @param  $tb_name table name where we want to insert data.
     * @param  $data the parameters that are being inserted into table.
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
     * Select function using prepared statement.
     * For selecting particular fields.
     * 
     * @param string $param field to select, can be multiple fields.
     * @param string $tb_name Table Name to Select.
     * @param array $data array of data to be used in WHERE clause in format('fieldname'=>$value). 'fieldname' must be a field in that table.
     * @param string $where where to select.
     * @return statement object.
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
     * Select function using prepared statement.
     * For selecting all fields in a table.
     * 
     * @param string $tb_name Table Name to Select.
     * @param array $data array of data to be used with WHERE part in format('fieldname'=>$value,...). 'fieldname' must be a field in that table.
     * @param string $where where to select in format('fieldname=:fieldname' AND ...).
     * @return statement object.
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
     * Update function with prepared statement.
     * 
     * @param string $tb_name name of the table on which operation to be performed.
     * @param array $data array of data in format('fieldname' => $value,...).Here, only those fields must be stored which needs to be updated.
     * @param string $where where part in format ('fieldname'= $value AND ...). 'fieldname' must be a field in that table.
     * @throws Exception error in updating.
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
     * insert function using prepared statements.
     * 
     * @param string $tb_name Name of the table on which operation to be performed.
     * @param array $data  array of data to insert in format('fieldname' => $value,....). 'fieldname' must be a field in that table.
     * @throws error in inserting.
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
             throw new Exception( "error in inserting" );
             } 
        } 
    
    /**
     * Delete database entery using prepared statement.
     * 
     * @param string $tb_name table name on which operations to be performed.
     * @param $data array with values in the format('fieldname'=> $value,...). 'fieldname' must be a field in that table.  
     * @param string $where condition based on $data array in the format('fieldname=:fieldname' AND ...).
     * @throws error in deleting.
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
            $this -> PDO -> rollBack();
             throw new Exception( "error in deleting" );
             } 
        
        } 
    }
