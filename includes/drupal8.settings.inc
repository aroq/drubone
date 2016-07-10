<?php

// Init instance settings.
if (!isset($config['instance_settings.settings']['environment'])) {
  $config['instance_settings.settings']['environment'] = 'local';
}

if (!isset($config['instance_settings.settings']['project']) || !isset($config['instance_settings.settings']['instance'])) {
  $site_dir = current(array_slice(explode('/', $site_path), -1));
  $dir_name_parts = preg_split("/[-_,\. ]/", $site_dir);
  $config['instance_settings.settings']['instance'] = array_pop($dir_name_parts);
  $config['instance_settings.settings']['project'] = implode('.', $dir_name_parts);
}

// Set drubone specific config.
if (function_exists('_drubone_infrastructure_config')) {
  $config['drubone.settings'] = _drubone_infrastructure_config(
    $config['instance_settings.settings']['environment'],
    $config['instance_settings.settings']['project'],
    $config['instance_settings.settings']['instance'])
  ;
  if (isset($config['drubone.settings']['project']['databases'])) {
    $databases = $config['instance_settings.settings']['databases'] = $config['drubone.settings']['project']['databases'];
  }
  if (isset($config['drubone.settings']['params']['conf']) && is_array($config['drubone.settings']['params']['conf'])) {
    foreach ($config['drubone.settings']['params']['conf'] as $param_name => $param_value) {
      if (is_array($config[$param_name]) && is_array($param_value)) {
        $config[$param_name] = _drubone_array_deep_merge(array($config[$param_name], $param_value));
      }
      else {
        $config[$param_name] = $param_value;
      }
    }
  }
}
