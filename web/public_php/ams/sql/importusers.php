<?php

    /**
    * This script will import all users of the nel db and add a matching ticket_user an ams_user entry for them.
    * @author Daan Janssens, mentored by Matthew Lagoe
    */
    
    //require the pages that are being needed.
    require( '../../config.php' );
    require( '../../../ams_lib/libinclude.php' );
    ini_set( "display_errors", true );
    error_reporting( E_ALL );
    
    //var used to access the DB;
    global $cfg;
    
    
    try{
        //SETUP THE WWW DB
        $dbs = new DBLayer("shard");
        $sql = "SELECT * FROM user";
        $statement = $dbs->executeWithoutParams($sql);
        $users = $statement->fetchAll();
        foreach($users as $user){
            //add user to web
            $dbw = new DBLayer("web");
            if (! $dbw->execute("SELECT * FROM ams_user WHERE Login = :name",array('name' => $user['Login']))->rowCount()){
                $query = "INSERT INTO ams_user (Login, Password, Email, Language) VALUES (:name, :pass, :mail, :lang)";
                global $DEFAULT_LANGUAGE;
                $vars = array('name' => $user['Login'], 'pass' => $user['Password'], 'mail' => $user['Email'], 'lang' => $DEFAULT_LANGUAGE);
                $id = $dbw->executeReturnId($query,$vars);
                $dbl = new DBLayer("lib");
                $query = "INSERT INTO `ticket_user` (Permission, ExternId) VALUES (1, :id)";
                $vars = array('id' => $id);
                $dbl->execute($query,$vars);
            }
        }
        print "The users were imported! ";
    }catch (PDOException $e){
        print "There was an error while creating the admin account! ";
    }
        
        
    