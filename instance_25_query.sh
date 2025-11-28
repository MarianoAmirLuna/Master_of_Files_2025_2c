run_aging()
{
	echo "Run Script $1"
	for i in $(seq 1 25);
	do
		query_control/bin/query_control "query_control/estabilidad_general.config" "$1" 20
	done
}
run_aging AGING_1
run_aging AGING_2
run_aging AGING_3
run_aging AGING_4