diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/1.in 2> /dev/null) tests/1.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/2.in 2> /dev/null) tests/2.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/3.in 2> /dev/null) tests/3.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/4.in 2> /dev/null) tests/4.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/5.in 2> /dev/null) tests/5.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/6.in 2> /dev/null) tests/6.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/7.in 2> /dev/null) tests/7.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < tests/8.in 2> /dev/null) tests/8.out
