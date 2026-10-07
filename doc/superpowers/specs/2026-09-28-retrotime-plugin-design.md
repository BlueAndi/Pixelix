# RetroTime Plugin — Design

Datum: 2026-09-28
Status: Entwurf zur Durchsicht

## 1. Ziel und Abgrenzung

Das `RetroTimePlugin` zeigt auf einem 64x64-Pixel-Display Datum und Uhrzeit und
verbindet beides mit Animationen im Stil klassischer Arcade-Spiele. Wechselt die
Uhrzeit, wird der Wechsel als Spielszene inszeniert: Bei *Space Invaders* fährt
ein Raumschiff unter die betroffene Ziffer, schiesst sie ab, sie zerfällt und die
neue Ziffer erscheint.

Die erste Animation ist Space Invaders. Weitere Animationen — als naechste Mario,
der die Ziffer anspringt — sollen ohne Eingriff in Layout- oder Ziffernlogik
hinzukommen koennen. Das ist die eigentliche Anforderung an den Entwurf: Nicht
eine Animation, sondern ein tragfaehiger Unterbau fuer viele.

### Nicht Bestandteil

- Sekundenanzeige. Die Uhrzeit wird als `HH:MM` dargestellt.
- Andere Displaygroessen als 64x64. Die Struktur sieht Layout-Varianten vor,
  implementiert wird zunaechst nur 64x64 plus ein schlichter Generic-Fallback.
- Die Mario-Animation selbst. Sie wird hier nur als Nachweis herangezogen, dass
  der Step-Vorrat traegt, und spaeter als eigenes Vorhaben umgesetzt.
- Konfigurierbare Farben, Geschwindigkeiten und 12-Stunden-Format.

## 2. Getroffene Entscheidungen

| Frage | Entscheidung |
|---|---|
| Sekunden anzeigen? | Nein. `HH:MM` gross, dazu eine eigenstaendige Leerlauf-Show zwischen den Minutenwechseln |
| Ziffern-Rendering | Eigene Ziffern-Sprites im Arcade-Look, 1 bpp im Flash |
| Animationsauswahl | Konfigurierbar ueber Topic, zusaetzlich `"random"` |
| Nicht-64x64-Displays | Struktur wie `lib/Views` (Generic + 64x64), weitere Layouts spaeter |
| Entwicklungsumgebung | Neue native 64x64-Umgebung fuer die visuelle Iteration |
| Konfigurationsumfang | `animation`, `dateFormat`, `timeZone` |
| Architektur | Partikel-/Sprite-Engine als Unterbau, Animationen als Daten (Scripts) |
| Unit-Tests | Verbindlich fuer die grafikfreie Logik |

## 3. Architektur

Die Animationen sind **Daten**, nicht Code: Jede Animation besteht aus einem
Satz Sprites, einem Satz Actors und zwei Scripts. Ausgefuehrt werden sie von
einer kleinen Engine.

Das bekannte Risiko dieses Ansatzes ist, dass der Step-Vorrat nur gegen die
erste Animation entworfen wird und bei der zweiten nicht passt. Gegenmassnahme:
Der Vorrat wird gegen Space Invaders **und** Mario entworfen; Abschnitt 6 weist
die Abbildung beider Animationen auf denselben Vorrat nach.

### 3.1 Ablage

