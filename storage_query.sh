run_query()
{
	echo "Run Script $1"
	query_control/bin/query_control "query_control/storage.config" "$1" $2 &
	sleep 0.25
}
run_query STORAGE_1 0
run_query STORAGE_2 2
run_query STORAGE_3 4
run_query STORAGE_4 6
