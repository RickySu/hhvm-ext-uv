<?php
require __DIR__ . '/../test-tools.php';
True(is_integer(UVUtil::version()), "UVUtil::version");
True(is_string(UVUtil::versionString()), "UVUtil::versionString");
