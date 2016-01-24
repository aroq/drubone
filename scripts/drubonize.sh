#!/bin/sh

if [ ! -d "drubone" ]; then
  echo "Please launch this command from project root. 'drubone' dir should be located in this dir."
  exit 1
fi

CWD=$(pwd)

if [ ! -f "sites/all/drush/local/aliases.drushrc.php" ]; then
  echo "Copying drush aliases."
  cp -fR drubone/templates/sites/* sites
fi

cd ${CWD}
cd docroot/sites/all
echo "Copying drush aliases for dev."
cp -f drush/local.aliases.drushrc.php drush/dev.aliases.drushrc.php

cd ${CWD}
if [ ! -d 'drubone.config' ]; then
fi

cd ${CWD}
if [ ! -d "drubone.config" ]; then
  echo "Copying Drubone.config."
  cp -fR drubone/templates/drubone.config .
fi

cd ${CWD}
cd docroot/sites
rm -f sites.php
ln -s ../drubone/sites.php
