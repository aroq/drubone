<?php

/**
 * @file
 */

function _drubone_infrastructure_get_sites() {
  static $sites = array();
  if (!$sites) {
    foreach (_drubone_config_get(array('infrastructure', 'projects')) as $project_id => $project) {
      if (isset($project['params']['site_dir_name_prefix'])) {
        $project_dir_name = $project['params']['site_dir_name_prefix'];
      }
      else {
        $project_dir_name = $project_id;
      }
      if (isset($project['instances'])) {
        foreach ($project['instances'] as $clone_name => $clone) {
          foreach ($clone['environments'] as $env => $environment) {
            foreach ($environment['sites'] as $group_name => $group) {
              foreach ($group['urls'] as $url) {
                $sites[$url] = $project_dir_name . _drubone_config_get(array('infrastructure', 'site_name_separator'), '.') . $clone_name;
              }
            }
          }
        }
      }
    }
  }

  return $sites;
}

function _drubone_infrastructure_get_projects($project) {
  do {
    $projects[$project] = _drubone_config_get(array('infrastructure', 'projects', $project));
  } while ($project = _drubone_config_get(array('infrastructure', 'projects', $project, 'parent')));

  return array_reverse($projects);
}

function _drubone_infrastructure_get_project_params($project) {
  $params = array();
  foreach (_drubone_infrastructure_get_projects($project) as $project_config) {
    if (isset($project_config['params'])) {
      $params[] = $project_config['params'];
    }
  }
  return _drubone_array_deep_merge($params);
}

function _drubone_infrastructure_site_config($site = NULL) {
  if (!$site) {
    // TODO: use main site if executed with drush.
    $site = current(explode(':', rtrim($_SERVER['HTTP_HOST'], '.')));
    foreach (_drubone_config_get(array('infrastructure', 'projects')) as $project_id => $project) {
      if (isset($project['instances'])) {
        foreach ($project['instances'] as $clone_name => $clone) {
          foreach ($clone['environments'] as $env => $environment) {
            foreach ($environment['sites'] as $group_name => $group) {
              foreach ($group['urls'] as $url) {
                if ($url == $site) {
                  $group['group_name'] = $group_name;
                  return $group;
                }
              }
            }
          }
        }
      }
    }
  }
  return array();
}

function _drubone_infrastructure_server_config($server) {
  return _drubone_config_get(array('infrastructure', 'servers', $server), array());
}

function _drubone_infrastructure_environment_config($env) {
  return _drubone_config_get(array('infrastructure', 'environments', $env), array());
}

function _drubone_infrastructure_project_config($env, $project, $instance) {
  $databases = _drubone_config_get(array('infrastructure', 'databases'));
  $config = _drubone_config_get(array('infrastructure', 'projects', $project, 'instances', $instance, 'environments', $env), array());
  $config['params'] = _drubone_infrastructure_get_project_params($project);
  foreach ($config['databases'] as &$db_group) {
    foreach ($db_group as $key => &$value) {
      if (is_string($value) && isset($databases[$value])) {
        $value = $databases[$value];
      }
    }

    unset($value);
  }

  if (isset($config['params']['url_patterns'])) {
    foreach ($config['params']['url_patterns'] as $key => $value) {
      if (!empty($value['patterns'])) {
        $patterns = implode("\n", $value['patterns']);
        if (_drubone_match_path($_GET['q'], $patterns)) {
          $config['params'] = array_replace_recursive($config['params'], $value['params']);
        }
      }
    }
  }
  return $config;
}

function _drubone_infrastructure_instance_config($project, $instance) {
  return _drubone_config_get(array('infrastructure', 'projects', $project, 'instances', $instance), array());
}

function _drubone_infrastructure_config($env, $project, $instance, $site = NULL) {
  $environment_config = _drubone_infrastructure_environment_config($env);
  $config = array(
    'server'      => _drubone_infrastructure_server_config($environment_config['server']),
    'environment' => $environment_config,
    'project'     => _drubone_infrastructure_project_config($env, $project, $instance),
    'projects'    => _drubone_infrastructure_get_projects($project),
    'instance'    => _drubone_infrastructure_instance_config($project, $instance),
    'site'        => _drubone_infrastructure_site_config($site),
  );

  $params = array();
  foreach ($config as $key => $value) {
    if (isset($value['params'])) {
      $params[] = $value['params'];
    }
  }

  $config['params'] = _drubone_array_deep_merge($params);
  $config['raw'] = _drubone_config();
  return $config;
}