```
lib/RetroGfx/                       Engine, plugin-unabhaengig
  library.json
  src/Sprite.h/.cpp                 1-bpp Bitmap + Groesse, Zeichnen mit Farbe, X-Flip
  src/SpriteAnim.h/.cpp             Frame-Sequenz mit Dauer je Frame
  src/Actor.h/.cpp                  Fixkomma-Position, Verhalten, SpriteAnim, Sichtbarkeit
  src/ParticleSystem.h/.cpp         Pool fester Groesse, Emitter
  src/Easing.h                      linear / easeOut / Wurfparabel
  src/Script.h                      Step-Datentyp und Step-Vorrat
  src/ScriptRunner.h/.cpp           fuehrt ein Script gegen eine Stage aus
  src/Stage.h                       Actors, Partikel, Ziel-Schnittstelle, Register
  src/ITargetField.h                Schnittstelle der Engine zum Ziffernfeld

lib/RetroTimePlugin/
  library.json
  pixelix.json
  src/RetroTimePlugin.h/.cpp        PluginWithConfig: Zeit, Konfiguration, Topics
  src/internal/View.h               Layout-Auswahl ueber LAYOUT_TYPE
  src/internal/RetroTimeViewGeneric.h/.cpp
  src/internal/RetroTimeView64x64.h/.cpp
  src/internal/DigitField.h/.cpp    4 Ziffern + Doppelpunkt, Zustand je Ziffer
  src/internal/DigitSprites.h/.cpp  10 Ziffern-Sprites
  src/internal/AnimationRegistry.h/.cpp
  src/internal/anim/SpaceInvaders.h/.cpp
  web/RetroTimePlugin.html
  web/RetroTimePlugin.jpg

test/test_RetroGfx/                 Unit-Tests der grafikfreien Logik
```

`RetroGfx` ist eine eigene Bibliothek, weil sie ohne das Plugin baubar und
testbar ist und ein spaeteres Spiele-Plugin sie nutzen kann. Sie wird ueber die
`library.json` des Plugins gezogen und belastet damit nur Builds, die das Plugin
enthalten.

Die Layout-Auswahl folgt dem Muster aus `lib/Views`: Template-Spezialisierung
ueber `LAYOUT_TYPE` aus `lib/Views/src/Layouts.h`. Die View-Varianten liegen im
Plugin, weil sie plugin-spezifisch sind und nicht von anderen Plugins genutzt
werden.

### 3.2 Schichten

```
RetroTimePlugin        Zeit beschaffen, Konfiguration, Topics
   |
RetroTimeView64x64     Layout, Zustandsautomat, besitzt Stage und DigitField
   |          \
DigitField     Stage   Ziffern als Ziel      Actors + Partikel + Register
   |             |
DigitSprites   ScriptRunner  <-- Script (Daten aus lib/RetroTimePlugin/src/internal/anim/)
```

## 4. Layout 64x64

```
y  0.. 6   Datum, TomThumb, horizontal zentriert (TextWidget mit Alignment)
y  7..11   Abstand
y 12..31   Uhrzeit: 4 Ziffern a 12x20 plus Doppelpunkt
           x:  2..13   16..27   [30..33]   36..47   50..61
y 32..47   Flugband: Schuesse, Sprungbogen, Leerlauf-Gegner
y 48..63   Spielflaeche: Figur patrouilliert, Bodenlinie bei y=63
```

Der Doppelpunkt blinkt im Sekundentakt. Da keine Sekunden dargestellt werden,
ist er das einzige Lebenszeichen unterhalb einer Minute.

Ziffern-Sprites: 10 Stueck, je 20 Zeilen zu 2 Byte = 400 Byte Flash. Das Datum
nutzt den vorhandenen TomThumb-Font aus `lib/Fonts`.

Der Partikel-Pool ist fest dimensioniert: 128 Partikel zu 12 Byte, rund 1,5 kB
RAM. Eine zerfallende Ziffer hat etwa 90 gesetzte Pixel, es bleibt Reserve fuer
Funken.

Der Generic-Fallback zeigt Datum und Uhrzeit ohne Animation mit den vorhandenen
Widgets. Er existiert, damit das Plugin auf jeder Displaygroesse uebersetzt und
laeuft, und dient als Andockpunkt fuer spaetere Layout-Varianten.

## 5. Engine-Datenmodell

### Sprite

1-bpp-Bitmap mit Breite und Hoehe, im Flash abgelegt. Gezeichnet wird mit einer
zur Laufzeit gewaehlten Farbe, optional horizontal gespiegelt. Ein Sprite kennt
keine Position.

### SpriteAnim

Eine Folge von Sprites mit Dauer je Frame, optional in Schleife. Beispiel: Der
Gegner hat zwei Frames zu je 400 ms.

### Actor

