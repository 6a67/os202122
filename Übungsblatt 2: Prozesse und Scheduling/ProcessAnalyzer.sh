#!/bin/bash



# ZombieSpawner im Hintergrund starten
./ZombieSpawner &

# 1 Sekunde warten
sleep 1

exec 3>&1 > process.log
proc="$(ps -e -o 'stat= pid= command=')"
# Zustand: Z (Überschrift ausgeben, gefolgt von Name, PID und Zustand aller Zombie Prozesse)

echo "Zustand: Zombie"
echo "$proc" | grep '^Z'


# Zustand: I (Überschrift ausgeben, gefolgt von Name, PID und Zustand aller Idle Prozesse)

echo "Zustand: Sleep"
echo "$proc" | grep '^S'
echo "$proc" | grep '^D'


echo "Zustand: Idle"
echo "$proc" | grep '^I'

echo "Zustand: Gestoppt"
echo "$proc" | grep '^T'
echo "$proc" | grep '^t'

echo "Zustand: Tot"
echo "$proc" | grep '^X'


# auf Beendigung von ZombieSpawner warten

wait

# Ende
echo "ProcessAnalyzer has finished its work!" >&3
