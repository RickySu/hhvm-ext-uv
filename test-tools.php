<?php
function Equal($expected, $condition, $message) {
  printf("$message %s\n", ($expected == $condition ? 'ok' : 'failed'));
}

function True($condition, $message) {
  Equal(true, $condition, $message);
}
