#!/usr/bin/env sh
set -eu

launcher_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$launcher_dir/.." && pwd)
player_dir=${1:-"$project_dir/Resources/Player"}

if [ ! -d "$player_dir" ]; then
  echo "Player directory does not exist: $player_dir" >&2
  exit 1
fi

if [ -n "${ASEPRITE_BIN:-}" ]; then
  aseprite_bin=$ASEPRITE_BIN
elif command -v aseprite >/dev/null 2>&1; then
  aseprite_bin=$(command -v aseprite)
else
  aseprite_bin="$HOME/.local/share/Steam/steamapps/common/Aseprite/aseprite"
fi

if [ ! -x "$aseprite_bin" ]; then
  echo "Aseprite executable was not found." >&2
  echo "Set ASEPRITE_BIN to its full path and run this launcher again." >&2
  exit 1
fi

exec "$aseprite_bin" \
  --noinapp \
  --script-param "root=$player_dir" \
  --script "$launcher_dir/open_player_animations.lua"
