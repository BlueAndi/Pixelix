# PlantUML Diagram Types — Syntax Reference

All PlantUML source files start with `@startuml <name>` and end with `@enduml`.  
Use `'` for single-line comments and `/' ... '/` for block comments.

---

## Component Diagram

Shows hardware/software component structure and interfaces.

```plantuml
@startuml my-components

title My System

component "Frontend" as fe
component "Backend" as be
database "Database" as db

interface "REST API" as iRest
interface "SQL" as iSql

fe --( iRest
iRest -- be
be --( iSql
iSql -- db

@enduml
```

**Grouping:**
```plantuml
package "Subsystem" {
    component "Part A" as a
    component "Part B" as b
    a --> b
}
```

**Interface styles:** `--( ` (lollipop), `--` (plain line), `-->` (arrow)

---

## Sequence Diagram

Shows interactions over time between participants.

```plantuml
@startuml my-sequence

autoactivate on

participant Client
participant Server
entity Database

Client -> Server: HTTP GET /api/data
    Server -> Database: SELECT *
    Server <-- Database: rows
Client <-- Server: 200 OK { data }

note over Client,Server: Optional note

loop Retry on error
    Client -> Server: retry
    Client <-- Server: response
end

@enduml
```

**Arrow types:**

| Syntax | Description          |
| ------ | -------------------- |
| `->`   | Solid arrow          |
| `-->`  | Dashed arrow         |
| `->>`  | Thin solid           |
| `-->>` | Thin dashed          |
| `-x`   | Lost message (cross) |
| `->o`  | Open arrow           |

**Blocks:** `loop`, `alt`/`else`, `opt`, `par`, `critical`, `break`, `group`

**Lifeline control:** `activate Foo` / `deactivate Foo` or `autoactivate on`

---

## Class Diagram

Shows class attributes, methods, and relationships.

```plantuml
@startuml my-classes

abstract class Animal {
    +String name
    +int age
    +{abstract} speak() : void
}

class Dog {
    +fetch() : void
}

class Bone {
    +material : String
}

Animal <|-- Dog : extends
Dog "1" o-- "*" Bone : owns

@enduml
```

**Relationship arrows:**

| Syntax  | Meaning                      |
| ------- | ---------------------------- |
| `<\|--` | Inheritance (extension)      |
| `*--`   | Composition                  |
| `o--`   | Aggregation                  |
| `-->`   | Association                  |
| `..>`   | Dependency                   |
| `..\|>` | Realization / implementation |

**Visibility:** `+` public, `-` private, `#` protected, `~` package

**Modifiers:** `{abstract}`, `{static}`, `{field}`, `{method}`

---

## State Diagram

Shows states and transitions for a state machine.

```plantuml
@startuml my-states

[*] --> Idle

Idle --> Running : start
Running --> Idle : stop
Running --> Error : fault
Error --> Idle : reset
Idle --> [*]

state Running {
    [*] --> Fetching
    Fetching --> Processing
    Processing --> [*]
}

@enduml
```

**Special states:** `[*]` (initial/final), `<<choice>>`, `<<fork>>`, `<<join>>`, `<<end>>`

**Notes:** `note right of StateName : text`

---

## Activity Diagram

Shows a process, algorithm, or decision flow.

```plantuml
@startuml my-activity

start

:Read configuration;

if (Config valid?) then (yes)
    :Initialize hardware;
    :Start main loop;
else (no)
    :Log error;
    stop
endif

repeat
    :Process event;
repeat while (more events?) is (yes)
->no;

stop

@enduml
```

**Swim lanes (partitions):**
```plantuml
|Actor A|
start
:Step 1;
|Actor B|
:Step 2;
stop
```

---

## Deployment Diagram

Shows deployment on hardware nodes and tasks/processes.

```plantuml
@startuml my-deployment

node "ESP32" {
    [loopTask] <<APP_CPU>>
    [processTask] <<APP_CPU>>
    [updateTask] <<no-affinity>>
    [wifi] <<PRO_CPU>>
}

node "External" {
    [MQTT Broker]
    [Web Browser]
}

[loopTask] --> [MQTT Broker] : MQTT
[Web Browser] --> [loopTask] : HTTP

@enduml
```

---

## Mindmap

Shows hierarchical topic/concept structure.

```plantuml
@startuml my-mindmap

@startmindmap
* Root Topic
** Branch A
*** Leaf A1
*** Leaf A2
** Branch B
*** Leaf B1
@endmindmap

@enduml
```

> Note: mindmaps use `@startmindmap` / `@endmindmap` instead of `@startuml` / `@enduml`.

---

## Entity-Relationship (ER) Diagram

Shows database table and entity relationships.

```plantuml
@startuml my-er

entity "Customer" as customer {
    *id : INTEGER <<PK>>
    --
    name : VARCHAR
    email : VARCHAR
}

entity "Order" as order {
    *id : INTEGER <<PK>>
    --
    created : DATE
    *customer_id <<FK>>
}

customer ||--o{ order : places

@enduml
```

**Cardinality:** `||` exactly one, `|o` zero or one, `o{` zero or many, `|{` one or many

---

## Skinparam Cheat Sheet

Apply at the top of the file to control visual style:

```plantuml
skinparam componentStyle rectangle
skinparam monochrome true
skinparam shadowing false
skinparam defaultFontName Arial
skinparam ArrowColor #444444
skinparam PackageBorderColor #888888
```

---

## Common Tips

- Alias long names: `component "Long Component Name" as shortAlias`
- Force layout direction: `left to right direction` (top-level) or `rankdir=LR`
- Add notes: `note left of Foo : text` or `note as N1 ... end note`
- Use `together { A \n B }` to force elements to be on the same rank
- Avoid special characters in IDs; use `as` for display labels with spaces or punctuation
- Split large diagrams: keep each `.wsd` file focused on one subsystem or interaction
