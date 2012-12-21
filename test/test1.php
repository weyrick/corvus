<?php

function foo($one, $two) {
    if (1) return 5;
    echo $hello;
}

function bar($hey, $two=5, $three) {
}

$two = 2;
$x = foo(1, "$two");
echo $x;

?>
