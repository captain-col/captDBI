#!/usr/bin/python

import datetime
import getopt
import os
import re
import sys
import gzip

import subprocess


# DEPENDENCIES
#
# allocate_seq_no.exe: must be on path (unless in --temporary_tables mode)
# mysql: must be on path (unless in --temporary_tables mode)


# Number of times to retry running a 'mysql' subprocess before giving up
MAX_ATTEMPTS = 3 

def usage():
    print """Database Updater Utility

    Note: Before invoking this utility the following database
    environmental variables must be set:-

    ENV_TSQL_URL This is a semi-colon separated list of URLs. Each
    URL takes the form:-
        
       protocol://host[:port]/[database]
         where:
             protocol - mysql
             host     - host name or IP address of database server
             port     - port number
                        database - name of database
         Example: mysql://myhost:3306/test

         ENV_TSQL_USER  The account user name. If  different names for
         different databases in the cascade then this can
         be a semi-colon separated list in the same order
         as ENV_TSQL_URL.  If the list is shorter than
         that list, then the first entry is used for the
         missing entries.

         ENV_TSQL_PSWD  The account password here. As with ENV_TSQL_USER
         it can be a semi-colon separated list with the
         first entry providing the default if the list is
         shorter than ENV_TSQL_URL.

         Although the oaOfflineDatabase supports multi-database
         environmental variables this utility ALWAYS uses the first
         entry.

    Invocation:  database_updater.py [<options>] <command> [<arg>]

    <options> are:-

        --convert_unsigned
                    Convert ANY integer data > 2147483647 to signed before
                    storing by subtracting 4294967296. Useful for
                    unsigned channel IDs.
                    CAUTION: Don't use if table contains BIGINTs.
                    
        --temporary_tables
                    * Any time this script would generate a 'CREATE
                      TABLE' statement, generate 'CREATE TEMPORARY
                      TABLE' instead.
                    * Do not connect to the database
                      --> SEQNOS are local to the temporary Validity Table
                      --> Print generated SQL instead of executing it
                          (non-SQL trace output is also silenced)
                    Note: any CREATE TABLE statements in the
                          update-file must be manually modified.
                    (for use from src/TTempTable.cxx)
                    
        --debug     Print out all MySQL commands
        --help      Prints this help.
        
    <command> [<arg>] is one of:-
              
          apply_global_update <update-file>  
              Applies update using global sequence numbers (SEQNOs)
              - fails if not authorising DB i.e. does not have a
                GLOBALSEQNO table 

          apply_local_update  <update-file>
              Applies update using local sequence numbers (SEQNOs)

          drop_table <table-name>
              Removes table (main and VLD) and any entry in
                GLOBALSEQNO and LOCALSEQNO tables 
              - asks confirmation if DB is authorising i.e. has a
                GLOBALSEQNO table 

    <update-file> can contain any number of the following:-
  
    o   Blank lines and comments (# in column 1)
  
    o   SQL  <sql-command>;
        Any arbitrary SQL command that ends with a ';'.  Can span
           several lines.  
        e.g.  SQL drop table
              if exists DEMO_DB_TABLE;
              
    o   BEGIN_TABLE <table-name> <start-date> <end-date>
            <aggregate-number> <creation-date> {<task>} {<key>=<value> ...}
        If not supplied a value of 0 is assumed for <task>.
        Valid <key>=<value> sets are:-
          SIMMASK=<value>  where <value> is one of: 'Data', 'MC' or 'all'
          EPOCH=<value>    where <value> is a small integer in range 0..100
            Note: If creating the VLD table, this forces it to have
                  EPOCH and REALITY columns.  Although it is not
                 required, this should be used for all new entries. 
        If using a <key>=<value> set the task value must also be supplied. 
        Followed by one or more rows of data, one per line in the form
            <value>,<value>, ... <value>
            
        This utility supplies both SEQNO and ROW_COUNTER to the start
        of each row 
        e.g.  BEGIN_TABLE DEMO_DB_TABLE  '2009-01-01 00:00:00' \
                '2009-02-00 00:00:00' 0 '2009-04-07 18:00:00' 0 EPOCH=0
              -2006450160, 101, 201, 301, 1.01, 2.01, 3.01
              -2006450170, 102, 202, 302, 1.02, 2.02, 3.02

"""
########################  DatabaseInterface  ########################
########################  DatabaseInterface  ########################
########################  DatabaseInterface  ########################

