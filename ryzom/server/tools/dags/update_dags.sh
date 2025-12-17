#/bin/sh

DAGS_PATH=/home/data/dagu/dags/

hostname=$(hostname)
Hostname=${hostname^}

cd Ryzom
python create_dags.py $Hostname
cd ..

cd MonitorAIS
python create_dags.py
cd ..

cp *.yaml $DAGS_PATH
cp $Hostname/*.yaml $DAGS_PATH
cp Ryzom/${Hostname}_*.yaml $DAGS_PATH
cp RTZ/*.yaml $DAGS_PATH
cp MonitorAIS/*.yaml $DAGS_PATH
