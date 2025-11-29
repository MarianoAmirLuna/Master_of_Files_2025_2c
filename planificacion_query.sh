run_query()
{
	echo "Run Script $1"
	query_control/bin/query_control "query_control/planificacion.config" "$1" $2
}
run_query FIFO_1 4
run_query FIFO_2 3
run_query FIFO_3 5
run_query FIFO_4 1