class DatabaseInterface:
    """A simple interface to a MySQL Database.

    Makes no assumption about the tables that the database holds."""

    def __init__(self,parent,testConnection=1):

        ##  Initialise state

        self.parent               = parent # The parent DatabaseUpdater
        self.debug                = parent.debug
        # Set the implementation program used to send commands to DB.
        #     This is "mysql" or "sqlite3"
        self.access_cmd       = ""
        # Access string needed for command:
        # mysql <access string> --execute=<command>
        self.access_string        = "" 
        self.SetAccessString()
        self.public_access_string = re.compile(r'--password=\S+')\
          .sub('--password=XXX',self.access_string)
        
        self.results = []

        if testConnection:
            if self.IsOK():
                print 'DatabaseInterface initialising with account info: ' \
                  + self.public_access_string
            else:
                print 'MySQL DBI connection failed with account info: ' \
                  + self.public_access_string

########################  DatabaseInterface  ########################

    def IsOK(self):
        """Return True if able to execute queries."""
        if not self.access_string: return False
        #See if we can run a dummy statement
        return self.Query('select 1;',False)
        
########################  DatabaseInterface  ########################

    def GetResults(self):
        """Return the results from the last query."""
        return self.results

########################  DatabaseInterface  ########################

    def Query(self,sql,log_error = True):
        """Apply query and return True if successful.

        Unless log_error is False, errors are logged."""

        num_attempts = 0
        
        while True:
        
            if num_attempts >= MAX_ATTEMPTS:
                print "ERROR: Failed to execute command after %d attempts." \
                  % (MAX_ATTEMPTS, )
                return False
            
            if num_attempts > 0:
                print "WARNING: Invocation failed; retrying:"
        
            self.results = []
        
            cmd = '%s %s ' % (self.access_cmd, self.access_string, )
            if self.debug:
                print 'DEBUG About to execute: "%s"' % (cmd,)
                for line in sql.splitlines():
                    print 'DEBUG  SQL = "%s"' % (line, )
            
            p = subprocess.Popen(cmd,
                                 stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT, shell=True)
            
            #import time
            #if sql.find("DEMO_DB_TABLE VALUES (") != -1:
            #  time.sleep(5)
            
            (stdoutdata, _) = p.communicate(sql)
   
            if log_error and p.returncode != 0:
                print "SQL Error Log: query failed with error code %d" \
                  % (p.returncode, )
            
            for line in stdoutdata.splitlines():
                if self.debug: print 'DEBUG result="%s"' % (line)
                line = line.strip()
                if log_error and p.returncode != 0:
                    print "	SQL Error Log: %s" % (line, )
                self.results.append(line)
             
            num_attempts += 1
            if p.returncode == 1 \
              and (stdoutdata.find('connection to MySQL server during') != -1 \
                   or stdoutdata.find('server has gone away') != -1):
                continue
            else:
                break
            
        return p.returncode == 0

########################  DatabaseInterface  ########################

    def SetAccessString(self):
        """Prepare self.SetAccessString from TSQL environment."""
        self.access_string  = ""
        env_url  = os.environ.get('ENV_TSQL_URL')
        env_user = os.environ.get('ENV_TSQL_USER')
        env_pswd = os.environ.get('ENV_TSQL_PSWD')
        if not env_url:
            print 'ENV_TSQL_URL not defined'
            return
        #  Collect server and db and port from from first element of
        #  ENV_TSQL_URL.
        implementation = None
        server = None
        db = None
        mo = re.search(r'mysql://(.*?)/(\w+)',env_url)
        if mo:
            (server,db) = mo.groups()
            implementation = "mysql"
        else:
            mo = re.search(r'sqlite://(.*)',env_url)
            if mo:
                db = mo.group(1)
                implementation = "sqlite"
        if implementation == None:
            print 'Cannot parse the environmental variable ENV_TSQL_URL'
            return
        port_opt = ''
        if server != None:
            mo = re.search(r'^(.*):(\d+)$',server)
            if mo:
                server = mo.group(1)
                port_opt = '--port=' +  mo.group(2)
        #  Collect user from first element of ENV_TSQL_USER
        if not env_user:
            print 'ENV_TSQL_USER is not defined.'
            return
        user = env_user
        mo = re.search(r'(.*?);',env_user)
        if mo: user = mo.group(1)
        #  Collect pswd from first element of ENV_TSQL_PSWD
        if not env_pswd:
            print 'ENV_TSQL_PSWD is not defined.'
            return
        pswd = env_pswd
        mo = re.search(r'(.*?);',env_pswd)
        if mo: pswd = mo.group(1)
        pswd_opt = '--password=' + pswd
        if pswd == '\\0': pswd_opt = ''
        if implementation == "mysql":
            self.access_cmd = "mysql"
            self.access_string='%s --compress --host %s %s --user=%s %s ' \
            % (db,server,port_opt,user,pswd_opt)
        elif implementation == "sqlite":
            self.access_cmd = "sqlite3"
            self.access_string='%s ' % (db,)
            

