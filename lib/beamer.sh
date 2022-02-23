# beamer.sh: sets Linux display for doing a presentation, 
#  for bumblebee configured on a laptop that has the HDMI
#  plugged on the NVidia board.
#
# Bruno Levy, Wed Jan 24 08:45:45 CET 2018
# Updated     Sat 02 Feb 2019 06:53:02 PM CET (automatic detection of outputs)
#
# Usage: 
#    beamer.sh widthxheight
#    (default is 1024x768)

# Test whether intel-virtual-output is running and start it.
ivo_process=`ps axu |grep 'intel-virtual-output' |egrep -v 'grep'`
if [ -z "$ivo_process" ]; then
   intel-virtual-output
   # Sleep for 3 seconds in order to let enough time for the
   # system to detect the beamer etc...
   sleep 3
fi

# Get the first two connected outputs
outputs=$(xrandr |grep ' connected' | awk '{ print $1}')
echo outputs=$outputs
read -r output1 output2 <<< $(echo $outputs)
echo output1=$output1
echo output2=$output2

# If this does not work, use the hardcoded outputs here,
#  and change according to your configuration.
#output1=eDP1
#output2=VIRTUAL4


wxh=$1

# Default is 1024x768 if unspecified
if [ -z "$wxh" ]; then
  wxh=1024x768
fi

# Back to laptop screen only.
# Note: sometimes impossible to re-switch-on after.
if [ "$wxh" = "off" ]; then
   xrandr --output $output1 --auto --output $output2 --off
   exit
fi

# Use automatically detected resolution.
# Note: sometimes needs to be done twice.
if [ "$wxh" == "auto" ]; then
   xrandr --output $output1 --auto --output $output2 --auto --same-as $output1
   exit
fi

# Note: I think that the following command should have done
# the job, but it does not work. 
#    xrandr --output eDP1 --size 1024x768 --output VIRTUAL1 --size 1024x768 --same-as eDP1
# My guess: --size is not implemented with VIRTUAL devices.
# Thus I try to find a --mode that fits my needs in the list of supported modes.

# Mode names on the primary output are simply wxh (at least on
#  my configuration...)
output1_mode=$wxh

echo Using mode for $output1: $output1_mode

# Mode names on the virtual output are like: VIRTUAL1.ID-wxh
# Try to find one in the list that matches what we want.
output2_mode=`xrandr |grep $output2\\\. |grep $wxh |awk '{print $1}'`
# There can be several modes, take the first one.
output2_mode=`echo $output2_mode |awk '{print $1}'` 


if [ -z "$output2_mode" ]; then
  echo ---- Did not find any mode with $wxh resolution for $output2 output
  exit
fi

echo Using mode for $output2: $output2_mode

# Showtime !
echo Command: xrandr --output $output1 --mode $output1_mode --output $output2 --mode $output2_mode --same-as $output1
xrandr --output $output1 --mode $output1_mode --output $output2 --mode $output2_mode --same-as $output1

