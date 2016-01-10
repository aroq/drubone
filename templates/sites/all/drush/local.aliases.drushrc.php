<?php

/**
 * @file
 * Environment drush aliases.
 */

$aliases = _drubone_load_component('aliases.generator', array('environment' => array_shift(explode('.', basename(__FILE__)))), FALSE);
