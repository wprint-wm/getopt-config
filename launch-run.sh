#!/bin/sh

# set XDG_RUNRIME_DIR and mkdir

# set COLOR of wprint

export COLOR1=00ffff

# set key binding

export WP_KEY=$(mktemp)

trap "rm -f '$WP_KEY'" EXIT

cat << EOF > $WP_KEY

# set keybindings here

EOF

# run
# wprint