########################  DatabaseInterface  ########################

    def TableExists(self,table_name):
        """Return True if table exists."""
        if self.access_cmd == "mysql":
            result = self.Query('describe ' + table_name,False)
            if self.debug: print "DEBUG: " + str(result)
            return result
        elif self.access_cmd == "sqlite3":
            result = self.Query('.tables ' + table_name, False)
            if self.debug: print "DEBUG: " + str(result)
            if len(self.results) == 0: return False;
            return result
               
########################  FakeDatabaseInterface  ########################
########################  FakeDatabaseInterface  ########################
########################  FakeDatabaseInterface  ########################
class FakeDatabaseInterface:
    """Provides the interface of DatabaseInterface, but simply prints
    the SQL to STDOUT instead of executing it"""
    
    def __init__(self,parent,testConnection=1):
        pass

    def IsOK(self):
        return True
        
#    def GetResults(self):
#        """Return the results from the last query."""
#        return self.results

    def Query(self,sql,log_error=True):
        print sql
        return True

    def TableExists(self,table_name):
        # This is a bit bogus, but we do it anyways because
        # TableUpdate::__init__() required it.  we are assuming that
        # the temporary constants table has been created (i.e.  the
        # user did not forget to include 'CREATE TEMPORARY TABLE ...'
        # in the update-file) but cannot check because
        # --temporary-tables mode functions without ever actually
        # connecting to the db.  --> Would be nice to have a test case
        # for this
        return True
        
