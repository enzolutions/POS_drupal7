function product_set_price(input) {
  var product_item = jQuery(input).val().split(':');

  jQuery.get( "/product/detail/" + product_item[0] , function( data ) {
    var lid = jQuery(input).attr('id').split('product_');
    jQuery('#price_' + lid[1]).val(data.price);
  });
}

function product_set_subtotal(input) {
  var quantity = jQuery(input).val();
  var lid = jQuery(input).attr('id').split('quantity_');
  var price = jQuery('#price_' + lid[1]).val();
  var subtotal = new Number(price*quantity);
  jQuery('#subtotal_' + lid[1]).val(subtotal.toFixed(2));

}
