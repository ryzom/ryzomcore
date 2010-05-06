# Database : `nel_tool` 

# -------------------------------------------------------- 
# 
# Table structure for table `server` 
# 

CREATE TABLE server ( 
  sid int(11) unsigned NOT NULL auto_increment, 
  name varchar(64) default NULL, 
  address varchar(64) default NULL, 
  PRIMARY KEY (sid) 
) TYPE=MyISAM;

#
# Dumping data for table `server` 
#
INSERT INTO server VALUES (1, 'Local Host', '127.0.0.1'); 

# -------------------------------------------------------- 
# 
# Table structure for table `service` 
# 
CREATE TABLE service ( 
  shid int(11) unsigned NOT NULL auto_increment, 
  shard varchar(64) default NULL, 
  server varchar(64) default NULL, 
  name varchar(64) default NULL, 
  PRIMARY KEY (shid) 
) TYPE=MyISAM; 

# 
# Dumping data for table `service` 
# 
INSERT INTO service VALUES (1, '300', 'Local Host', 'localhost'); 

# -------------------------------------------------------- 
# 
# Table structure for table `variable` 
# 
CREATE TABLE variable ( 
  path text, 
  error_bound text, 
  alarm_order text, 
  graph_update text 
) TYPE=MyISAM; 

# 
# Dumping data for table `variable` 
#