Eine bewegliche Figur mit Position in Fixkomma (1/16 Pixel). Fixkomma ist
notwendig, weil der Update-Task adaptiv zwischen 20 ms und 200 ms laeuft
(`DisplayMgr::UPDATE_TASK_PERIOD_MIN/MAX`) und langsame Bewegungen bei
Ganzzahl-Position ruckeln wuerden.

Jeder Actor traegt ein *Verhalten*, das bei jedem Tick fortgeschrieben wird:

| Verhalten | Beschreibung | abgeschlossen wenn |
|---|---|---|
| `NONE` | steht | sofort |
| `PATROL_X(x0, x1, speed)` | faehrt zwischen x0 und x1 hin und her, spiegelt das Sprite an den Wendepunkten | nie |
| `MOVE_TO_X(x, speed, easing)` | faehrt horizontal zu x | x erreicht |
| `BALLISTIC(zx, zy, speed, arc)` | bewegt sich zum Zielpunkt; `arc=0` gerade, `arc>0` Wurfparabel mit `arc` Pixel Scheitelhoehe ueber der Verbindungslinie | Zielpunkt erreicht |
| `FALL(bodenhoehe)` | faellt mit Schwerkraft bis zur Bodenhoehe | gelandet |

### ParticleSystem

Pool fester Groesse. Ein Partikel besteht aus Position und Geschwindigkeit in
Fixkomma, Farbe und Restlebensdauer; es unterliegt der Schwerkraft.

Emitter:

- `explodeSprite(sprite, x, y, style)` — erzeugt aus **jedem gesetzten Pixel des
  Sprites** ein Partikel mit Zufallsimpuls. Damit zerfaellt eine Ziffer exakt in
  ihre eigene Form, ohne dass eine eigene Explosionsgrafik noetig ist. Reicht der
  Pool nicht, werden Pixel ausgelassen, statt zu ueberlaufen.
- `burst(x, y, count, style)` — Funken, etwa fuer das Muendungsfeuer.

Styles legen Farbverlauf, Streuung und Lebensdauer fest: `BLAST` (Explosion,
weiss nach rot), `BRICK` (Splitter, braun), `SPARK` (kurze helle Funken).

### ITargetField und DigitField

Die Engine kennt Ziffern nicht. Sie kennt ein *Zielfeld* mit nummerierten
Zielen:

```
class ITargetField
{
    uint8_t getTargetCount() const;
    bool    getTargetBounds(uint8_t index, Rect& bounds) const;
    void    explodeTarget(uint8_t index, ParticleStyle style);
    void    commitTarget(uint8_t index);
    void    spawnTarget(uint8_t index, SpawnStyle style);
    bool    isTargetBusy(uint8_t index) const;
};
```

`DigitField` im Plugin implementiert das. Es haelt vier Ziffern und den
Doppelpunkt, kennt die Geometrie aus Abschnitt 4 und fuehrt je Ziffer den
Zustand:

```
STABLE  --explodeTarget()-->  EXPLODING  --(Zerfall fertig)-->  EMPTY
EMPTY   --commitTarget()-->   EMPTY (neuer Wert uebernommen)
EMPTY   --spawnTarget()-->    SPAWNING   --(Einblendung fertig)-->  STABLE
```

Spawn-Styles: `FLASH_IN` (aufblitzen und einblenden) und `POP_UP` (von unten
hochschieben, fuer Mario gedacht).

### Stage

Die Buehne haelt Actors, das ParticleSystem und einen Zeiger auf das Zielfeld.
Dazu ein Register `target` — der Index des Ziels, das das aktuelle Script
bearbeitet. Dadurch bleiben Scripts allgemein formuliert (*bewege dich zum
Ziel*) statt auf eine feste Ziffer verdrahtet.

## 6. Script-Vorrat

### 6.1 Grundentscheidung: keine Verzweigungen

Damit aus dem Step-Vorrat keine halbgare Programmiersprache wird, kennt er weder
Bedingungen noch Spruenge. Jede Animation liefert genau zwei Scripts:

- **Idle-Script** — laeuft in Schleife und bildet die Leerlauf-Show.
- **Kill-Script** — laeuft einmal je geaenderter Ziffer, mit gesetztem `target`.

