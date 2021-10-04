#!/bin/sh

while true
do
    mode=$(python /opt/sakulab_ex_unit/main_menu.py)

    if [ $mode -eq "0" ]; then
        echo $mode
        sudo /opt/sakulab_ex_unit/pulse_meter
    elif [ $mode -eq "1" ]; then
        echo $mode
        sudo /opt/sakulab_ex_unit/chua_mm
    elif [ $mode -eq "2" ]; then
        echo $mode
        sudo /opt/sakulab_ex_unit/tp_mm
    elif [ $mode -eq "-1" ]; then
        echo "push shutdown button"
        sudo shutdown -h now
    else
        break
    fi
done
echo "finish program"
