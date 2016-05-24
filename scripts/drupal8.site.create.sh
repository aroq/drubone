#!/usr/bin/env bash

# Default option values
DEBUG=false
VERBOSE=false
TRACE=false
FORCE=false

# Initial state variables
DEBUGMSG=false
VERBOSEMSG=false

MSG='echo # '
ERROR='echo '

#
# Exit with a message if the previous function returned an error.
#
# Usage:
#
#   aborterr "Description of what went wrong"
#
function aborterr() {
  if [ $? != 0 ]
  then
    echo "$1" >&2
    exit 1
  fi
}

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#  P A R S E   C O M M A N D   L I N E   A R G S   A N D   O P T I O N S
#
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

while [ $# -gt 0 ] ; do

  option="$1"
  shift

  case "$option" in
    -v|--verbose)
      VERBOSE=true
      VERBOSEMSG='echo # '
      ;;

    -d|--debug)
      DEBUG=true
      DEBUGMSG='echo ### '
      VERBOSE=true
      VERBOSEMSG='echo # '
      ;;

    -q|--quiet)
      MSG=false
      ;;

    --trace)
      TRACE=true
      ;;

    --force)
      FORCE=true
      ;;

    --site.name)
      SITE_NAME=$1
      shift
      ;;

    -*)
      $ERROR "Unknown option $option" >&2
      exit 1
      ;;

    *)
      if [ -z "$SITE_NAME" ]
      then
        SITE_NAME="$option"
      else
        $ERROR "Too many arguments" >&2
        exit 1
      fi
      ;;
  esac
done

DRUPAL_ROOT=$(pwd)

SITE_DIR=${SITE_NAME}

# Turn on bash debugging if --trace is specified
if $TRACE
then
  set -x
fi

if $FORCE
then
  rm -fR "$DRUPAL_ROOT/sites/$SITE_DIR"
fi

if [ ! -d "core" ]; then
  $ERROR "Please launch this command from Drupal root. 'core' dir should be located in this dir."
  exit 1
fi

if [ ! -d "$DRUPAL_ROOT/sites/$SITE_DIR" ]; then
  CONFIG_RANDOM="$(base64 /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 75 | head -n 1)"

  mkdir -p "$DRUPAL_ROOT/sites/$SITE_DIR"

  if [ ! -f "$DRUPAL_ROOT/sites/$SITE_DIR/settings.php" ];
  then
    cp "$DRUPAL_ROOT/sites/default/default.settings.php" "$DRUPAL_ROOT/sites/$SITE_DIR/settings.php"
  fi

  grep -q "sites/$SITE_DIR/config_" "$DRUPAL_ROOT/sites/$SITE_DIR/settings.php"
  if [ "$?" != "0" ]
  then
    $MSG "Creating ${SITE_DIR} site..."
    HASH_SALT="$(base64 /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 75 | head -n 1)"
    $DEBUGMSG "Hash_salt is $HASH_SALT"

    chmod +w "$DRUPAL_ROOT/sites/$SITE_DIR"
    chmod +w "$DRUPAL_ROOT/sites/$SITE_DIR/settings.php"
    cat << __EOF__ >> "$DRUPAL_ROOT/sites/$SITE_DIR/settings.php"

\$settings['hash_salt'] = '$HASH_SALT';

\$config_directories['active'] = 'sites/$SITE_DIR/files/config_$CONFIG_RANDOM/active';
\$config_directories['sync'] = '../code/$SITE_DIR/config/sync';

if (file_exists(DRUBONE_DIR_FULL . '/includes/drupal8.include.settings.inc')) {
  include_once DRUBONE_DIR_FULL . '/includes/drupal8.include.settings.inc';
}
__EOF__

    mkdir -p "$DRUPAL_ROOT/sites/$SITE_DIR/files/config_$CONFIG_RANDOM/active"
    aborterr "Could not create the active configuration directory for the site."

    mkdir -p "$DRUPAL_ROOT/../code/$SITE_DIR/config/sync"
    aborterr "Could not create the staging configuration directory for the site."
  fi
else
  $ERROR "Directory ${SITE_DIR} already exists."
fi