Die Entscheidung, welche Ziffer als naechstes drankommt, trifft der
C++-Zustandsautomat in der View. Das Script braucht dafuer keine Sprachmittel.

### 6.2 Steps

Ein Step ist ein einfacher Datensatz (Typ plus bis zu drei Parameter) und liegt
als `const`-Array im Flash. Steps sind entweder *setzend* — sie kehren sofort
zurueck — oder *blockierend* — der Runner wartet auf den Abschluss eines
Verhaltens.

| Step | Art | Wirkung |
|---|---|---|
| `WAIT(ms)` | blockierend | Pause |
| `PATROL_X(actor, x0, x1, speed)` | setzend | Dauerverhalten Patrouille |
| `MOVE_TO_TARGET_X(actor, speed, easing)` | blockierend | Actor faehrt unter das Ziel |
| `SET_ANIM(actor, animId)` | setzend | Sprite-Sequenz wechseln |
| `SHOW(actor, on)` | setzend | ein-/ausblenden |
| `SPAWN_AT(actor, refActor, dx, dy)` | setzend | Actor relativ zu einem anderen positionieren |
| `LAUNCH_TO(actor, x, y, speed, arc)` | setzend | ballistisches Verhalten zu einem festen Punkt starten |
| `LAUNCH_TO_TARGET(actor, speed, arc)` | setzend | wie `LAUNCH_TO`, Zielpunkt ist die Mitte der Unterkante des Ziels aus dem Register `target` |
| `AWAIT_IMPACT(actor)` | blockierend | wartet, bis der Actor das Ziel beruehrt |
| `EXPLODE_TARGET(style)` | setzend | Ziel zerfaellt in Partikel |
| `COMMIT_TARGET()` | setzend | neuen Wert uebernehmen |
| `SPAWN_TARGET(style)` | setzend | neues Ziel einblenden |
| `RETURN_TO_GROUND(actor, speed)` | blockierend | faellt zurueck auf Bodenhoehe |
| `EMIT(actor, style, count)` | setzend | Partikel an der Actor-Position |
| `END` | — | Ende des Scripts; Idle-Scripts starten neu |

### 6.3 Nachweis: derselbe Vorrat fuer beide Animationen

Kill-Script Space Invaders:

```
MOVE_TO_TARGET_X(HERO, 40, easeOut)
WAIT(120)
SPAWN_AT(SHOT, HERO, +5, -4)
SHOW(SHOT, true)
EMIT(HERO, SPARK, 4)
LAUNCH_TO_TARGET(SHOT, 90, arc = 0)
AWAIT_IMPACT(SHOT)
SHOW(SHOT, false)
EXPLODE_TARGET(BLAST)
WAIT(250)
COMMIT_TARGET()
SPAWN_TARGET(FLASH_IN)
WAIT(150)
END
```

Kill-Script Mario (noch nicht Bestandteil der Umsetzung, hier als Nachweis):

```
SET_ANIM(HERO, RUN)
MOVE_TO_TARGET_X(HERO, 45, linear)
SET_ANIM(HERO, JUMP)
LAUNCH_TO_TARGET(HERO, 70, arc = 18)
AWAIT_IMPACT(HERO)
EXPLODE_TARGET(BRICK)
COMMIT_TARGET()
SPAWN_TARGET(POP_UP)
RETURN_TO_GROUND(HERO, 70)
SET_ANIM(HERO, IDLE)
END
```

Der Unterschied liegt darin, **welcher Actor fliegt** — der Schuss oder die
Figur selbst — und ob die Bahn gerade oder ein Bogen ist. Mario braucht keinen
zusaetzlichen Step. Damit ist der Vorrat an zwei Beispielen belegt statt an
einem geraten.

### 6.4 Idle-Script Space Invaders

Die vier Gegner sind ein Actor mit einem vier Sprites breiten Frame, damit die
Gruppe mit einem `PATROL_X` marschiert und nicht vier Verhalten synchron
gehalten werden muessen.

