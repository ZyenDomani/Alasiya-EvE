
#!/bin/bash

host="localhost"    #Database Host
port="3306"     #Database Port
user="eve"     #Database Username
pass="onlyme"     #Database Password
database="EVE_Crucible"   #Database name



tables=(`find tables/. -name "*.sql"`)
for i in ${!tables[*]}
do
    echo " >> Installing.. ($(($i+1))/${#tables[@]})  ${tables[$i]}"
    mysql -h ${host} --user=${user} --port=${port} --password=${pass} ${database} < ${tables[$i]}
done