########################  TableUpdate  ########################
########################  TableUpdate  ########################
########################  TableUpdate  ########################
########################  TableUpdate  ########################
class TableUpdate :
    """A table update for a single aggregate."""
    
    def __init__(self,parent,begin_line,is_global):
        # Parent DatabaseUpdater
        self.parent           = parent
        # Debug flag
        self.debug            = parent.debug
        # Database Interface
        self.dbi              = parent.dbi 
        # Determine local/global SEQNO
        self.is_global        = is_global 
        # Set True if any error found while assembling update
        self.failed           = False 
        # Set True once applied.
        self.applied          = False 
        # Number of unsigned integers converted
        self.num_conv_unsign  = 0 
        # List of rows of data.
        self.rows             = [] 
        # Table name parsed from BEGIN_TABLE line
        self.table_name       = "" 
        # Start date parsed from BEGIN_TABLE line
        self.start_date       = ""
        # End date parsed from BEGIN_TABLE line
        self.end_date         = ""
        # Hardwired, at least for now
        self.detectormask     = "1"
        # Overridden by the option SIMMASK=
        self.simmask          = "1"
        # Overridden by the option EPOCH=
        self.epoch            = ""
        # Aggregate number parsed from BEGIN_TABLE line
        self.aggregate        = ""
        # Task number optionally parsed from BEGIN_TABLE line (default 0).
        self.task             = "0"
        # Creation date parsed from BEGIN_TABLE line
        self.creation_date    = ""
        # Empty until Apply method executed.
        self.seqno            = ""
        
        # Parse BEGIN_TABLE line
        mo = re.search(r"^BEGIN_TABLE\s+(\w+)"
                       +r"\s+'(.*?)'"
                       +r"\s+'(.*?)'"
                       +r"\s+(\d+)"
                       +r"\s+'(.*?)'(|\s+(.*))$",begin_line)
        #    BEGIN_TABLE   TNAME   'START'   ' END '   AGGRE   'CREAT' dummy
        # dummy is matched to <space><options> if 1 or more options
        # exist otherwise is the empty string. if dummy != "" then
        # <options> is matched to options 
        if not mo:
            print "Failing update; cannot parse line: " + begin_line
            self.failed = True
            return
        (self.table_name,self.start_date,self.end_date,\
         self.aggregate,self.creation_date,dummy,options) = mo.groups()
        if options:

            # Parse out {<task>} {key=value key=value ...}
            options = options.lower()
            #next two lines grab the task number if it is supplied as
            #a lone digit  (can be over-written by a subsequent
            #TASK=N)
            mo = re.match(r"\s*(\d+)(|\s+(.*))$",options)
            if mo: (self.task,dummy,options) = mo.groups()
            while options:
                mo = re.search(r"^(\S+)=(\S+)(|\s+(.*))$",options)
                if not mo:
                    print "Failing update; cannot parse: '%s'" % options
                    self.failed = True
                    return
                (key,value,dummy,options) = mo.groups()

                # Deal with TASK=
                if key == "task":
                    print "TASK from key/value"
                    self.task = int(value)
                        
                # Deal with SIMMASK=
                elif key == "simmask":
                    if   value == "data" : self.simmask = "1"
                    elif value == "mc"   : self.simmask = "4"
                    elif value == "all"  : self.simmask = "-1"
                    else:
                        print "Failing update; bad SIMMASK: '%s'" % value
                        self.failed = True
                        return

                # Deal with EPOCH=
                elif key == "epoch":
                    try:
                        val_int = int(value)
                        if val_int >= 0 and val_int <= 100: self.epoch = value
                        else: raise
                    except:
                        print "Failing update; bad EPOCH: '%s'" % value
                        self.failed = True
                        return
                    
                else:
                    print "Failing update; bad option key: '%s'" % key
                    self.failed = True
                    return

        if not self.dbi.TableExists(self.table_name):
            print "Failing update; table %s does not exist." % self.table_name
            self.failed = True
            return
        for date in (self.start_date,self.end_date,self.creation_date):
            if not re.search(r'\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d',date):
                print "Failing update; bad date '%s' in %s." % (date,begin_line)
                self.failed = True
                return

            
    def AddRow(self,row_line): # TableUpdate
        """Add a row."""

        # Look for unsigned integers to convert if required.
        if ( not self.parent.convert_unsigned ) :
            self.rows.append(row_line.strip())
            return
        converted_line = ""
        for value in row_line.strip().split(','):
            try:
                int_value = int(value)
                if int_value > 2147483647:
                   int_value -= 4294967296
                   self.num_conv_unsign += 1
                   value = str(int_value)
            except: pass
            if converted_line: converted_line += ","
            converted_line += value
        self.rows.append(converted_line)
                

    def Apply(self): # TableUpdate
        """Apply update and return True if successful.
        The table must already exist but it's VLD will be created if
        necessary. 
        If it fails it attempts to remove any data already committed."""

        if not self.CanApply():
            print "Cannot apply this data."
            return False

        # If required, create VLD table.
        
        # Warning: TempTableUpdate::need_to_create_vld_tbl() has a side effect
        if self.need_to_create_vld_tbl() == True:
            if self.parent.temporary_tables == False:
                print "  Creating table %sVLD" % self.table_name
            sql = "%s %sVLD (" % (self.create_stmt_start, self.table_name)
            sql += "SEQNO integer not null primary key"
            sql += ",TIMESTART datetime not null"
            sql += ",TIMEEND datetime not null"
            if self.epoch != "":
                sql += ",EPOCH smallint"
                sql += ",REALITY smallint"
            sql += ",DETECTORMASK smallint"
            sql += ",SIMMASK smallint"
            sql += ",TASK integer"
            sql += ",AGGREGATENO integer"
            sql += ",CREATIONDATE datetime not null"
            sql += ",INSERTDATE datetime not null"
            # sql += ",key TIMESTART (TIMESTART) "
            # sql += ",key TIMEEND (TIMEEND)"
            sql += ");"
            
            if not self.dbi.Query(sql):
                print "Cannot create vld table"
                return False
        
        if self.parent.temporary_tables == False:
            print "  Updating %s validity '%s' - '%s' aggregate %s task %s..."\
                % (self.table_name,self.start_date,self.end_date,\
                   self.aggregate,self.task)
            if self.num_conv_unsign:
        	    print "   (%d unsigned integer converted before update)" \
                  % self.num_conv_unsign
    	    
    	self.seqno = self.pop_next_seqno()    

        # Process rows
        chunks = [ "(%d, %d, %s)" % (seqno, row_num, row) \
                   for (seqno, row_num, row) in \
                   zip([self.seqno]*len(self.rows), \
                       range(len(self.rows)+1)[1:], self.rows) ]

        dataLine = ""
        for chunk in chunks:
            if len(dataLine) > 1: dataLine += ", "
            dataLine += chunk
            if len(dataLine) > 2000:
                sql = "INSERT INTO %s VALUES %s;" % (self.table_name, dataLine)
                if not self.dbi.Query(sql):
                    print "Update has failed: " + sql
                    print "--> Attempting to remove any data"
                    self.RemoveSeqno(self.seqno)
                    return False
                dataLine = ""

        if len(dataLine) > 0:
            sql = "INSERT INTO %s VALUES %s;" % (self.table_name, dataLine)
            if not self.dbi.Query(sql):
                print "Update has failed: " + sql
                print "--> Attempting to remove any data"
                self.RemoveSeqno(self.seqno)
                return False

        # Add VLD entry

        sql  = "INSERT INTO %sVLD VALUES (" % self.table_name
        sql += "%d, '%s', '%s'," % \
               (self.seqno,self.start_date,self.end_date)
        if self.epoch != "": sql += "%s, 0," % self.epoch
        sql += "'%s', '%s', %s, %s, '%s', '%s');" % \
               (self.detectormask,self.simmask,self.task,\
                self.aggregate,self.creation_date,\
                datetime.datetime.today().strftime("%Y-%m-%d %H:%M:%S"))
        if not self.dbi.Query(sql):
            print "Update has failed"
            print "--> Attempting to remove any data"
            self.RemoveSeqno(self.seqno)
            return False
        
        self.applied = True
        return True
    

    def CanApply(self): # TableUpdate
        """Return True if no errors have been found processing the data,
        it has not already applied and it has some rows."""
        return not self.failed and not self.applied and self.rows

