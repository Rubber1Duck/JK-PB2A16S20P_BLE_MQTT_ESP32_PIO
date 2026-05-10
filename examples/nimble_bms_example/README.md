# NimBLE BMS Example

Dieses Beispiel zeigt einen einfachen ESP32-NimBLE-Client fuer ein BMS mit:

- Service UUID: `0xFFE0`
- Write Characteristic UUID: `0xFFE1`
- Notification Characteristic UUID: `0xFFE1`
- Write Handle: `0x03`
- Notify Handle: `0x05`

Wichtig: Auf Client-Seite wird in NimBLE normalerweise ueber UUIDs gearbeitet. Die Handles sind auf dem Geraet relevant, aber fuer das Beispiel nicht direkt noetig, solange es dieselbe Characteristic ist.

## Was das Beispiel macht

- scannt nach Geraeten mit Service `0xFFE0`
- verbindet sich mit dem gefundenen BMS
- abonniert Notifications auf `0xFFE1`
- sendet zwei Beispiel-Kommandos per Write
- druckt eingehende Notification-Daten auf die serielle Konsole

## Anpassung

Im Quellcode kannst du direkt anpassen:

- `kUseDeviceNameFilter` aktivieren, wenn nur ein bestimmter Advertiser verbunden werden soll
- `kTargetDeviceName` setzen, falls dein BMS einen festen Namen sendet
- `kCmdDeviceInfo` und `kCmdCellData` an dein Protokoll anpassen, falls deine Befehle anders sind

## Einbau in PlatformIO

Das Beispiel ist als separates Beispiel unter `examples/nimble_bms_example/` abgelegt. Du kannst es entweder:

- als Referenz fuer deinen bestehenden Code nutzen
- oder die Logik in dein Hauptprojekt uebernehmen

Wenn du moechtest, kann ich dir als naechstes auch noch eine Version bauen, die direkt in deine bestehende `src/ble_client.cpp` Struktur passt.
