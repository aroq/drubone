<?php

$config['environment.servers'] = array(
  'local' => array(
    'alias' => array(
      'path-aliases' => array(
        '%dump-dir' => '/tmp',
      ),
    ),
  ),
  'main' => array(
    'alias' => array(
      'remote-host' => 'PLACEHOLDER: REMOTE.HOST.HERE',
      'remote-user' => 'PLACEHOLDER: REMOTE.USER.HERE',
      'path-aliases' => array(
        '%dump-dir' => '/tmp',
      ),
    ),
  ),
);

$config['environments'] = array(
  'local' => array(
    'server' => 'local',
  ),
  'dev' => array(
    'server' => 'main',
    'alias' => array(
      'root' => 'PLACEHOLDER: PATH.TO.DOCROOT',
    ),
  ),
);

$config['projects'] = array(
  'PLACEHOLDER: PROJECT.NAME' => array(
    'instances' => array(
      'com' => array(
        'urls' => array(
          'local' => array(
            'main' => array(
              'url' => 'PLACEHOLDER: SITE.URL.LOCAL',
              'params' => array(),
            ),
          ),
          'dev' => array(
            'main' => array(
              'url' => 'PLACEHOLDER: SITE.URL.DEV',
              'params' => array(),
            ),
          ),
        ),
      ),
    ),
  ),
);
