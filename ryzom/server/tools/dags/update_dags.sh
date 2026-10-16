#/bin/sh

DAGS_PATH=/home/data/dagu/dags/

hostname=$(hostname -s | tr '[:upper:]' '[:lower:]')
Hostname=${hostname^}
echo $hostname
cd Ryzom
python create_dags.py $Hostname
cd ..

cd MonitorAIS
python create_dags.py
cd ..

cp *.yaml $DAGS_PATH
if [ -d $Hostname ]; then
	cp $Hostname/*.yaml $DAGS_PATH
fi
cp Ryzom/${Hostname}_*.yaml $DAGS_PATH
cp RTZ/*.yaml $DAGS_PATH
cp MonitorAIS/*.yaml $DAGS_PATH
