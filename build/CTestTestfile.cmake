# CMake generated Testfile for 
# Source directory: /home/alpha/Downloads/kio-main (4)
# Build directory: /home/alpha/Downloads/kio-main (4)/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(kio_hello "/home/alpha/Downloads/kio-main (4)/build/kio" "/home/alpha/Downloads/kio-main (4)/examples/hello.kio")
set_tests_properties(kio_hello PROPERTIES  PASS_REGULAR_EXPRESSION "^42
kio
\$" _BACKTRACE_TRIPLES "/home/alpha/Downloads/kio-main (4)/CMakeLists.txt;167;add_test;/home/alpha/Downloads/kio-main (4)/CMakeLists.txt;0;")
add_test(kio_types "/home/alpha/Downloads/kio-main (4)/build/kio" "/home/alpha/Downloads/kio-main (4)/examples/type_test.kio")
set_tests_properties(kio_types PROPERTIES  _BACKTRACE_TRIPLES "/home/alpha/Downloads/kio-main (4)/CMakeLists.txt;184;add_test;/home/alpha/Downloads/kio-main (4)/CMakeLists.txt;0;")
