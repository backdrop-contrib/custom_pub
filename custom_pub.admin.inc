<?php
/**
 * @file
 * Admin functions.
 */

/**
 * Callback function for Custom Publishing Options menu item.
 */
function custom_pub_admin() {
  $output = '<h3>Custom Publishing Options</h3>';

  $output .= '<p>Creating/editing/deleting options requires clearing the Backdrop cache before you can use them. It is advised to make as many edits here that you need, and then clear the cache to propogate the updates to the node system.</p>';

  $table = '';

  //add the js for the admin page
  backdrop_add_js(backdrop_get_path('module', 'custom_pub') . '/custom_pub.admin.js');

  //get the current custom publishing types
  $types = config_get('custom_pub.settings', 'types');

  foreach ($types as $type) {
    // Build table rows for the
    $form_row = array();

    $types_of_nodes = '';

    if (isset($type['node_types']) && is_array($type['node_types'])) {
      $types_of_nodes = implode(', ', $type['node_types']);
    }

    $row = array($type['name'], $type['type'], $types_of_nodes, array('data' => '', 'class' => array('custom_pub-option-edit-cell')));

    $form_row[] = array(
      'data' => backdrop_get_form('custom_pub_edit_' . $type['type'], $type),
      'colspan' => 4,
      'class' => array('custom_pub-form-edit'),
    );

    $rows[] = array('data' => $row, 'class' => array('custom_pub-option-row'));
    $rows[] = array('data' => $form_row, 'class' => array('custom_pub-form-edit'));
  }

  if (!empty($rows)) {
    $vars = array(
      'header' => array(
        t('Label'),
        t('Machine Name'),
        t('Node Types'),
        array('data' => '', 'class' => array('custom_pub-head-edit')),
      ),
      'rows' => $rows
    );

    $table = theme('table', $vars);
  }

  return $output . $table;
}

/**
 * Form callback to add a publishing option.
 * @param $form
 * @param $form_state
 * @return mixed
 */
function custom_pub_add($form, &$form_state) {
  $form['state_fs'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add a Publishing Option'),
  );

  $form['state_fs']['state'] = array(
    '#title' => t('Publishing label'),
    '#type' => 'textfield',
    '#description' => t('The label for your custom publishing option. This is the text that will be displayed on the node add/edit form.'),
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['state_fs']['state_machine'] = array(
    '#type' => 'textfield',
    '#title' => t('Option name'),
    '#description' => t('The machine-readable name of this publishing option. This text will be used for constructing the database table column name. This name must contain only lowercase letters, numbers, and underscores. This name must be unique.'),
    '#required' => TRUE,
  );

  $form['state_fs']['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Available on'),
    '#description' => t('The Node Types this option is available on.'),
    '#options' => node_type_get_names(),
  );

  $form['state_fs']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 * Validate handler
 * @param $form
 * @param $form_state
 */
function custom_pub_add_validate($form, &$form_state) {
  $types = config_get('custom_pub.settings', 'types');

  $type = array();
  $type['type'] = trim($form_state['values']['state_machine']);
  $type['name'] = trim($form_state['values']['state']);

  $node = backdrop_get_schema('node');

  if (isset($types[$type['type']])) {
    form_set_error('state_machine', t('The machine-readable name %type is already taken.', array('%type' => $type->type)));
  }
  if (!preg_match('!^[a-z0-9_]+$!', $type['type'])) {
    form_set_error('state_machine', t('The machine-readable name must contain only lowercase letters, numbers, and underscores.'));
  }
  // 'theme' conflicts with theme_node_form().
  // '0' is invalid, since elsewhere we check it using empty().
  if (in_array($type['type'], array_keys($node['fields'])) && !isset($types[$type['type']])) {
    form_set_error('state_machine', t("Invalid machine-readable name. That name is already taken by a database column. Please enter a name other than %invalid.", array('%invalid' => $type['type'])));
  }
  foreach ($types as $check) {
    if ($type['name'] == $check['name']) {
      form_set_error('state', t("Invalid Label. That Publishing Label is already taken. Please enter a label other than %invalid.", array('%invalid' => $type['name'])));
    }
  }
}

/**
 * Submit handler
 * @param $form
 * @param $form_state
 */
function custom_pub_add_submit($form, &$form_state) {
  $types = config_get('custom_pub.settings', 'types');
  $node_types = _node_types_build();

  $type = array();
  $type[trim($form_state['values']['state_machine'])]['type'] = trim($form_state['values']['state_machine']);
  $type[trim($form_state['values']['state_machine'])]['name'] = trim($form_state['values']['state']);

  foreach ($form_state['values']['node_types'] as $node_type => $value) {
    if (!empty($value)) {
      $type[trim($form_state['values']['state_machine'])]['node_types'][$node_type] = $node_types->types[$node_type]->name;
    }
  }

  $spec = array(
    'description' => 'Custom publishing option ' . t(trim($form_state['values']['state'])),
    'type' => 'int',
    'not null' => TRUE,
    'default' => 0
  );
  try {
    db_add_field('node', trim($form_state['values']['state_machine']), $spec);
    $success = TRUE;
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('custom_pub', $e, NULL, WATCHDOG_ERROR);
  }
  if ($success) {
    cache_clear_all('*', 'cache', TRUE);
    config_set('custom_pub.settings', 'types', array_merge($types, $type));
    backdrop_set_message(t('Publishing option %option created.', array('%option' => $form_state['values']['state'])));
    $form_state['redirect'] = 'admin/structure/custom_pub';
  }
  else {
    backdrop_set_message(t('There was an error creating your Publishing option.'), 'error');
  }
}

/**
 * Form callback function for the edit form.
 */
function custom_pub_edit($form, &$form_state, $type) {
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  $form['state'] = array(
    '#title' => t('Publishing label'),
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#size' => 100,
    '#default_value' => $type['name'],
  );
  $form['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Available on'),
    '#description' => t('The Node Types this option is available on.'),
    '#options' => node_type_get_names(),
    '#default_value' => isset($type['node_types']) ? array_keys($type['node_types']) : array(),
  );
  $form['stateh'] = array(
    '#type' => 'hidden',
    '#value' => $type['type'],
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if (!isset($type['default'])) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#attributes' => array('id' => 'edit-delete'),
    );
  }
  $form['#theme'][] = 'custom_pub_edit_form';
  $form['#validate'][] = 'custom_pub_edit_validate';
  $form['#submit'][] = 'custom_pub_edit_submit';

  return $form;
}

