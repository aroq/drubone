#!/bin/bash

DIR_NAME=${1:-drubone} # drubone by default
DRUPAL_VERSION=${2:-7} # 7 by default
GIT_ENABLED=${3:-false} # false by default

set -x
mkdir ${DIR_NAME}
cd ${DIR_NAME}
if [ "$GIT_ENABLED" = true ] ; then
  git init
fi
if [ "$GIT_ENABLED" = true ] ; then
  touch README
  git add README
  git commit -m "Initial commit"
  git subtree add --squash --prefix=drubone git@github.com:aroq/drubone.git master
else  
  git clone git@github.com:aroq/drubone.git
fi
drubone/scripts/drupalcore.sh $DRUPAL_VERSION
if [ "$GIT_ENABLED" = true ] ; then
  git add -A
  git commit -m "Added drupal core"
fi
cd docroot
cd ..
drubone/scripts/drubonize.sh
if [ "$GIT_ENABLED" = true ] ; then
  git add -A
  git commit -m "Drubonized"
fi
cd drubone
composer require symfony/config
composer require symfony/yaml
