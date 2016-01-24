#!/bin/sh

DRUPAL_VERSION=$1

if [ ! -d "drubone" ]; then
  echo "Please launch this command from project root. 'drubone' dir should exists."
  exit 1
fi

if [ -L "docroot/sites" ]; then
  echo "Removing sites link."
  rm -f docroot/sites
fi

CWD=$(pwd)
DIR=`mktemp -d 2>/dev/null || mktemp -d -t 'drupalcore'`

mkdir -p docroot
cd $DIR
drush make "$CWD/drubone/makefiles/drupal$DRUPAL_VERSION.core.make" -y
rm *.txt

cp -fR . "$CWD/docroot"
cd $CWD

if [ ! -d "sites" ]; then
  mv docroot/sites .
fi

cd $CWD/docroot
rm -fR sites
ln -s ../sites sites

rm -fR $DIR
