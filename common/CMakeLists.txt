if (MYSQL_FOUND)
    message(STATUS "mysql found, adding db_conn to exact_common library!")
    add_library(exact_common arguments random exp db_conn)
else (MYSQL_FOUND)
    add_library(exact_common arguments exp random)
endif (MYSQL_FOUND)
