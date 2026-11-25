# Dot Globe Radio

Something fun I tried to get back to C-Programming, now with AI. Internet radio on a dot-matrix globe. Click a city marker to tune in. Click it again to cycle through Channels. India dominated but you can make it however you like. 

## Run

After `make install`, launch **Dot Globe Radio** from your application menu, or run:

```sh
dotradio
```

(`make install` puts the binary in `~/.local/bin`; make sure it is on your `PATH`.)

Audio plays through `libmpv` if installed, otherwise `ffplay` (from FFmpeg). Install one of them.

## Controls

| Input | Action |
| --- | --- |
| Drag | Rotate the globe |
| Click a marker | Select the city and play its station |
| Click the selected marker again | Next station in that city |
| **S** or the *Radio Stations* button | Open/close the station sidebar |
| **R** | Reload the station list |
| **Q** / **Esc** | Close the sidebar, or quit |

### Sidebar

- **Remove a station:** click the red `x` on its row, then **Yes**.
- **Add a station:** click **+ Add Channel**, then:
  1. Type or paste the stream URL (**Ctrl+V**, **Shift+Insert**, right-click, or middle-click), then press **Enter** or **Continue**.
  2. Choose the city for its marker.
  3. Click **Confirm**.

If a station fails to play, the terminal asks whether to remove it (`y`/`n`).

## Stations

Stations are stored in a CSV file, one per line:

```csv
city,latitude,longitude,station name,stream URL
London,51.5074,-0.1278,BBC World Service,https://stream.live.vc.bbcmedia.co.uk/bbc_world_service
```

Rows with the same city share one marker. Blank lines and `#` comments are ignored.
The list is loaded from the first of: `$DOTRADIO_STATIONS`, `./stations.csv`, `~/.config/dotradio/stations.csv`, then the installed copy.

## Uninstall

```sh
make uninstall
```

## Credits and third-party material

- **Land mask (`landmask.h`)** — rasterised from [Natural Earth](https://www.naturalearthdata.com/) 110m land polygons (public domain).
- **Radio streams (`stations.csv`)** — the URLs are publicly listed streams; the audio and station names/trademarks belong to their broadcasters, including BBC, All India Radio (Prasar Bharati), Radio France, Deutschlandfunk, NPO, ORF, RAI, Sveriges Radio, SRG SSR, CBC, KEXP, NTS, Radio Paradise, WFMU, WNYC, WKCR, WWOZ, FluxFM, Resonance FM, RÚV, Red FM, Radio Mirchi, Big FM, Radio City, Hello FM, Suryan FM and Shonan Beach FM. Streams may change or be withdrawn at any time.
- **Audio playback** — uses [mpv](https://mpv.io/) (`libmpv`, loaded at runtime) or [FFmpeg](https://ffmpeg.org/)'s `ffplay`. Neither is bundled or redistributed.
- **City list (`cities.h`)** — names with approximate public geographic coordinates.
- **Display** — X11 and OpenGL/GLX via system libraries. The on-screen text uses a hand-drawn 5x7 bitmap font.
