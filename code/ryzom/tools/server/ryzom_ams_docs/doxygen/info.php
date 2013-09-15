<?php

/*! \mainpage The Ryzom AMS information pages.
 *
 * \section intro_sec Introduction
 *
 * Welcome to the documentation pages of the ryzom account management system library.\n Doxygen is being used to generate these webpages. They should offer a good reference for anyone who is interested in working with the AMS library. 
 *
 * \section install_sec More info?
 *
 * if you want more information take a look at the ryzomcore wikipages and the \link design \endlink pages
 * 
 */


/**
* @page design Design Info
* \section intro_design A brief introduction to the design of the AMS
* 
* We will take a look at the current db design, the way the classes are designed, the way the WWW version works and how it was reused in the drupal module.
*
* \subsection db_struct The database structure
*
* <p>My project started with the design of our database. I had to think about the advanced AMS features in advance. This is the reason why there are still a few unused DB tables in the design, the plan however is to use those as soon as possible by implementing the extra features.</p>
* 
* <p>The tables that are unused are the following:
* <ul>
*  <li>ticket_group</li>
*  <li>in_group</li>
*  <li>tag</li>
*  <li>tagged</li>
* </ul>
* <i>The idea for the ticket_groups was to provide the ability to bundle multiple tickets together in groups, this could be used for tickets that are alike or are in a weird way related. The idea for the tagging was
* to provide an extra system that allows to query tickets based on their tags (datamining). These features can be easily added in the future!</i></p>
* 
* <p>Let's take a look at the 'used' tables. The database structure is shown in the image below. For each table I made a matching class that handles the info of that table.</p>
* <p>Quite central you can see the <b>ticket</b> table. As you can see, a ticket has a ticket_category field and author field, these hold the id of the related row in the <b>ticket_category</b> and <b>ticket_user</b> table.
* There's also the relation between a ticket and it's log entries, this is done by the ticket foreign key in the <b>ticket_log</b> table. The same counts for most other tables that are related to the ticket, they all got a ticket column used as foreign key.</p>
* <p>Another thing that you might notice is the separation between <b>ticket_reply</b> and <b>ticket_content</b>, this is a 1-to-1 relation and this makes it easier to search between the replies if we only need their general information without having to take care of the content.</p>
* The <b>ticket_user</b> is another quite important table that's being foreigned keyed by the others. It holds the permission of a user and the externID links to an ID given by the CMS(or our own www version)</p>
* <p><i>Most things are pretty clear and straight forward, you can find the MYSQL Workbench file in the ryzom_ams/www/html/sql folder, which might give a better overview and can be used to update the DB easily when adding/modifying features in the future.</i></p>
* \image html db.png
*
** \subsection used_tech Technologies used
* <ul>
*  <li>Smarty (for templating) (http://www.smarty.net/)</li>
*  <li>multiple language .ini files to support different languages</li>
*  <li>Charisma (WWW layout (uses bootstrap)) (http://usman.it/themes/charisma/)</li>
*  <li>Drupal (drupal module) (https://drupal.org/)</li>
* </ul>
* 
*\subsection struct_info Information regarding the structure
* <p>As you might have noticed, the ryzom_ams directory contains 3 directories: the ams_lib dir, the www dir and a drupal_module dir.</p>
*  <p>
*  the ams_lib contains the following important dirs/files:
* <ul> 
*  <li><b>autoload dir</b> <i>holds all classes of the lib</i></li>
*  <li><b>cron dir</b> <i>holds the cron functions regarding email and the ams_querycache</i></li>
*  <li><b>ingame_templates dir</b> <i>holds the templates that are being used while ingame</i></li>
*  <li><b>smarty dir</b> <i>the smarty files (http://www.smarty.net/)</i></li>
*  <li><b>translations dir</b> <i>multiple .ini files, one for each language that's being supported.</i></li>
*  <li><b>libinclude.php</b> <i>php file that holds the __autoload function</i></li>
* </ul>
* </p>
* <p>
*  the www contains the following important dirs/files:
* <ul> 
*  <li><b>autoload dir</b> <i>holds the webusers.php file (which extends the Users.php file in the lib)</i></li>
*  <li><b>func dir</b> <i>holds php files that contain a function that is being executed after filling in a form.</i></li>
*  <li><b>inc dir</b> <i>holds php files that contain a function that is being executed before loading a specific file.</i></li>
*  <li><b>templates dir</b> <i>holds the templates being used outgame.</i></li>
*  <li><b>config.php</b> <i>php file that holds configuration settings</i></li>
* </ul>
* </p>
* <p>
*  the drupal_module contains the following important dirs/files:
* <ul> 
*  <li><b>autoload dir</b> <i>holds the webusers.php file that uses drupal functions (which extends the Users.php file in the lib)</i></li>
*  <li><b>func dir</b> <i>holds php files that contain a function that is being executed after filling in a form.</i></li>
*  <li><b>inc dir</b> <i>holds php files that contain a function that is being executed before loading a specific file.</i></li>
*  <li><b>templates dir</b> <i>holds the templates being used outgame.</i></li>
*  <li><b>config.php</b> <i>php file that holds configuration settings</i></li>
*  <li><b>ryzommanage.info</b> <i>drupal file that holds information being used by drupal</i></li>
*  <li><b>ryzommanage.install</b> <i>drupal file thats being used for installing the module</i></li>
*  <li><b>ryzommanage.module</b> <i>drupal file that holds all functionality that's being needed to handle the AMS in drupal. (read more about it at the wiki page)</i></li>
* </ul>
* <i><b>Important:</b> the func dir and inc dir in the drupal_module are almost empty, that's because the inc/func directories of the WWW version can be copied to the drupal version, they are exactly the same.
* However, because the drupal_module isn't completely up to date, the settings page doesn't has the extra fields (like gender,country,..) therefore the ingame template file, inc files related to that are still in the module.</i>
* </p>
* \subsection pageload How does the page loading work?
* \image html info.jpg
*
* \subsection classload How are the classes being used?
* <p>Like I mentioned above, each DB table has a class related that handles the data linked to that table and has functions working with that data.</p>
* <p>The private attributes of each class are similar to the fields in the DB table. Every class also has the following functions:
* <ul>
*   <li>function __construct()</li>
*   <li>function set($values)</li>
*   <li>function create()</li>
*   <li>function delete()</li>
*   <li>function load( $id) <i>or named similar</i></li>
*   <li>some also have: update ()</li>
* </ul>
* These methods are being used by the public static functions of that class, which represent the 'real' AMS-functions, the ones being used by the inc/func files.
* </p>
* <p>You can reference for example the Support_Group class's information, which shows this setup!</p>
*/
