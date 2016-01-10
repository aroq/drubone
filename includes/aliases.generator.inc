<?php

_drubone_load_component('infrastructure');

// Get configuration.
$environment = $params['environment'];
$environments = _drubone_config_get(array('infrastructure', 'environments'));
$servers = _drubone_config_get(array('infrastructure', 'servers'));

$dir_handle = new DirectoryIterator(DRUBONE_DRUPAL_ROOT . '/sites');
while ($dir_handle->valid()) {
  if ($dir_handle->isDir() && !$dir_handle->isDot()) {
    // Check if subdirectory contains a Drupal site.
    if (file_exists($dir_handle->getPathname() . '/settings.php')) {
      $basename = $dir_handle->getBasename();
      $domains[$basename] = array(
        'uri' => $basename,
        'root' => $dir_handle->getPathname(),
      );
    }
  }
  $dir_handle->next();
}

// Function to create aliases for given params.
$drush_aliases = function($domains, $env, $environments, $servers) {
  foreach ($domains as $prefix => $domain) {
    $aliases[$prefix] = array(
      'uri' => $domain['uri'],
      'path-aliases' => array(
      ),
      'root' => DRUBONE_DRUPAL_ROOT,
    );

    if (isset($environments[$env])) {
      $aliases[$prefix] = _drubone_array_deep_merge(array(
        $aliases[$prefix],
        $servers[$environments[$env]['server']]['alias'],
        $environments[$env]['alias'],
      ));
    }
    $aliases[$prefix]['path-aliases']['%files'] = $aliases[$prefix]['root'] . '/sites/' . $domain['uri'] . '/files';
  }

  return $aliases;
};

$aliases = $drush_aliases($domains, $environment, $environments, $servers);

$all = array();
foreach ($aliases as $alias_name => $alias) {
  $all[] = "@$environment.$alias_name";
}
$aliases['all'] = array('site-list' => $all);

// Remove "remote-host" from entries that correspond with the current server.
// This allows us to use the same alias file in all environments.
$ip = gethostbyname(php_uname('n'));
drush_log(dt('Hostname: @hostname', array('@hostname' => $ip)));
foreach ($aliases as $key => &$alias) {
  if (empty($alias['remote-host'])) {
    continue;
  }
  $alias_hostname = gethostbyname($alias['remote-host']);
  drush_log(dt('Alias hostname: @hostname', array('@hostname' => $alias_hostname)));
  if ($alias_hostname === $ip) {
    unset($alias['remote-host']);
  }
}

return $aliases;