########################  RealTableUpdate  ########################
########################  RealTableUpdate  ########################
########################  RealTableUpdate  ########################
########################  RealTableUpdate  ########################
class RealTableUpdate(TableUpdate):

    create_stmt_start = "CREATE TABLE"

    def __init__(self,parent,begin_line,is_global):
        TableUpdate.__init__(self,parent,begin_line,is_global)
        
    def need_to_create_vld_tbl(self): # RealTableUpdate
        return not self.dbi.TableExists(self.table_name + "VLD")

    def pop_next_seqno(self): # RealTableUpdate
        seqno = ""
        print "    requesting sequence number ... ",
        require_global = -1
        if self.is_global: require_global = 1
        cmd = 'allocate_seq_no.exe %s %d | tail -1' \
          % (self.table_name,require_global)
        if self.debug: print "About to execute " + cmd
        inp = os.popen(cmd,"r")
        line = inp.readline()
        mo = re.search(r'(\d+)',line)
        if mo:  seqno = int(mo.group(1))
        if not seqno:
            print "failed to allocate a sequence number!"

            self.failed = True
            return False
        print "allocated sequence number %d" % seqno

        # Just to be on the safe side remove this sequence number.
        self.RemoveSeqno(seqno)
        
        return seqno

    def RemoveSeqno(self, seqno): # RealTableUpdate
        """Attempt to remove sequence number."""
        self.dbi.Query("DELETE FROM %s    WHERE SEQNO = %s" \
                       % (self.table_name,seqno))
        self.dbi.Query("DELETE FROM %sVLD WHERE SEQNO = %s" \
                       % (self.table_name,seqno))
        
