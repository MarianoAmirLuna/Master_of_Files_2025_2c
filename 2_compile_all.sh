readonly BLUE=34
readonly GREEN=32
readonly RED=31
readonly QUERYCONTROL="QUERYCONTROL"
readonly MASTER="MASTER"
readonly WORKER="WORKER"
readonly STORAGE="STORAGE"
readonly END="END"
readonly COMPILING="COMPILING"

CLEARINPUTS=false

successfull_compilations=()
errors_compilations=()
start_compiling(){
	echo -e "\e["$BLUE"m--- " $COMPILING" $1---\e[0m"
}
end_compiling(){
	echo -e "\e["$GREEN"m--- " $END" $1---\e[0m"
}
error_compiling(){
	echo -e "\e["$RED"m--- " ERROR COMPILATION" $1---\e[0m"
}

compiling(){
	local compile_name="$1"
  	local name_path="$2"
	echo "Name path" $name_path " compile name" $compile_name
	if [[ "$name_path" == "query" ]]; then
		name_path="query_control"
	fi

	start_compiling $compile_name
	cd ./$name_path
	make clean
	make
	
	if [ $? -ne 0 ]; then
        echo "Compilation failed with errors."
		error_compiling $compile_name
		errors_compilations+=("$compile_name")
        # You can add further actions here, like exiting the script or logging the error.
        exit 1
    else
        echo "Compilation "$compile_name" successful."
        successfull_compilations+=("$compile_name")
    fi
	end_compiling $compile_name
	cd ..
	if [ "$CLEARINPUTS" = true ] ; then
		clear
	fi
}

if [ -z "$1" ]
  then
    CLEARINPUTS=true
fi

compiling $QUERYCONTROL "query"
compiling $MASTER "master"
compiling $WORKER "worker"
compiling $STORAGE "storage"

for item in "${successfull_compilations[@]}"; do
  echo "Success compilation: $item"
done

for item in "${errors_compilations[@]}"; do
  echo "Error compilation: $item"
done

current_date_time="`date "+%d-%m-%Y %H:%M:%S"`";
echo $current_date_time;
