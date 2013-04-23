#! /bin/sh
# Check that Calibration database environment variables are set; if not, use defaults

set_defaults () {

#  For more information, see: 
#   * the package's doxygen documentation
#   * http://www.t2k.org/nd280/database/nd280calib

   echo "Calibration database environment variables not set."

   echo "Setting variables for: master database (without temporary table support)"
   export ENV_TSQL_URL='mysql://trcaldb.t2k.org/nd280calib'
   export ENV_TSQL_USER='t2kcaldb_reader'
   export ENV_TSQL_PSWD='rdt2kcaldb'

}


if [ -z ${ENV_TSQL_PSWD} ]; then
   set_defaults
fi

if [ -z ${ENV_TSQL_USER} ]; then
   set_defaults
fi

if [ -z ${ENV_TSQL_URL} ]; then
   set_defaults
fi


