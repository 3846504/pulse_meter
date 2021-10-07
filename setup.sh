#/boot/config.txtの変更
sudo cp /opt/sakulab_ex_module/boot/config.txt /boot/config.txt

#スリープの無効化
@xset s off >> /etc/xdg/lxsession/LXDE/autostart
@xset s noblank >> /etc/xdg/lxsession/LXDE/autostart
@xset -dpms >> /etc/xdg/lxsession/LXDE/autostart

[SeatDefaults] >> /etc/lightdm/lightdm.conf
xserver-command=X -s 0 -dpms >> /etc/lightdm/lightdm.conf

#権限の変更
sudo chmod 755 /opt/sakulab_ex_module/src/start_program.sh
sudo chmod 755 /opt/sakulab_ex_module/bin/chua_mm
sudo chmod 755 /opt/sakulab_ex_module/bin/pulse_meter
sudo chmod 755 /opt/sakulab_ex_module/bin/tp_mm

#serviceファイルの有効化
sudo cp /opt/sakulab_ex_module/src/StartMesure.service /etc/systemd/system
sudo systemctl enable StartMesure

#再起動
sudo reboot