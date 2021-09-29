#!/bin/sh
#
# A sample script demonstrating how the feature of getting the path of the app's
# enclosing folder as first argument can be used
#
# A Platypus app created from this script will always move itself to the user's
# home folder when launched, regardless of its location.
#

mv "$1" ~
