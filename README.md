# Wayfire kbdd plugin: per-window keyboard layout for Wayfire

The _wayfire-kbdd-plugin_ can be used to automatically change the keyboard layout
on a per-window basis.

## Usage

For automatic start enable the plugin in Wayfire Config Manager or add the word _kbdd_ to the _core.plugins_ list.

The information about current keyboard layout can be exported to a file for use by external layout indicator.

## Build and install

```
meson build
ninja -C build
sudo ninja -C build install
```
This will install to `/usr/local/`. To install in `/usr`  you can use  the option: `meson build --prefix=/usr`.

## Layout indicator in Waybar

The plugin exposes `/tmp/layout.json` file, with content like
```
{"id": 0, "name": "Polish", "code": "pl"}
```

You can use this data and implement own keyboar layout indicator.

Here is an example for Waybar.

You need to create a helper script `~/bin/kbd-layout.sh` that waits for updates in that file
```
#!/usr/bin/env sh

FILE="/tmp/layout.json"
DEFAULT_LAYOUT="pl"

function get_code() {
    CODE=$(test -f "${FILE}" \
               && grep -oP '"code":\s*"\K[^"]+' "${FILE}" \
                   || echo "${DEFAULT_LAYOUT}")

    case "${CODE}" in
        us) echo "🇺🇸" ;;
        de) echo "🇩🇪" ;;
        ro) echo "🇷🇴" ;;
        fr) echo "🇫🇷" ;;
        it) echo "🇮🇹" ;;
        gb|uk|en) echo "🇬🇧" ;;
        ua) echo "🇺🇦" ;;
        pl) echo "🇵🇱" ;;
        *)  echo "${CODE}" ;;
    esac
}

get_code

inotifywait -q -m -e close_write "$(dirname "${FILE}")" |
    while read path action file
    do
        if [ "${file}" = "$(basename "${FILE}")" ]
        then
            get_code
        fi
    done
```

This script just waits for updates, and wakes up for a short moment of time only when layout is changed.

Then, in `~/.config/waybar/config` we can configure a custom module, that waits for updates from upe script and shows what that script returned – flags:
```
    "custom/kbdlayout": {
        "exec": "~/bin/kbd-layout.sh",
        "exec-on-event": false,
        "interval": 0,
        "return-type": "text"
    }
```
