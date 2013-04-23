#! /bin/csh
# Check that calibration database environment variables are set; if not, use defaults

alias set_defaults 'echo "Calibration database environment not set.  Using defaults."; \
   setenv ENV_TSQL_URL "mysql://trcaldb.t2k.org/nd280calib"; \
   setenv ENV_TSQL_USER "t2kcaldb_reader"; \
   setenv ENV_TSQL_PSWD "rdt2kcaldb"; \
'

if (! $?ENV_TSQL_PSWD ) then
  set_defaults
endif

if (! $?ENV_TSQL_USER ) then
  set_defaults
endif

if (! $?ENV_TSQL_URL ) then
  set_defaults
endif
