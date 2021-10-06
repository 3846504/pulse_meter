#!/bin/sh

while true
do
    mode=$(python /opt/sakulab_ex_module/src/main_menu.py)

    if [ $mode -eq "0" ]; then
        echo $mode
        sudo /opt/sakulab_ex_module/bin/pulse_meter
    elif [ $mode -eq "1" ]; then
        echo $mode
        sudo /opt/sakulab_ex_module/bin/chua_mm
    elif [ $mode -eq "2" ]; then
        echo $mode
        sudo /opt/sakulab_ex_module/bin/tp_mm
    elif [ $mode -eq "-1" ]; then
        echo "push shutdown button"
        sudo shutdown -h now
    else
        break
    fi
done
echo "finish program"
