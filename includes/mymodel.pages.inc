<?php

/*
 * Function to create a list of orders.
 */
function mymodel_orders_list($form, &$form_state) {

  drupal_add_css(drupal_get_path('module', 'mymodel') .'/styles/mymodel.css', 'file');

  $header = array(
    'order_id' => array('data' => t('Order #')),
    'created' => array('data' => t('Date')),
    'owner' => array('data' => t('Owner')),
    'total' => array('data' => t('Total')),
    'operations' => array('data' => t('Operations')),
  );

  $options = array();

  $query = db_select('orders','o');
  $query->fields('o');
  $query->OrderBy('o.oid', 'DESC');

  $result = $query->execute();
  //print "\nQuery\n";
  //_dpq($query); // Display the query.

  $destination = drupal_get_destination();

  foreach($result as $order) {
    $options[] = array(
      'order_id' => $order->oid,
      'created' => format_interval(time() - $order->created, 1) . ' ' . t('ago'),
      'owner' => $order->uid,
      'total' => $order->total,
      'operations' => array('data' => array(
                                        'edit' => array(
                                            '#type' => 'link',
                                            '#title' => t('edit'),
                                            '#href' => "order/$order->oid/edit",
                                            '#options' => array('query' => $destination)
                                            ),
                                        'delete' => array(
                                            '#type' => 'link',
                                            '#title' => t('delete'),
                                            '#href' => "order/$order->oid/delete",
                                            '#options' => array('query' => $destination)
                                            ),
                                        ),
                      ),
    );
  }

  $form['accounts'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No orders available.'),
  );

  return $form;
}


/*
 * Function to create form to add an order.
 */
function mymodel_order($form, &$form_state, $order_id = NULL) {

   drupal_add_js(drupal_get_path('module', 'mymodel') .'/js/mymodel.js', 'file');

   if($order_id) {
    $form_state['storage']['order_id'] = $order_id;
    $query = db_select('order_line','ol');
    $query->fields('ol');
    $query->condition('ol.oid', $order_id);
    $result = $query->execute();

    $lines=1;
    foreach($result as $line) {
      $product = _mymodel_product_detail($line->pid);
      $form_state['input']['product_' . $lines] = $line->pid . ":" . $product->name;
      $form_state['input']['price_' . $lines] = $line->price;
      $form_state['input']['quantity_' . $lines] = $line->quantity;
      $form_state['input']['subtotal_' . $lines] = $line->total;
      $lines++;
    }

    $form_state['storage']['lines'] = $lines;
   } else {
    if(!isset($form_state['storage']['lines']))
      $form_state['storage']['lines'] = 1;
   }

   $form['order_lines_wrapper'] = array(
     '#type' => 'fieldset',
     '#title' => t('Order Lines'),
     '#collapsible' => TRUE,
     '#collapsed' => TRUE,
   );

   $form['order_lines_wrapper']['order_lines'] = array(
     '#type' => 'container',
     '#tree' => TRUE,
     '#prefix' => '<div id="order_lines">',
     '#suffix' => '</div>',
   );

   $header = array (
     'product' => t('Product'),
     'price' => t('Price'),
     'quantity' => t('Quantity'),
     'subtotal' => t('Sub-total'),
   );

   $options = array();
   $total = 0;
   for ($i = 1; $i <= $form_state['storage']['lines']; $i++) {
    $options[$i] =array
    (
      'product' => array(
        'data' => array(
          'product_' . $i => array(
            '#type' => 'textfield',
            '#value' => isset($form_state['input']['product_' . $i])?$form_state['input']['product_' . $i]:'',
            '#name' => 'product_' . $i,
            '#size' => 60,
            '#autocomplete_path' => 'product/autocomplete',
            '#id' => 'product_' . $i,
            '#attributes' => array(
              'onblur' => "product_set_price(this);",
            ),
           )
         ),
      ),
      'price' => array(
        'data' => array(
          'quantity_' . $i => array(
            '#type' => 'textfield',
            '#value' => isset($form_state['input']['price_' . $i])? $form_state['input']['price_' . $i]: '',
            '#name' => 'price_' . $i,
            '#id' => 'price_' . $i,
            '#size' => 20,
           )
         ),
       ),
      'quantity' => array(
        'data' => array(
          'quantity_' . $i => array(
            '#type' => 'textfield',
            '#value' => isset($form_state['input']['quantity_' . $i])? $form_state['input']['quantity_' . $i]: '',
            '#name' => 'quantity_' . $i,
            '#id' => 'quantity_' . $i,
            '#size' => 20,
            '#attributes' => array(
              'onblur' => "product_set_subtotal(this);",
            ),
           )
         ),
       ),
       'subtotal' => array(
        'data' => array(
          'subtotal_' . $i => array(
            '#type' => 'textfield',
            '#value' => isset($form_state['input']['subtotal_' . $i])? $form_state['input']['subtotal_' . $i]: '',
            '#name' => 'subtotal_' . $i,
            '#size' => 20,
            '#id' => 'subtotal_' . $i,
           )
         ),
       ),
    );

    if(isset($form_state['input']['subtotal_' . $i])) {
      $total += $form_state['input']['subtotal_' . $i];
    }
  }


  $options = array_reverse($options);

  $options[] = array('product' => '', 'price' => '', 'quantity' => t('Total') , 'subtotal' => t('$') . round($total,2));

  $form_state['storage']['lines'] = $i;

  $form['order_lines_wrapper']['order_lines']['table'] = array
  (
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No lines found'),
    );

  $form['submit'] = array
    (
      '#type' => 'submit',
      '#value' => t('Save Order'),
    );

  $form['add_more'] = array
    (
      '#type' => 'button',
      '#value' => t('Add new line'),
      '#ajax' => array(
       'callback' => 'mymodel_ajax_add_lines',
       'wrapper' => 'order_lines',
       ),
    );

  return $form;
}

