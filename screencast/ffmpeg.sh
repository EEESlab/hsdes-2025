#!/bin/bash
ffmpeg -i setup_screencast.mp4 -vf "fps=25,scale=1024:-1" setup_screencast.gif
