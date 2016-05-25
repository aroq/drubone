<?php
/**
 *
 * @file
 * include.settings.inc
 *
 * This settings file should be included from settings.php file.
 * Put all include files related logic here.
 *
 * Everything in this file is executed on DRUPAL_BOOTSTRAP_CONFIGURATION
 * bootstrap phase, so don't put here anything not available on this phase.
 */
$config['instance_settings.settings']['version'] = 2;

_instance_settings_init_params($site_path);

if (isset($config['instance_settings.settings']['databases'])) {
  $databases = $config['instance_settings.settings']['databases'];
}

// Include files from provided array.
foreach (_instance_settings_get_includes($config['instance_settings.settings']['pattern_types'], $config['instance_settings.settings']['token_values'], $config['instance_settings.settings']['file_extensions']) as $filename) {
  if (file_exists($filename)) {
    $tempconf = $config;
    require $filename;
    $config['instance_settings.settings']['included_files'][] = $filename;
    foreach ($config as $key => $value) {
      if (is_array($value) && isset($tempconf[$key])) {
        $config[$key] = _instance_settings_include_array_merge_deep_no_duplicate_values(array($tempconf[$key], $config[$key]));
      }
    }
    $config = array_merge($tempconf, $config);
  }
}

// To keep compatibility with settings files from previous versions.
if (isset($config['instance_settings.settings']['compatibility_mode']) && $config['instance_settings.settings']['compatibility_mode'] == 1) {
  foreach ($config['instance_settings.settings'] as $key => $setting) {
    $config['instance_settings_' . $key] = $setting;
  }
}

/**
 * Initialize Instance settings.
 */
function _instance_settings_init_params($site_path) {
  global $config, $databases;

  if (!isset($config['instance_settings.settings']['env_get_method'])) {
    $config['instance_settings.settings']['env_get_method'] = 'env_file';
  }

  $default_values = array();
  $default_values['included_files'] = array();
  $default_values['file_extensions'] = array('.settings.inc');
  $default_values['site_dir_full'] = DRUPAL_ROOT . '/' . $site_path;
  $default_values['site_dir'] = current(array_slice(explode('/', $site_path), -1));
  $default_values['environment'] = _instance_settings_get_environment($default_values['site_dir_full'], $config['instance_settings.settings']['env_get_method']);

  // TODO: Allow override here.
  $default_values['settings_dirs'] = (array(
    DRUBONE_ROOT . '/code/common/config/settings',
    DRUBONE_ROOT . '/code/all/config/settings',
    DRUPAL_ROOT . '/sites/all/config/settings',
  ));

  // Name of the site's dir should be in format:
  // {PROJECT}{SEPARATOR}{SUBPROJECT}{SEPARATOR}{INSTANCE} to pick up these settings
  // automatically, where {SEPARATOR} could be "." or "_".
  // {SUBPROJECT} is optional.
  $default_values['project_name_full'] = $default_values['site_dir'];
  $project_name = isset($config['instance_settings.settings']['project_name_full']) ? $config['instance_settings.settings']['project_name_full'] : $default_values['site_dir'];

  $dir_name_parts = preg_split("/[-_,\. ]/", $project_name);

  $default_values['project_names'] = array();
  foreach ($dir_name_parts as $name_part) {
    foreach ($default_values['project_names'] as $pname) {
      $default_values['project_names'][] = $pname . '.' . $name_part;
    }
    $default_values['project_names'][] = $name_part;
  }

  $default_values['instance'] = array_pop($dir_name_parts);

  $default_values['project'] = $dir_name_parts ? implode('.', $dir_name_parts) : $default_values['instance'];
  $default_values['projects'] = array();
  $default_values['projects_path_variants'] = array('code', 'projects');

  // TODO: Remove patterns if no tokens present.
  $default_values['pattern_types'] = array(
    // Default patterns to find settings files based on params defined.
    'default' => array(
      'all',
      '!environment_group',
      '!environment',
      '!instance',
      '!instance/!environment_group',
      '!instance/!environment',
      '!instance.!environment_group',
      '!instance.!environment',
      '!environment_group.!instance',
      '!environment.!instance',
      '!environment_group/!instance',
      '!environment/!instance',
      'subsite.!subsite',
    ),
    'final' => array(
      'final.all',
      'final.!environment',
      'final.subsite.!subsite',
    ),
  );

  if (isset($config['instance_settings.settings'])) {
    $config['instance_settings.settings'] = _instance_settings_include_array_merge_deep_no_duplicate_values(array($default_values, $config['instance_settings.settings']));
  }
  else {
    $config['instance_settings.settings'] = $default_values;
  }


  // Set drubone specific config.
  if (function_exists('_drubone_infrastructure_config')) {
    if (file_exists(DRUBONE_DIR_FULL . '/includes/drupal8.settings.inc')) {
      include DRUBONE_DIR_FULL . '/includes/drupal8.settings.inc';
    }
    if (isset($config['infrastructure']['site']['group_name']) && !isset($config['instance_settings.settings']['subsite'])) {
      $config['instance_settings.settings']['subsite'] = $config['infrastructure']['site']['group_name'];
    }
  }

  // TODO: unset pattern_types that not have token values.
  // Params for patterns.
  $config['instance_settings.settings']['token_values'] = array(
    '!environment' => $config['instance_settings.settings']['environment'],
    '!environment_group' => isset($config['instance_settings_environment_group']) ? $config['instance_settings_environment_group'] : $config['instance_settings.settings']['environment'],
    '!instance' => $config['instance_settings.settings']['instance'],
    '!project' => $config['instance_settings.settings']['project'],
    '!subsite' => isset($config['instance_settings.settings']['subsite']) ? $config['instance_settings.settings']['subsite'] : '',
  );

  // If no projects are defined, try to find project by project name.
  // TODO: support projects hierarchy in project name.
  if (!$config['instance_settings.settings']['projects']) {
    $drupal_root = rtrim(DRUPAL_ROOT, '/');
    $root = substr($drupal_root, 0, strrpos($drupal_root, '/'));

    foreach ($config['instance_settings.settings']['projects_path_variants'] as $projects_path_variant) {
      foreach ($config['instance_settings.settings']['project_names'] as $project_name) {
        if (is_dir($root . '/'. $projects_path_variant . '/' . $project_name)) {
          $config['instance_settings.settings']['projects'][$project_name] = array(
            'params' => array(
              'path' => $projects_path_variant . '/' . $project_name,
            ),
          );
        }
      }
    }
  }

  // Set settings dirs.
  _instance_settings_settings_dirs($config['instance_settings.settings']['settings_dirs']);
  _instance_settings_settings_dirs(array_values(_instance_settings_get_project_settings_dirs($config['instance_settings.settings']['projects'])));
  _instance_settings_settings_dirs(array($config['instance_settings.settings']['site_dir_full']));
}

