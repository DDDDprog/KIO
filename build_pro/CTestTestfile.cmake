# CMake generated Testfile for 
# Source directory: /home/kali/Downloads/KIO-main/KIO
# Build directory: /home/kali/Downloads/KIO-main/KIO/build_pro
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(kio_hello "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/hello.kio")
set_tests_properties(kio_hello PROPERTIES  PASS_REGULAR_EXPRESSION "^42\\nkio\\n\$" _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;200;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_equality "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/equality_test.kio")
set_tests_properties(kio_equality PROPERTIES  PASS_REGULAR_EXPRESSION "^1\\n1\\n\$" _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;204;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_comparisons "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/comparisons_test.kio")
set_tests_properties(kio_comparisons PROPERTIES  PASS_REGULAR_EXPRESSION "^1\\n1\\n1\\n1\\n\$" _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;207;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_modulo "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/modulo_test.kio")
set_tests_properties(kio_modulo PROPERTIES  PASS_REGULAR_EXPRESSION "^1\\n\$" _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;210;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_arithmetic "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/arithmetic_test.kio")
set_tests_properties(kio_arithmetic PROPERTIES  PASS_REGULAR_EXPRESSION "^1\\n1\\n1\\n1\\n\$" _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;213;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_parallel "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/parallel_test.kio")
set_tests_properties(kio_parallel PROPERTIES  _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;218;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_performance "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/performance_test.kio")
set_tests_properties(kio_performance PROPERTIES  _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;222;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_modules "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/module_test.kio")
set_tests_properties(kio_modules PROPERTIES  _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;226;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
add_test(kio_types "/home/kali/Downloads/KIO-main/KIO/build_pro/kio" "/home/kali/Downloads/KIO-main/KIO/examples/type_test.kio")
set_tests_properties(kio_types PROPERTIES  _BACKTRACE_TRIPLES "/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;230;add_test;/home/kali/Downloads/KIO-main/KIO/CMakeLists.txt;0;")
