#!/bin/bash

echo "Seleccione el tipo de prueba a realizar:"
echo "1. Planificacion"
echo "2. Race Condition"
echo "3. Particiones Fijas"
echo "4. Particiones Dinamicas"
echo "5. Fibonacci Sequence"
echo "6. Stress"

read -p "Ingrese el numero de la prueba: " prueba

case $prueba in
    1)
        echo "Prueba Planificacion"
        cp "./pruebas/1_prueba_plani/cpu.config" "./cpu/cpu.config"
        cp "./pruebas/1_prueba_plani/kernel.config" "./kernel/kernel.config"
        cp "./pruebas/1_prueba_plani/memoria.config" "./memoria/memoria.config"
        cp "./pruebas/1_prueba_plani/filesystem.config" "./filesystem/filesystem.config"
        rm -f /home/utnso/mount_dir/bitmap.dat
        rm -f /home/utnso/mount_dir/bloques.dat
        rm -f /home/utnso/mount_dir/files/*
        ;;
    2)
        echo "Prueba Race Condition"
        cp "./pruebas/2_prueba_race_condition/cpu.config" "./cpu/cpu.config"
        cp "./pruebas/2_prueba_race_condition/kernel.config" "./kernel/kernel.config"
        cp "./pruebas/2_prueba_race_condition/memoria.config" "./memoria/memoria.config"
        cp "./pruebas/2_prueba_race_condition/filesystem.config" "./filesystem/filesystem.config"
        rm -f /home/utnso/mount_dir/bitmap.dat
        rm -f /home/utnso/mount_dir/bloques.dat
        rm -f /home/utnso/mount_dir/files/*
        ;;
    3)
        echo "Prueba Particiones Fijas"
        cp "./pruebas/3_parts_fijas/cpu.config" "./cpu/cpu.config"
        cp "./pruebas/3_parts_fijas/kernel.config" "./kernel/kernel.config"
        cp "./pruebas/3_parts_fijas/memoria.config" "./memoria/memoria.config"
        cp "./pruebas/3_parts_fijas/filesystem.config" "./filesystem/filesystem.config"
        rm -f /home/utnso/mount_dir/bitmap.dat
        rm -f /home/utnso/mount_dir/bloques.dat
        rm -f /home/utnso/mount_dir/files/*
        ;;
    4)
        echo "Prueba Particiones Dinamicas"
        cp "./pruebas/4_parts_dinamicas/cpu.config" "./cpu/cpu.config"
        cp "./pruebas/4_parts_dinamicas/kernel.config" "./kernel/kernel.config"
        cp "./pruebas/4_parts_dinamicas/memoria.config" "./memoria/memoria.config"
        cp "./pruebas/4_parts_dinamicas/filesystem.config" "./filesystem/filesystem.config"
        rm -f /home/utnso/mount_dir/bitmap.dat
        rm -f /home/utnso/mount_dir/bloques.dat
        rm -f /home/utnso/mount_dir/files/*
        ;;
    5)
        echo "Prueba FS - Fibonacci Sequence"
        cp "./pruebas/5_fibonacci_sequence/cpu.config" "./cpu/cpu.config"
        cp "./pruebas/5_fibonacci_sequence/kernel.config" "./kernel/kernel.config"
        cp "./pruebas/5_fibonacci_sequence/memoria.config" "./memoria/memoria.config"
        cp "./pruebas/5_fibonacci_sequence/filesystem.config" "./filesystem/filesystem.config"
        rm -f /home/utnso/mount_dir/bitmap.dat
        rm -f /home/utnso/mount_dir/bloques.dat
        rm -f /home/utnso/mount_dir/files/*
        ;;
    6)
        echo "Prueba De Stress"
        cp "./pruebas/6_stress/cpu.config" "./cpu/cpu.config"
        cp "./pruebas/6_stress/kernel.config" "./kernel/kernel.config"
        cp "./pruebas/6_stress/memoria.config" "./memoria/memoria.config"
        cp "./pruebas/6_stress/filesystem.config" "./filesystem/filesystem.config"
        rm -f /home/utnso/mount_dir/bitmap.dat
        rm -f /home/utnso/mount_dir/bloques.dat
        rm -f /home/utnso/mount_dir/files/*
        ;;

    *)
        echo "Opcion incorrecta"
        ;;
esac

exit 0