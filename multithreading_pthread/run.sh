#!/bin/bash

make
echo "Starting server in background..."
./server &              # מריץ את השרת ברקע ואז אפשר להריץ לקוח במקביל
sleep 1                 # ממתין שנייה לוודא שהשרת מוכן
echo "Opening client..."
./client                # מריץ לקוח אחד
