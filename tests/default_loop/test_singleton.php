<?php
require __DIR__ . '/../../test-tools.php';
$loop = UVLoop::defaultLoop();
True($loop === UVLoop::defaultLoop(), "UVLoop Singleton");