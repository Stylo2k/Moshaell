diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 1.in 2> /dev/null) 1.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 2.in 2> /dev/null) 2.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 3.in 2> /dev/null) 3.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 4.in 2> /dev/null) 4.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 5.in 2> /dev/null) 5.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 6.in 2> /dev/null) 6.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 7.in 2> /dev/null) 7.out
diff <(valgrind --error-exitcode=111 --leak-check=full ./shell < 8.in 2> /dev/null) 8.out
