<?php
/**
 * @file
 * Theme functions for Custom Publishing Options.
 */

/**
 * Return markup for the edit form.
 */
function theme_custom_pub_edit_form($vars) {
  $form = $vars['form'];
  $type = $form['type']['#value'];
  $head = array(
    t('Label'),
    t('Machine Name'),
    t('Node Types'),
    array('data' => '', 'class' => array('close-custom-pub-table')),
  );
  $rows = array(
    array(
      backdrop_render($form['state']),
      $type['type'] . backdrop_render($form['stateh']),
      backdrop_render($form['node_types']),
      backdrop_render_children($form),
    ),
  );
  $title = t('Edit ' . $type['name'] . ' option');
  $tvars = array(
    'header' => $head,
    'rows' => $rows,
    'caption' => $title,
  );
  return theme('table', $tvars);
}