/**
 * Validate handler
 * @param $form
 * @param $form_state
 */
function custom_pub_edit_validate($form, &$form_state) {
  $types = config_get('custom_pub.settings', 'types');
  $type = $form_state['values']['type'];
  $name = trim($form_state['values']['state']);

  foreach ($types as $check) {
    if ($type['name'] == $check['name'] && $type['type'] != $check['type']) {
      form_set_error('state', t("Invalid Label. The publishing label is already taken. Please enter a label other than %invalid.", array('%invalid' => $type['name'])));
    }
  }
}

/**
 * Form submit function for the edit form.
 */
function custom_pub_edit_submit($form, &$form_state) {
  $node_types = _node_types_build();
  $type = $form_state['values']['type'];
  $types = config_get('custom_pub.settings', 'types');

  if ($form_state['values']['op'] == t('Delete')) {
    $op = t('Deleted');
    unset($types[$type['type']]);
    try {
      db_drop_field('node', $type['type']);
      $success = TRUE;
    }
    catch (Exception $e) {
      $success = FALSE;
      watchdog_exception('custom_pub', $e, NULL, WATCHDOG_ERROR);
    }
  }
  else {
    $op = t('Edited');
    unset($types[$type['type']]['node_types']);
    foreach ($form_state['values']['node_types'] as $node_type => $value) {
      if (!empty($value)) {
        $types[$type['type']]['node_types'][$node_type] = $node_types->types[$node_type]->name;
      }
    }
    $type['name'] = trim($form_state['values']['state']);
    $types[$type['type']]['name'] = $type['name'];
    $success = TRUE;
  }
  if ($success) {
    cache_clear_all('*', 'cache', TRUE);
    config_set('custom_pub.settings', 'types', $types);
    backdrop_set_message(t('!op the publishing option %name.', array('!op' => $op, '%name' => trim($form_state['values']['state']))));
  }
  else {
    backdrop_set_message(t('There was an error trying to !op the publishing option. Please Try Again', array('!op' => $form_state['values']['op'])), 'error');
  }
}
