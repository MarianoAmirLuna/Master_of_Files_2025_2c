run_masive_worker()
{
	worker/bin/worker "worker/planificacion_1.config" 1 &
	worker/bin/worker "worker/planificacion_1.config" 2 &
	worker/bin/worker "worker/planificacion_1.config" 3 &
	worker/bin/worker "worker/planificacion_1.config" 4 &
	worker/bin/worker "worker/planificacion_1.config" 5 &
	worker/bin/worker "worker/planificacion_1.config" 6 &
	worker/bin/worker "worker/planificacion_1.config" 7 &
	worker/bin/worker "worker/planificacion_1.config" 8 &
}
run_masive_worker