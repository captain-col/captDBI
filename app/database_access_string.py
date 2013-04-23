#!/usr/bin/python

from database_updater import DatabaseInterface
import sys

class HasDebug():
    """A dummy class to enable construction of a DatabaseInterface"""
    def __init__(self):
        self.debug =0

def usage():
    print """Returns the account string used by database_updater.py.
Example:     mysql `database_access_string.py`
This is convenient to connect interactively to the same DB with the same permissions."""


def main():
    """A simple script for evaluating the string database_updater.py will use for connecting to MySQL database""" 

    if len(sys.argv) > 1 : 
        usage()
        exit()
    hd = HasDebug() 
    dbi = DatabaseInterface(hd,0) 
    print dbi.access_string



if __name__ == "__main__":
    main()


