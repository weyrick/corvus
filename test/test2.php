<?php

namespace test_other {

class foo {
    const DEF1 = 5;
}

}

namespace test_main {

use \test_other\foo;

// nodiag: from use
class bar extends foo {

  function fun1() {
      // nodiag: from use
      $a = foo::DEF1;
      return self::DEF1;
  }

}

$a = new bar();
$a->fun1();

}


?>
