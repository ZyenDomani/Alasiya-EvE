#/bin/bash
MYSQL_USER=allan
MYSQL_PASS=none
DB_NAME=new

SQL_STRING="SHOW TABLES;"
# Pipe the SQL into mysql
TABLES=$(echo $SQL_STRING | mysql -u$MYSQL_USER -p$MYSQL_PASS $DB_NAME -Bs)

mkdir tables

#echo $TABLES

for i in ${TABLES} ; do
    echo "Dumping $i"
    mysqldump --add-drop-table -u $MYSQL_USER -p$MYSQL_PASS $DB_NAME $i > "tables/${i}.sql"
done
