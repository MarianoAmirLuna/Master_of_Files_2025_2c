run_query()
{
	echo "Run Script $1"
	query_control/bin/query_control "query_control/storage.config" "$1" $2 &
}
run_query ESCRITURA_ARCHIVO_COMMITED 1
run_query FILE_EXISTENTE 1
run_query LECTURA_FUERA_DEL_LIMITE 1
run_query TAG_EXISTENTE 1