########################  TempTableUpdate  ########################
########################  TempTableUpdate  ########################
########################  TempTableUpdate  ########################
########################  TempTableUpdate  ########################
class TempTableUpdate(TableUpdate):

    create_stmt_start = "CREATE TEMPORARY TABLE"
    
    # Map of {table_nameVLD->next_seqno}
    # Provides 2 pieces of information:
    # * Has this table already been created by the current script invocation?
    #   (caution: update-file should support multiple BEGIN_TABLE lines with
    #   the same table-name, with other table-names in between)
    # * What is the seqno for the next Validity Interval?
    #   (caution: update-file should support multiple Validity Intervals)	
    vld_tables={}

    def __init__(self,parent,begin_line,is_global):
        TableUpdate.__init__(self,parent,begin_line,is_global)
        
    def need_to_create_vld_tbl(self): # TempTableUpdate
        if self.table_name+'VLD' in self.__class__.vld_tables:
            return False
        else:
            self.__class__.vld_tables[self.table_name+'VLD'] = 1
            return True
    
    def pop_next_seqno(self): # TempTableUpdate
        seqno = self.__class__.vld_tables[self.table_name+'VLD']
        self.__class__.vld_tables[self.table_name+'VLD'] = seqno + 1
        return seqno
    
    def RemoveSeqno(self, seqno): # TempTableUpdate
        pass
    
