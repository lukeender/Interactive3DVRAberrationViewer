#!/bin/bash
COUNTER=0
while [ $COUNTER -lt 10 ]; do
        ./ffmpeg  -protocol_whitelist file,udp,rtp -i foo.sdp  -vcodec copy -acodec copy -f flv "rtmp://104.168.29.115/hls/test"
done