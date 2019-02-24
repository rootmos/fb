#!/bin/sh

FB=${1-fb}

trap "rm -rf $FB" EXIT

DISP=:1
WHD=${WHD-800x600x8}
N=0

Xvfb $DISP -screen $N $WHD -fbdir /tmp &
XVFB_PID=$!

sleep 1

x11vnc -localhost -display $DISP 1>&2 &
X11VNC_PID=$!

sleep 1

vncviewer :$N 1>&2 &
VNCVIEWER_PID=$!

ln -s /tmp/Xvfb_screen0 $FB

wait -n $XVFB_PID $X11VNC_PID $VNCVIEWER_PID
kill $XVFB_PID; kill -9 $X11VNC_PID; kill $VNCVIEWER_PID;
