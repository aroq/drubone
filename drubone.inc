<?php

/**
 * @file
 * init.inc
 *
 * Init Drubone.
 */

function _drubone_get_drupal_root() {
  return function_exists('drush_main') ? drush_get_context('DRUSH_SELECTED_DRUPAL_ROOT') : DRUPAL_ROOT;
}

function _drubone_get_config_engines() {
  static $engines = array();
  if (!$engines) {
    $config = &_drubone_config();
    $engines = isset($config['config_engines']) ? $config['config_engines'] : array('php');
    foreach ($engines as $engine) {
      _drubone_include($engine . '.config.engine.inc');
    }
  }
  return $engines;
}

function _drubone_load_component($name = '', $params = array(), $include_once = TRUE) {
  $config = &_drubone_config();
  if (!$config) {
    $filepath = DRUBONE_CONFIG_DIR_FULL . '/' . 'config.inc';
    if (is_file($filepath)) {
      $config = include $filepath;
    }
  }

  static $loaded = array();
  if ($include_once && isset($loaded[$name])) {
    $result = $loaded[$name];
  }
  else {
    foreach (_drubone_get_config_engines() as $config_engine) {
      if (function_exists($function = '_drubone_' . $config_engine . '_get_config')) {
        $config = _drubone_array_deep_merge(array($config, $function($name)));
      }
    }
    $result = _drubone_include($name . '.inc', $params);

    $loaded[$name] = $result;
  }
  return $result;
}

function _drubone_get_config_files($name, $file_suffix = '.config.inc', $directories = array(), $file_patterns = array()) {
  $includes = array();
  if (!$directories) {
    $directories = array(DRUBONE_DIR_FULL . '/config', DRUBONE_CONFIG_DIR_FULL);
  }
  if (!$file_patterns) {
    $file_patterns = array('local.');
  }
  foreach ($directories as $directory) {
    $files[] = $directory . '/' . $name . $file_suffix;
    foreach($file_patterns as $pattern) {
      $files[] = $directory . '/' . $pattern . $name . $file_suffix;
    }
  }
  foreach ($files as $file) {
    if (is_file($file)) {
      $includes[] = $file;
    }
  }
  return $includes;
}

function &_drubone_config($config = array()) {
  static $conf = array();
  if ($config) {
    $conf = _drubone_array_deep_merge($conf, $config);
  }
  return $conf;
}

function _drubone_config_get($arg, $default = NULL) {
  $config = _drubone_config();
  if (is_string($arg)) {
    return isset($config[$arg]) ? $config[$arg] : $default;
  }
  elseif (is_array($arg)) {
    $key_exists = NULL;
    $value = _drubone_array_get_nested_value($config, $arg, $key_exists);
    return $key_exists ? $value : $default;
  }
}

function drubone_get(&$var, $default = null) {
  return isset($var) ? $var : $default;
}

function _drubone_include($filename, $params = array()) {
  $filepath = DRUBONE_DIR_FULL . '/includes/' . $filename;
  if (is_file($filepath)) {
    return include $filepath;
  }
}

function _drubone_array_deep_merge($arrays) {
  $result = array();

  foreach ($arrays as $array) {
    foreach ($array as $key => $value) {
      // Renumber integer keys as array_merge_recursive() does. Note that PHP
      // automatically converts array keys that are integer strings (e.g., '1')
      // to integers.
      if (is_integer($key)) {
        if (is_array($value)) {
          $result[] = $value;
        }
        elseif (!in_array($value, $result)) {
          $result[] = $value;
        }
      }
      // Recurse when both values are arrays.
      elseif (isset($result[$key]) && is_array($result[$key]) && is_array($value)) {
        $result[$key] = _drubone_array_deep_merge(array($result[$key], $value));
      }
      // Otherwise, use the latter value, overriding any previous value.
      else {
        $result[$key] = $value;
      }
    }
  }

  return $result;
}

function &_drubone_array_get_nested_value(array &$array, array $parents, &$key_exists = NULL) {
  $ref = &$array;
  foreach ($parents as $parent) {
    if (is_array($ref) && array_key_exists($parent, $ref)) {
      $ref = &$ref[$parent];
    }
    else {
      $key_exists = FALSE;
      $null = NULL;
      return $null;
    }
  }
  $key_exists = TRUE;
  return $ref;
}

function _drubone_match_path($path, $patterns) {
  static $regexps = array();

  if (!isset($regexps[$patterns])) {
    // Convert path settings to a regular expression.
    // Therefore replace newlines with a logical or, /* with asterisks and the
    // <front> with the frontpage.
    $to_replace = array(
      // Newlines.
      '/(\r\n?|\n)/',
      // Asterisks.
      '/\\\\\*/',
    );
    $replacements = array(
      '|',
      '.*',
    );
    $patterns_quoted = preg_quote($patterns, '/');
    $regexps[$patterns] = '/^(' . preg_replace($to_replace, $replacements, $patterns_quoted) . ')$/';
  }
  return (bool) preg_match($regexps[$patterns], $path);
}

_drubone_load_component('drubone');
