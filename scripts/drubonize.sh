#!/bin/sh

if [ ! -d "drubone" ]; then
  echo "Please launch this command from project root. 'drubone' dir should be located in this dir."
  exit 1
fi

CWD=$(pwd)

if [ ! -D 'sites' ]; then
  cp -fR drubone/templates/sites .
  cd docroot/sites/all
fi

cd ${CWD}
cd docroot/sites/all
echo "Copying drush aliases."
cp -f drush/local.aliases.drushrc.php drush/dev.aliases.drushrc.php

cd ${CWD}
if [ ! -d 'drubone.config' ]; then
  cp -fR drubone/templates/drubone.config .
  cp -fR drubone/templates/git/drubone.config/. drubone.config
  echo "Drubone.config initialized."
fi

cd ${CWD}
if [ ! -D 'drubone.config' ]; then
  mkdir drubone.config
else
  for file in drubone/templates/drubone.config/*; do
    filename="${file##*/}" #basename
    if [ ! -f "drubone.config/${filename}" ]; then
      echo "${filename} copied to drubone.config"
      cp ${file} drubone.config
    fi
  done
fi

cd ${CWD}
cd docroot/sites
rm -f sites.php
ln -s ../drubone/sites.php
