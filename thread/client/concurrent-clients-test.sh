#!/bin/bash
./client.out 0.0.0.0 4000 6666 <Dostoyevsky.txt &
./client.out 0.0.0.0 4000 9999 <Dostoyevsky.txt &
./client.out 0.0.0.0 4000 12345 <Dostoyevsky.txt &
./client.out 0.0.0.0 4000 42357 <Dostoyevsky.txt