function mymodel_ajax_add_lines($form, &$form_state) {
 return $form['order_lines_wrapper']['order_lines'];
}

/*
 * Function to handle submit of form to add an order.
 */
function mymodel_add_order_submit($form, &$form_state) {
  global $user;

  $order_id = NULL;
  $order = NULL;
  $order_total=0;
  for ($i = 1; $i <= $form_state['storage']['lines']; $i++) {
    $product_item = $form_state['input']['product_' . $i];
    $price = $form_state['input']['price_' . $i];
    $quantity = $form_state['input']['quantity_' . $i];
    if(isset($product_item) && isset($quantity)) {
      // Create order only if there at least one order line valid.
      if(!$order_id) {
        $order = new stdclass();
        $order->created = time();
        $order->uid = $user->uid;
        drupal_write_record('orders', $order);
        $order_id = $order->oid;
      }
      list($pid, $name) = explode(":", $product_item);
      $order_line = new stdclass();
      $order_line->oid = $order_id;
      $order_line->pid = $pid;
      $order_line->price = $price;
      $order_line->quantity = $quantity;
      $order_line->total = $price*$quantity;
      $order_total+= $order_line->total;
      drupal_write_record('order_line', $order_line);
    }
  }

  if($order_id) {
    $order->total = $order_total;
    drupal_write_record('orders', $order, 'oid');
    drupal_set_message('Ordes was created sucessfuly');
  }
}

/*
 * Function to create form to add a product.
 */
function mymodel_add_product($form, &$form_state) {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => '',
    '#required' => TRUE,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => '',
  );

  $form['price'] = array(
    '#type' => 'textfield',
    '#title' => t('Price'),
    '#default_value' => '',
    '#element_validate' => array('element_validate_number'),
  );

  $form['submit'] = array
    (
      '#type' => 'submit',
      '#value' => t('Save Product'),
    );

  return $form;
}

/*
 * Function to handle submit of form to add a product.
 */
function mymodel_add_product_submit($form, &$form_state) {
  $product = new stdclass();
  $product->name = $form_state['values']['name'];
  $product->description = $form_state['values']['description'];
  $product->price = $form_state['values']['price'];

  drupal_write_record('product',$product);
  drupal_set_message(t('Product was created sucessfully'));
}

/**
 * Confirmation page to delete an order.
 */
function mymodel_order_delete($form, &$form_state, $order_id) {

  global $base_url;

  $form_state['storage']['oid'] = $order_id;


  $form['instructions'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="container-instructions">' . t('You cannot undo this action') . '</div>',
  );


  $confirmation_form = confirm_form($form, t('Are you sure you want to delete this order?'), $base_url, t('DELETE ORDER #') . $order_id,t('Yes'), t('No'));

  return $confirmation_form;
}

function mymodel_order_delete_submit($form, &$form_state) {
  // Delete order lines first to avoid broke the relation
  db_delete('order_line')
    ->condition('oid', $form_state['storage']['oid'])
    ->execute();

  // Delete order
  db_delete('orders')
    ->condition('oid', $form_state['storage']['oid'])
    ->execute();

  drupal_set_message(t('Order was deleted successfully'));
  $form_state['redirect']  = drupal_get_destination();
}
