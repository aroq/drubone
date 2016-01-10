<?php

/**
 * @file
 * php.config.engine.inc
 *
 * PHP config engine.
 */

function _drubone_php_get_config($name) {
  return _drubone_php_merge_config_files(_drubone_get_config_files($name));
}

function _drubone_php_merge_config_files($includes) {
  $config = array();
  // Deep merge config values.
  foreach ($includes as $filename) {
    if (is_file($filename)) {
      $tempconf = require $filename;
      $config['drubone_included_files'][] = $filename;
      $config = _drubone_array_deep_merge(array($config, $tempconf));
    }
  }
  return $config;
}