########################  DatabaseUpdater  ########################
########################  DatabaseUpdater  ########################
########################  DatabaseUpdater  ########################
########################  DatabaseUpdater  ########################
class DatabaseUpdater :
    """The main work routine that applies ASCII update files to a
    database. The constructor also invokes command execution.  The
    only action in __main__ is to construct an object of this class.

    """ 

    
    def __init__(self): # DatabaseUpdater
        #  Get command state.
        self.convert_unsigned = False    # --convert_unsigned option.
        self.temporary_tables = False    # --temporary_tables option
        self.debug            = 0        # Set to 1 if --debug
        self.command          = ''       # The command supplied
        self.args             = ''       # The command args (if any)
        self.ParseCommand()

        if self.temporary_tables == True:
            dbi_class = FakeDatabaseInterface
            self.tu_class = TempTableUpdate
        else:
            dbi_class = DatabaseInterface
            self.tu_class = RealTableUpdate
            
        # Connect the database.
        self.dbi         = dbi_class(self)
        if not self.dbi.IsOK():
            print "\nCannot connect to database"
            print "please check values of ENV_TSQL_URL/USER/PSWD"
            print "Aborting."
            return
        
        # Process command
        if   self.command == "apply_global_update":
            self.ExecuteApplyUpdate(True)
        elif self.command == "apply_local_update":
            self.ExecuteApplyUpdate(False)
        elif self.command == "drop_table":
            self.ExecuteDropTable()
        else:
            print "\nUnknown command %s.\n" % self.command
            return

    

    def ExecuteApplyUpdate(self,is_global): # DatabaseUpdater
        """Apply updates either local or global sequence numbers."""

        update_file = self.args[0]

        if not os.path.isfile(update_file):
            print "\n Cannot '%s'; cannot find update file '%s'" \
              % (self.command,update_file)
            return

        # IsAuthorising() shall not be executed if
        # self.temporary_tables == True,  because the option requires
        # command=local_update, making is_global=False --> Would be
        # nice to set up a test case for this
        if is_global and not self.IsAuthorising():
            print "\n Cannot 'apply_global_update';"
            print " connected database is not authorising"
            print " i.e. does not have a GLOBALSEQNO table"
            return

        # Open the input file.  It might be a compressed file, so this
        # is where we check.
        if re.search(r'.*gz$', update_file):
            file_update = gzip.open(update_file)
        else:
            file_update = open(update_file)
            
        # Read the update file and create a list of TableUpdate
        tu = None
        for line in file_update:
            # Ignore blank lines and comments
            if re.search(r'^\s*$',line) or re.search(r'^\s*#',line): continue
            # Process SQL
            mo = re.search(r'^\s*SQL\s+(.*)',line)
            if mo:
                if not self.ExecuteSQL(mo.group(1),file_update): return
                continue
            if not tu or re.search(r'^\s*BEGIN_TABLE',line):
                # Complete process of last update (if any)
                if tu:
                   if not tu.Apply():
                       print "\n Aborting '%s'; file '%s' w/ bad update." \
                         % (self.command,update_file)
                       return
                   tu = None
                tu = self.tu_class(self,line,is_global)
            else:
                tu.AddRow(line)
        file_update.close()

        # Apply last update
        if not tu:
            print "No table data in '%s': will only apply SQL statements." \
              % (update_file)
            return
        if not tu.Apply():
            print "Aborting '%s'; file '%s' with bad update." \
              % (self.command,update_file)
            return
        

    def ExecuteDropTable(self): # DatabaseUpdater
        """Removes table (main and VLD) and any entry in GLOBALSEQNO
        and LOCALSEQNO tables."""  

        table_name = self.args[0]
        if not self.dbi.TableExists('table_name'):
            print 'Database does not have table %s' % table_name
        else:

            # Table exists, ask for confirmation 
            print '\nDo you want to remove tables %s and %sVLD Ans:[y or n]' \
              % (table_name,table_name),
            ans = raw_input()
            if ans[0] != 'y' and ans[0] != 'Y':
                print "\nRequest not confirmed, table not deleted."
                return
            if self.IsAuthorising():
                print '\nAre you sure SURE? (connected Master) Ans:[y or n]',
                ans = raw_input()
                if ans[0] != 'y' and ans[0] != 'Y':
                    print "\nRequest not confirmed, table not deleted."
                    return

            # Drop table and it's VLD.
            for table in (table_name, table_name + 'VLD'):
                if self.dbi.TableExists(table):
                    print "Dropping table " + table
                    if not self.dbi.Query('DROP TABLE %s' % table):
                        print "\nFailed to drop table %s." % table
                        return

        # Drop entries in LOCALSEQNO and GLOBALSEQNO
        for seqno_table in ('LOCALSEQNO','GLOBALSEQNO'):
            if self.dbi.TableExists(seqno_table):
                print "Removing entry from %s" % seqno_table
                self.dbi.Query("DELETE FROM %s WHERE TABLENAME = '%s'" \
                                % (seqno_table,table_name))
                            
    def ExecuteSQL(self,sql_start,file_update): # DatabaseUpdater
        """Assemble and execute SQL command.  Return True if no errors
        when executed.""" 
        sql = sql_start.strip()
        if not re.search(r';\s*$',sql):
            for line in file_update:
                sql += line.strip()
                if re.search(r';\s*$',sql): break
                
        return self.dbi.Query(sql);

    	        
    def IsAuthorising(self): # DatabaseUpdater
        """Return True if database is authorising i.e. has a
        GLOBALSEQNO table.""" 
        return  self.dbi.TableExists('GLOBALSEQNO')

    
    def ParseCommand(self): #  DatabaseUpdater
        try:  opts, args = getopt.getopt(sys.argv[1:], "ctdh",\
                                         ["convert_unsigned",\
                                         "temporary_tables",\
                                         "debug",\
                                         "help"])
        except getopt.GetoptError, err:
            print str(err) 
            usage()
            sys.exit(2)

        # Process options
        for o, a in opts:
            #  Deal with --convert_unsigned
            if o in ("-c","--convert_unsigned"): self.convert_unsigned = True
            elif o in ("-t", "--temporary_tables"):
                self.temporary_tables = True
            #  Deal with debug
            elif o in ("-d", "--debug"): self.debug = 1
            #  Deal with help
            elif o in ("-h", "--help"): 
                usage()
                sys.exit()
            else:
                assert False, "unhandled option: " + o

        # Process command and args
        if not args:
            print "\nNo command supplied.  See -h option\n"
            usage()
            sys.exit()
        self.command = args.pop(0)
        
        if self.temporary_tables == True \
          and self.command != 'apply_local_update':
            print "--temporary_tables only for command=apply_local_update"
            sys.exit()
        
        if not args:
            print "\nNo command args supplied.\n"
            # usage()
            sys.exit()
        self.args = args


#  Driver code.
if __name__ == "__main__":
    dbu = DatabaseUpdater()