```
SET_ANIM(FOES, MARCH)
SHOW(FOES, true)
PATROL_X(FOES, 4, 44, 10)
PATROL_X(HERO, 2, 51, 18)
WAIT(3000)
SPAWN_AT(SHOT, HERO, +5, -4)
SHOW(SHOT, true)
LAUNCH_TO(SHOT, x = aktuelle Schiffsposition, y = 32, speed = 90, arc = 0)
WAIT(600)
SHOW(SHOT, false)
WAIT(2500)
END
```

`END` startet das Idle-Script neu. Der Leerlauf-Schuss geht ins Leere. Dass der
Held im Leerlauf **Gegner** abschiesst, braeuchte ein zweites Ziel-Konzept
(`AWAIT_ACTOR_HIT`) und wird bewusst zurueckgestellt, bis sich zeigt, ob die
Show sonst zu duenn wirkt.

Weil das Schiff waehrend des Schusses weiterpatrouilliert, bekommt `LAUNCH_TO`
fuer die x-Koordinate den Sonderwert *Position des Referenz-Actors zum
Startzeitpunkt*; der Schuss fliegt also senkrecht von dort hoch, wo das Schiff
beim Abfeuern stand.

## 7. Plugin und Ablaufsteuerung

`RetroTimePlugin` leitet von `PluginWithConfig` ab. Die Zeit kommt wie im
DateTimePlugin von `ClockDrv::getInstance().getTime()` beziehungsweise
`getTzTime(m_timeZone)`.

- `process()` prueft im Sekundentakt die Zeit und reicht `HH:MM` sowie das nach
  `dateFormat` formatierte Datum an die View.
- `update(gfx)` bildet `dt` aus der millis-Differenz, tickt die Buehne und
  zeichnet in der Reihenfolge Datum, Ziffernfeld, Partikel, Actors.

### 7.1 Zustandsautomat der View

```
          active()
             |
             v
          [SYNC] ---- Ziffern hart auf aktuelle Zeit, Runner auf Idle
             |
             v
          [IDLE] <-------------------+
             |                        |
   Minutenwechsel erkannt             | Warteschlange leer
             |                        |
             v                        |
          [KILL] --- Kill-Script je Ziffer, links nach rechts
             |                        |
             +------------------------+
```

Bei einem Minutenwechsel werden alte und neue Ziffernfolge verglichen; die
Indizes der abweichenden Ziffern bilden die Warteschlange.

### 7.2 Festgelegtes Verhalten in Sonderfaellen

| Fall | Verhalten |
|---|---|
| Zeit noch nicht verfuegbar (kein NTP/RTC) | Anzeige `--:--`, Leerlauf-Show laeuft |
| Slot wird aktiv | Ziffern hart auf die aktuelle Zeit setzen, keine Animation |
| Zeitsprung durch NTP-Sync oder Zeitzonenwechsel | hart setzen statt abschiessen |
| Zeit laeuft rueckwaerts | hart setzen |
| Slot wird mitten in der Sequenz inaktiv | Sequenz verwerfen, beim naechsten `active()` hart synchronisieren |
| Partikel-Pool erschoepft | ueberzaehlige Pixel auslassen, kein Ueberlauf |

Als Zeitsprung gilt, wenn die neue Zeit nicht die unmittelbar folgende Minute
der zuletzt angezeigten ist.

### 7.3 Zeitbudget

Rund 1,3 s je Ziffer. Der laengste Fall ist 23:59 auf 00:00 mit vier geaenderten
Ziffern, also etwa 5,2 s — deutlich innerhalb einer Minute.

### 7.4 Konfiguration

Topic `/config`, wirksam ueber REST, MQTT und Web-UI:

```json
{
    "animation": "spaceInvaders",
    "dateFormat": "%d.%m.%Y",
    "timeZone": ""
}
```

- `animation`: Name einer registrierten Animation oder `"random"`. `"random"`
  waehlt bei jeder Slot-Aktivierung neu. Ein unbekannter Name faellt auf die
  erste registrierte Animation zurueck und wird geloggt.
