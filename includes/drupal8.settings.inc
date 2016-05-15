<?php

// Set drubone specific config.
if (function_exists('_drubone_infrastructure_config')) {
  $config['drubone.settings'] = _drubone_infrastructure_config(
    $settings['instance_settings.settings']['environment'],
    $settings['instance_settings.settings']['project'],
    $settings['instance_settings.settings']['instance'])
  ;
  if (isset($config['drubone.settings']['project']['databases'])) {
    $databases = $config['drubone.settings']['project']['databases'];
  }
  if (isset($config['drubone.settings']['params']['conf']) && is_array($config['drubone.settings']['params']['conf'])) {
    foreach ($config['drubone.settings']['params']['conf'] as $param_name => $param_value) {
      $config[$param_name] = $param_value;
    }
  }
}