/**
 * Get project settings directories.
 *
 * @param array $projects
 *  Projects list to process.
 *
 * @return array
 *   Project settings dir list.
 */
function _instance_settings_get_project_settings_dirs($projects) {
  $project_dirs = array();
  $drupal_root = rtrim(DRUPAL_ROOT, '/');
  $root = substr($drupal_root, 0, strrpos($drupal_root, '/'));

  foreach ($projects as $project_name => $project) {
    if (isset($project['params']['path'])) {
      $project_dirs[$project_name] = $root . '/' . $project['params']['path'] . '/config/settings';
    }
  }

  return $project_dirs;
}

/**
 * Get settings files to include.
 *
 * @param array $pattern_types
 *   File patterns to search.
 *
 * @param $token_values
 *   Token values to use in patterns.
 *
 * @param $instance_settings_file_extensions
 *   Settings files extensions.
 *
 * @return array
 *   File paths to include.
 */
function _instance_settings_get_includes($pattern_types, $token_values, $instance_settings_file_extensions) {
  $settings_sources = _instance_settings_get_settings_sources($pattern_types);

  static $includes = array();
  if ($includes) {
    return $includes;
  }
  foreach ($settings_sources as &$dir) {
    if (isset($dir['pattern_types'])) {
      $dir['patterns'] = array();
      foreach ($dir['pattern_types'] as $pattern_type) {
        $dir['patterns'] += $pattern_types[$pattern_type];
      }
      foreach ($dir['patterns'] as $pattern) {
        if ($pattern) {
          foreach ($instance_settings_file_extensions as $instance_settings_extension) {
            $file = $dir['dir'] . '/' . strtr($pattern, $token_values) . $instance_settings_extension;
            if (!in_array($file, $includes) && is_file($file)) {
              $includes[] = $file;
            }
          }
        }
      }
    }
  }
  return $includes;
}

/**
 * Add settings dir to list.
 *
 * @param array $dirs
 *   Settings dir list to add.
 *
 * @return array
 *   Settings dir list.
 */
function _instance_settings_settings_dirs($dirs = array()) {
  static $settings_dirs = array();
  if ($dirs) {
    $settings_dirs = array_merge($settings_dirs, $dirs);
  }
  return $settings_dirs;
}

/**
 * Add params to settings dirs list.
 *
 * @param array $pattern_types
 *   Pattern types list.
 *
 * @return array
 *   Processed settings dirs list.
 */
function _instance_settings_get_settings_sources($pattern_types) {
  static $settings_sources = array();

  if ($settings_sources) {
    return $settings_sources;
  }

  $settings_dirs = _instance_settings_settings_dirs();

  foreach (array_keys($pattern_types) as $pattern_type) {
    foreach ($settings_dirs as $settings_dir) {
      $settings_sources[] = array(
        'dir' => $settings_dir,
        'pattern_types' => array($pattern_type),
      );
    }
  }

  return $settings_sources;
}

/**
 * Deep merge array values without dublicating array keys.
 *
 * @param array $arrays
 *   Arrays to merge.
 *
 * @return array
 *   Merged array.
 */
function _instance_settings_include_array_merge_deep_no_duplicate_values($arrays) {
  // TODO: Add override options (e.g. "disable override", "remove key", etc).
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
        $result[$key] = _instance_settings_include_array_merge_deep_no_duplicate_values(array($result[$key], $value));
      }
      // Otherwise, use the latter value, overriding any previous value.
      else {
        $result[$key] = $value;
      }
    }
  }

  return $result;
}

/**
 * Get current environment based on passed method.
 *
 * @param string $site_dir
 *   Site config directory.
 *
 * @param string $method
 *   Method to used to determine current environment.
 *
 * @return string
 *   Current environment.
 */
function _instance_settings_get_environment($site_dir, $method = 'env_file') {
  $ENVIRONMENT = 'local';
  if ($method == 'env_file') {
    // This is only settings file ignored by Git.
    $file = $site_dir . '/' . "env.settings.php";
    if (is_file($file)) {
      // $ENVIRONMENT variable should be set inside "env.settings.php" file.
      include $file;
    }
  }
  elseif ($method = 'acquia') {
    if (isset($_ENV['AH_SITE_ENVIRONMENT'])) {
      $ENVIRONMENT = $_ENV['AH_SITE_ENVIRONMENT'];
    }
  }
  return $ENVIRONMENT;
}