- `dateFormat`: Format nach `strftime()`.
- `timeZone`: leer bedeutet lokale Zeit.

`AnimationRegistry` fuehrt eine statische Tabelle aus Name, Actor-Satz,
Idle-Script und Kill-Script. Eine neue Animation wird durch einen Eintrag in
dieser Tabelle bekannt gemacht.

## 8. Build-Integration

- `lib/RetroGfx/library.json` und `lib/RetroTimePlugin/library.json`.
  Abhaengigkeiten des Plugins: `Plugin`, `Logging`, `IRtc`, `Utilities`,
  `Views`, `RetroGfx`, `LittleFS`.
- `lib/RetroTimePlugin/pixelix.json` mit den Web-Dateien je Layout, Muster wie
  `lib/CountdownPlugin/pixelix.json`.
- Eintrag in `config/configNormal.ini` sowie in der neuen nativen
  64x64-Konfiguration. `src/Generated/PluginList.cpp` entsteht daraus
  automatisch und wird nicht von Hand bearbeitet.
- Flash-Budget: Die beiden 64x64-Boards haengen an unterschiedlichen
  Feature-Saetzen. `adafruit_matrixportal_s3-HUB75-64x64` nutzt `config:normal`
  mit 8 MB, `esp32doit-devkit-v1-HUB75-64x64` dagegen `config:tiny` mit 4 MB.
  Ob das Plugin auch in `configTiny.ini` aufgenommen wird, wird nach einer
  Groessenmessung entschieden und ist nicht Teil der Zusage dieses Entwurfs.
- Neue Umgebung `[env:native-64x64]` mit `[display:led_matrix_native_64x64]`
  und `[config:native64x64]`, damit die Animation im SDL-Fenster entwickelt
  werden kann.
- Dokumentation: Abschnitt in `doc/PLUGINS.md`, Web-UI-Fragment und Screenshot.

## 9. Verifikation

### 9.1 Unit-Tests

`test/test_RetroGfx/` deckt die grafikfreie Logik ab:

- `ScriptRunner`: Reihenfolge der Steps, blockierende gegen setzende Steps,
  Neustart von Idle-Scripts bei `END`, Abbruch mitten im Script.
- `Actor`: jedes Verhalten aus Abschnitt 5 inklusive Abschlussbedingung;
  Zeitschritte von 20 ms und 200 ms fuehren zur gleichen Endposition.
- `ParticleSystem`: Pool laeuft nicht ueber, Partikel verfallen, `explodeSprite`
  erzeugt hoechstens so viele Partikel wie das Sprite gesetzte Pixel hat.
- `DigitField`: Zustandsfolge je Ziffer, Erkennung der geaenderten Ziffern.
- Sonderfaelle aus Abschnitt 7.2, soweit ohne Grafik pruefbar.

Ausgefuehrt mit `platformio test --environment test`.

### 9.2 Visuelle Pruefung

In der nativen 64x64-Umgebung. Damit nicht je Iteration eine Minute auf die
Animation gewartet werden muss, gibt es den Compile-Switch
`CONFIG_RETROTIME_FAST_DEMO`, der nur im nativen Build gesetzt wird und die
angezeigte Uhrzeit alle paar Sekunden um eine Minute weiterstellt.

### 9.3 Pflichtlaeufe vor Abschluss

- `clang-format --dry-run --Werror` ueber alle neuen Dateien
- `platformio check --fail-on-defect=medium`
- Build `esp32doit-devkit-v1-HUB75-64x64`
- Build eines 32x8-Boards, damit der Generic-Fallback nachweislich uebersetzt
- `platformio test --environment test`
- Doxygen ohne neue Warnungen

## 10. Offene Punkte

- Aufnahme in `configTiny.ini` haengt an der Groessenmessung (Abschnitt 8).
- Die genaue Gestaltung der Ziffern-Sprites entsteht waehrend der Umsetzung an
  der nativen Simulation.
- Ob die Leerlauf-Show um `AWAIT_ACTOR_HIT` erweitert wird, entscheidet sich
  nach dem ersten sichtbaren Ergebnis.
