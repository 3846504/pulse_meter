#githubからの取得(強制にリモートに合わせる)
sudo git -C /opt/sakulab_ex_module fetch origin master
sudo git -C /opt/sakulab_ex_module reset --hard origin/master

#権限の変更
sudo chmod 755 /opt/sakulab_ex_module/src/start_program.sh
sudo chmod 755 /opt/sakulab_ex_module/bin/chua_mm
sudo chmod 755 /opt/sakulab_ex_module/bin/pulse_meter
sudo chmod 755 /opt/sakulab_ex_module/bin/tp_mm
sudo chmod 755 /opt/sakulab_ex_module/bin/amp_mm

#serviceファイルの有効化
sudo cp /opt/sakulab_ex_module/src/StartMesure.service /etc/systemd/system
sudo systemctl enable StartMesure
sudo systemctl start StartMesure