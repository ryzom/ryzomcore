<?php
/** 
\defgroup Plugins Plugin management system 
\section section1 AMS plugin Requirements and functionalities.
The following are Ams Plugin Requirements and techniques for  users and developers.
\subsection subsection11 Plugin Management: Files included in the AMS
These are the files that must be in the plugins folder:
\n --> . \b info -- This file contains the information related to plugins and is described in below section.
\n --> \b Plugin_Name.php -- This file contains the hooks for the plugins . For more info related to hooks see last section: creating hooks. 
\subsection subsection1 Plugin Management: Supportable Extensions 
Plugin management system always looks for the Plugins which are compressed with .zip extension.If the uploaded plugin is not a file type of "zip/application", it throws an error and stops updloading.
\subsection subsection2 Plugin Management: .info File
Plugins always with a .info file inside it which contains the information related to plugins which will be displayed below, otherwise installation will not proceed further.
\n\b -------- Content of the .info File----------
\n\b PluginName = Sample Plugin
\n\b Description = Sample Plugin shows sample
\n\b Version = 1.0.0
\n\b TemplatePath = Path to the template
\n\b Type = Manual or Automatic
\n\n If this is an update there must be a field in the .info file
\n\b UpdateInfo = what is updated?
\subsection subsection3 Plugin Management: Versioning
For all plugins we have used semantic versioning
\n -->Format: X.Y.Z ,X->Major, Y->Minor, Z->Patch
\n -->change in the X Y & Z values refer the type of change in the plugin.
\n -->for initial development only Minor an Patch MUST be 0.
\n -->if there is bug fix then there MUST be an increment in the Z value.
\n -->if there is change in the functionality or addition of new functionality
\n -->then there MUST be an increment in the Y value.
\n -->When there is increment in the X value , Y and Z MUST be 0.
\n -->comparing if there is some change
\n -->For more info refer: https://semver.org
\subsection subsection4 Plugin Management: Naming Conventions
--> The plugin folder and hooks file must have same names in the format 
\n ----> Plugin Folder name = "Sample_Plugin"
\n ----> Plugin hooks file = "Sample_Plugin.php"
\n--> All the fields in the .info file must follow the pattern as written in sample above.
\subsection subsection5 Plugin Management: Installing New plugins
To install a plugin we have to upload the plugin compressed with zip extension.There is an option to Install Plugin in the AMS plugins template which redirects
the admin to the uploading pannel. Here, in uploading pannel user uploads the plugin which he want to install and an option to install will occur when uploads finished.
Now, admin select the option to install plugin , AMS starts checking for the .info file and compare names for the plugin folder with hook file.
If all successfully completed without error your plugin is installed.
\subsection subsection6 Plugin Management: Activate Plugin
When a plugin is installed now you can perform many actions on it, one of them is Activate plugin.
When admin selects Activate Plugin option , AMS start executing hooks available in the Plugin.
\subsection subsection7 Plugin Management: Deactivate Plugin
When the admin wants to deactivate a running Plugin , Deactivate Plugin option can help him out.
\subsection subsection8 Plugin Management: Delete Plugin
When a admin want to Delete a plugin from the AMS , he must have to check if the plugin is activated or not.
\n if active then to use delete option he must have to deactivate that plugin.
\n if inactive then the delete option is already there. 
\subsection subsection9 Plugin Management: Install Update 
If admin wants to install the update follow the following steps:
\n --> Modify the changes or write the code which need to be added.
\n --> Improve the Versioning by following the Versioning sections.
\n --> Add a field 'UpdateInfo' (as described above in .info file section) in the .info file.
\n --> compress the plugin again with zip extension and upload it in the Install Plugin part.
\n\n When the upload is validated , the update is added to the updates template with proper information provided.
If admin wants to apply update to the plugin , he have to select the Update option.

\section section2 Creating Plugins for AMS:
\subsection subsection10 Creating Local and Global Variables
\b Global \b Variables:
\n --> These variables are defined to store the Global information or information that are usable Globally in AMS.
\n --> These must be an array to store data with key => value pair.
\n --> This is the only variable which is returned after going through all hooks.
\n --> Sample:
\n \n	
\b $return_set \b = \b array();
\n \n All the information that hooks return will store in it to use with smarty loader or other functionalities.
\n --> Field that must be defined to display in menu bar :
\n \n
\b $return_set[ \b 'menu_display'] \b = \b 'Name \b to \b display \b in \b menu \b bar' 
\n \n
\n \b Local \b Variables:
\n --> These variables are defined to store the local information or information that are usable in hooks.
\n --> It is used to store values which we get from GET and POST requests.
\n --> These must be an array to store data with key => value pair.
\n --> Sample:
\n \n	
\b $var_set \b = \b array();
\n \n All the information that needs to be usable in hooks is stored in this local array.
\subsection subsection12 Creating Global Hooks
\b Defining \b Hooks:
\n --> Hooks are defined as  \b $PluginName_hook_task().
\n --> Where \b $PluginName must be same as the file name in which hooks are defined.
\n Example:
\n 
 --> \b Display \b hook:
\n \n 
\b function \b PluginName_hook_display()
\n \b {
\n	\b $return_set[ \b 'menu_display'] \b = \b 'PluginName' 
\n \b }
\n \n 
--> For creating hooks for storing and fetching data from databases. You have to define hook like \b $PluginName_hook_store_db() or \b $PluginName_hook_get_db(), Then create the object of the DBLayer class present in AMS libraries.

\n \b --> \b One \b Global \b hook \b must \b always \b be \b in \b hooks \b list \b that \b will \b return \b the \b Global \b variables.
\subsection subsection13 Creating Local Hooks
\b Defining \b Hooks:
\n --> Hooks are defined as  \b hook_task( \b $param,..).
\n --> Where \b $param must be the values stored in Local variables.
\n Example:
\n 
 --> \b cron \b hook:
\n \n 
\b function \b hook_set_cron( \b $connection, \b $path, \b $handle, \b $cron_file )
\n \b {
\n	\b ... \b statements \b ...... 
\n \b }
\n \n 
--> Local hooks are created to set or use values from GET and POST request or stored values stored in local variables.
\n --> Local hooks are called according to requests made by the server.(for more info see sample apps at the end of this section) 

\subsection subsection14 Plugins Already installed with AMS
\n \b --> \b Achievements.php
\n \b --> \b API_key_management.php
*/

