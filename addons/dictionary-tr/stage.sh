#!/bin/bash
# Fetches this add-on's dictionary files into the staging folder (see tools/stage-dictionary.sh)
exec "$(dirname "$0")/../../tools/stage-dictionary.sh" "$(dirname "$0")" "$2"
