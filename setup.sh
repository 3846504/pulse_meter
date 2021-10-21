#/boot/config.txtの変更
sudo cp /opt/sakulab_ex_module/boot/config.txt /boot/config.txt

#スリープの無効化
echo @xset s off >> /etc/xdg/lxsession/LXDE/autostart
echo @xset s noblank >> /etc/xdg/lxsession/LXDE/autostart
echo @xset -dpms >> /etc/xdg/lxsession/LXDE/autostart

echo [SeatDefaults] >> /etc/lightdm/lightdm.conf
echo xserver-command=X -s 0 -dpms >> /etc/lightdm/lightdm.conf

#権限の変更
sudo chmod 755 /opt/sakulab_ex_module/src/start_program.sh
sudo chmod 755 /opt/sakulab_ex_module/bin/chua_mm
sudo chmod 755 /opt/sakulab_ex_module/bin/pulse_meter
sudo chmod 755 /opt/sakulab_ex_module/bin/tp_mm
sudo chmod 755 /opt/sakulab_ex_module/bin/amp_mm

#serviceファイルの有効化
sudo cp /opt/sakulab_ex_module/src/StartMesure.service /etc/systemd/system
sudo systemctl enable StartMesure

#必要コマンドの入手
sudo apt-get update
sudo apt-get install fbi

#シャットダウン
sudo shutdown -h now
