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

  $form['add'] = array(
    '#markup' => l(t('Add Order'), 'node/add/order'),
  );

  $form['orders'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No orders available.'),
  );

  return $form;
}

/*
 * Function to create a list of products.
 */
function mymodel_products_list($form, &$form_state) {

  drupal_add_css(drupal_get_path('module', 'mymodel') .'/styles/mymodel.css', 'file');

  $header = array(
    'product_id' => array('data' => t('Product #')),
    'name' => array('data' => t('Name')),
    'price' => array('data' => t('Price')),
    'operations' => array('data' => t('Operations')),
  );

  $options = array();

  $query = db_select('product','p');
  $query->fields('p');
  $query->OrderBy('p.pid', 'DESC');

  $result = $query->execute();
  //print "\nQuery\n";
  //_dpq($query); // Display the query.

  $destination = drupal_get_destination();

  foreach($result as $product) {
    $options[] = array(
      'product_id' => $product->pid,
      'name' => $product->name,
      'price' => $product->price,
      'operations' => array('data' => array(
                                        'edit' => array(
                                            '#type' => 'link',
                                            '#title' => t('edit'),
                                            '#href' => "product/$product->pid/edit",
                                            '#options' => array('query' => $destination)
                                            ),
                                        'delete' => array(
                                            '#type' => 'link',
                                            '#title' => t('delete'),
                                            '#href' => "product/$product->pid/delete",
                                            '#options' => array('query' => $destination)
                                            ),
                                        ),
                      ),
    );
  }

  $form['add'] = array(
    '#markup' => l(t('Add Product'), 'node/add/product'),
  );

  $form['orders'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No products available.'),
  );

  return $form;
}

/*
 * Function to create form to add/edit an order.
 */
