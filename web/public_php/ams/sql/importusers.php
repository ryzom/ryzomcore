<?php

    /**
    * This script will import all users of the nel db and add a matching ticket_user an ams_user entry for them.
    * @author Daan Janssens, mentored by Matthew Lagoe
    */
    
    //require the pages that are being needed.
    require( '../../config.php' );
    require_once( $AMS_LIB . '/libinclude.php' );

    // Log errors server-side; printing them here hands the caller the pdo
    // message, which carries the connection details.
    error_reporting( E_ALL );
    ini_set( 'display_errors', '0' );
    ini_set( 'log_errors', '1' );

    // Meant to be run once when the ticket system is set up. Over http it
    // walks the shard account table and writes to two databases, so anyone who
    // could reach the url could make it run; require an admin session there,
    // the same way the cron scripts do.
    if (PHP_SAPI !== 'cli') {
        session_start();
        if (!isset($_SESSION['ticket_user']) || !Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))) {
            header('HTTP/1.1 403 Forbidden');
            echo 'Access denied';
            return;
        }
    }

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
        
        
    