#!/bin/bash
# ============================================================
# Skrypt aktualizacji zBitx - obsługa WiFi
# Uruchom z katalogu ~/sbitx  (lub gdzie masz źródła zbitx)
# ============================================================

set -e  # zatrzymaj przy błędzie

SBITX_DIR="$(pwd)"
echo ">>> Katalog zBitx: $SBITX_DIR"

# ---- 1. Sprawdź wymagane narzędzia -------------------------
echo ""
echo ">>> Sprawdzam zależności..."

missing=""
for pkg in gcc pkg-config libgtk-3-dev libasound2-dev libsqlite3-dev libncurses-dev; do
    if ! dpkg -s "$pkg" &>/dev/null; then
        missing="$missing $pkg"
    fi
done

if [ -n "$missing" ]; then
    echo "Brakuje pakietów: $missing"
    echo "Instaluję..."
    sudo apt-get install -y $missing
fi

# nmcli (NetworkManager) - potrzebny do obsługi WiFi
if ! command -v nmcli &>/dev/null; then
    echo "Instaluję NetworkManager (nmcli)..."
    sudo apt-get install -y network-manager
    sudo systemctl enable NetworkManager
    sudo systemctl start NetworkManager
    echo "UWAGA: Może być potrzebny restart: sudo reboot"
fi

echo "OK - wszystkie zależności spełnione"

# ---- 2. Backup starego pliku --------------------------------
echo ""
echo ">>> Tworzę backup sbitx_gtk.c..."
if [ -f "$SBITX_DIR/sbitx_gtk.c" ]; then
    cp "$SBITX_DIR/sbitx_gtk.c" "$SBITX_DIR/sbitx_gtk.c.bak_$(date +%Y%m%d_%H%M%S)"
    echo "Backup: sbitx_gtk.c.bak_$(date +%Y%m%d_%H%M%S)"
fi

# ---- 3. Skopiuj nowy plik ----------------------------------
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
if [ -f "$SCRIPT_DIR/sbitx_gtk.c" ]; then
    echo ">>> Kopiuję nowy sbitx_gtk.c..."
    cp "$SCRIPT_DIR/sbitx_gtk.c" "$SBITX_DIR/sbitx_gtk.c"
else
    echo "BŁĄD: Nie znaleziono $SCRIPT_DIR/sbitx_gtk.c"
    echo "Umieść sbitx_gtk.c w tym samym katalogu co ten skrypt."
    exit 1
fi

# ---- 4. Kompilacja -----------------------------------------
echo ""
echo ">>> Kompiluję sbitx (może potrwać kilka minut na Pi Zero)..."
cd "$SBITX_DIR"

gcc -O1 -o sbitx \
    vfo.c si570.c sbitx_sound.c fft_filter.c sbitx_gtk.c sbitx_utils.c \
    i2cbb.c si5351v2.c ini.c hamlib.c queue.c modems.c logbook.c \
    modem_cw.c settings_ui.c oled.c hist_disp.c ntputil.c \
    telnet.c macros.c modem_ft8.c remote.c mongoose.c webserver.c sbitx.c \
    ft8_lib/libft8.a \
    -lwiringPi -lasound -lm -lfftw3 -lfftw3f -pthread -lncurses -lsqlite3 \
    $(pkg-config --cflags gtk+-3.0) $(pkg-config --libs gtk+-3.0)

echo ""
echo "============================================"
echo "  Kompilacja zakończona sukcesem!"
echo "  Uruchom: ./sbitx"
echo "============================================"