function mymodel_order($form, &$form_state, $order_id = NULL) {

   drupal_add_js(drupal_get_path('module', 'mymodel') .'/js/mymodel.js', 'file');

   $form['add'] = array(
    '#markup' => l(t('Add Product'), 'node/add/product'),
    );


   if($order_id && !isset($form_state['triggering_element']['#value'])) {
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

   }
   elseif(!isset($form_state['storage']['lines'])) {
      $form_state['storage']['lines'] = 1;
   }

   $form['order_lines_wrapper'] = array(
     '#type' => 'fieldset',
     '#title' => t('Order Lines'),
     '#collapsible' => FALSE,
     '#collapsed' => FALSE,
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

   // Detect if deleted button was press.
   $deleted = 0;
   $lines = $form_state['storage']['lines'];
   if(isset($form_state['triggering_element']['#value']) &&
    $form_state['triggering_element']['#value'] == 'Delete selected') {
    $lines--;
   }

   for ($i = 1; $i <= $lines; $i++) {
    // Remove selected lines, requiere a math due a reserve order
    // to maintain new line at the top
    if(isset($form_state['values']['order_lines']['table']) && $form_state['values']['order_lines']['table'][$form_state['storage']['lines'] - $i - 1]) {
      $deleted++;
      continue;
    }

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

  // Remove lines if were deleted
  $form_state['storage']['lines'] = $i - $deleted;

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

   $form['delete_selected'] = array
    (
      '#type' => 'button',
      '#value' => t('Delete selected'),
      '#ajax' => array(
       'callback' => 'mymodel_ajax_add_lines',
       'wrapper' => 'order_lines',
       ),
    );

  $form['orders'] = array(
    '#markup' => l(t('List of Orders'), 'orders'),
  );

  return $form;
}

function mymodel_ajax_add_lines($form, &$form_state) {
 return $form['order_lines_wrapper']['order_lines'];
}

/*
 * Function to handle submit of form to add an order.
 */
function mymodel_order_submit($form, &$form_state) {
  global $user;

  $order_id = isset($form_state['storage']['order_id'])? $form_state['storage']['order_id']: NULL;
  $order = NULL;
  $order_total=0;

  // Delete previous order lines in edit process
  if($order_id) {
    // Delete order lines first to avoid broke the relation
    db_delete('order_line')
      ->condition('oid', $order_id)
      ->execute();

    $order = new stdclass();
    $order->uid = $user->uid;
    $order->oid = $order_id;
  }

  for ($i = 1; $i <= $form_state['storage']['lines']; $i++) {

    if(!isset($form_state['input']['product_' . $i])) {
      continue;
    }

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
        drupal_set_message('Ordes was created sucessfuly');
        $order_id = $order->oid;
      }

      if(empty($product_item)) {
        continue;
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
    drupal_set_message('Ordes was updated sucessfuly');
  }
}

/*
 * Function to create form to add/edit a product.
 */
function mymodel_product($form, &$form_state, $product_id = NULL) {

  drupal_add_css(drupal_get_path('module', 'mymodel') .'/styles/mymodel.css', 'file');

  $product = new stdclass();
  $product->name = '';
  $product->price ='';
  $product->description = '';

  if($product_id) {
    $form_state['storage']['product_id'] = $product_id;
    $query = db_select('product','p');
    $query->fields('p');
    $query->condition('p.pid', $product_id);
    $product = $query->execute()->fetch();
  }

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $product->name,
    '#required' => TRUE,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $product->description,
  );

  $form['price'] = array(
    '#type' => 'textfield',
    '#title' => t('Price'),
    '#default_value' => $product->price,
    '#element_validate' => array('element_validate_number'),
  );

  $form['submit'] = array
    (
      '#type' => 'submit',
      '#value' => t('Save Product'),
    );

  $form['product_list'] = array(
    '#markup' => l(t('List of products'), 'products'),
  );

  $form['add_order'] = array(
    '#markup' => l(t('Add Order'), 'node/add/order'),
  );

  return $form;
}

/*
 * Function to handle submit of form to add a product.
 */
function mymodel_product_submit($form, &$form_state) {
  $product = new stdclass();
  $product->name = $form_state['values']['name'];
  $product->description = $form_state['values']['description'];
  $product->price = $form_state['values']['price'];

  if(isset($form_state['storage']['product_id'])) {
    $product->pid = $form_state['storage']['product_id'];
    drupal_write_record('product',$product, 'pid');
    drupal_set_message(t('Product was updated sucessfully'));
  }
  else {
    drupal_write_record('product',$product);
    drupal_set_message(t('Product was created sucessfully'));
  }
}

/*
 * Confirmation page to delete a product.
 */
function mymodel_product_delete($form, &$form_state, $product_id) {

  global $base_url;

  $form_state['storage']['pid'] = $product_id;

  // Get product information
  $query = db_select('product','p');
  $query->fields('p');
  $query->condition('p.pid', $product_id);
  $product = $query->execute()->fetch();

  // Verify if product is used in current orders
  $query = db_select('order_line','ol');
  $query->fields('ol', array('oid'));
  $query->condition('ol.pid', $product_id);
  $query->groupBy('ol.oid');
  //_dpq($query);
  $result = $query->execute();

  if($result->rowCount()) {
    $form['orders_related_to_product'] = array(
      '#type' => 'fieldset',
      '#title' => t("Product !product can't be deleted becuase is related with following orders", array('!product' => $product->name)),
    );
    foreach ($result as $order) {
      $form['orders_related_to_product'][$order->oid] = array(
          '#type' => 'link',
          '#title' => 'Order # ' . $order->oid,
          '#href' => 'order/' . $order->oid . '/edit',
          '#prefix' => '<div>',
          '#suffix' => '</div>',
          );
    }

    return $form;
  }

  $form['instructions'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="container-instructions">' . t('You cannot undo this action') . '</div>',
  );


  $confirmation_form = confirm_form($form, t('Are you sure you want to delete this product?'), $base_url, t('DELETE Product: ') . $product->name,t('Yes'), t('No'));

  return $confirmation_form;
}

function mymodel_product_delete_submit($form, &$form_state) {
  // Delete product
  db_delete('product')
    ->condition('pid', $form_state['storage']['pid'])
    ->execute();

  drupal_set_message(t('Product was deleted successfully'));
  $form_state['redirect']  = drupal_get_destination();
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
