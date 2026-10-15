# Structure Diagrams

Class, object, component, deployment and use case diagrams. All wrapped in
`@startuml` / `@enduml`.

This file covers the syntax. For the UML vocabulary each diagram kind may use
and the constraints PlantUML will happily let you break, read
[uml-conformance.md](uml-conformance.md).

## Class diagrams

### Declaring elements

`class`, `interface`, `abstract class`, `enum`, `entity`, `annotation`.

`entity` and `annotation` are PlantUML shapes, not UML classifiers. In a UML
diagram use `class` with a stereotype from a named profile instead.

```plantuml
class Account
interface Repository
abstract class Shape
enum Status
class "Order Line Item" as LineItem
```

Quote any name containing spaces and bind a short alias with `as`; reference the
alias afterwards.

### Relationships

| Relationship | Symbol | Meaning |
| ------------ | ------ | ------- |
| Inheritance | `<\|--` | a subclass specialises a superclass |
| Implementation | `<\|..` | a class realises an interface |
| Composition | `*--` | the part cannot exist without the whole |
| Aggregation | `o--` | the part can exist independently |
| Dependency | `-->` | one element uses another |
| Weak dependency | `..>` | looser dependency, drawn dotted |

Replacing `--` with `..` anywhere turns a solid line dotted.

```plantuml
Animal <|-- Dog
Repository <|.. SqlRepository
Car *-- Engine
Library o-- Book
Controller --> Service
Service ..> Logger
```

### Labels and multiplicity

Label with `:`. Multiplicities go in quotes on either side. A `<` or `>` in the
label draws a reading-direction arrow.

```plantuml
Order "1" *-- "many" LineItem : contains
Customer "1" --> "0..*" Order : places >
```

### Members

`ClassName : member` adds one member; a `{ }` body lists several. PlantUML
decides field vs. method by the presence of parentheses.

```plantuml
class Flight {
    flightNumber : Integer
    board()
    cancel()
}
```

Visibility prefixes: `-` private, `#` protected, `~` package private, `+` public.
Hide the rendered icons with `skinparam classAttributeIconSize 0`.

Modifiers `{abstract}` and `{static}` may sit at either end of the member line.

Separators `--`, `..`, `==`, `__` divide a class body into sections and may carry
a title: `.. identity ..`.

### Notes and stereotypes

```plantuml
class Object <<general>>
note top of Object : The root of the hierarchy.
note "A floating note" as N1
Object .. N1
```

## Object diagrams

A snapshot of concrete instances. Same relation symbols as class diagrams.

```plantuml
object firstObject
object "My Second Object" as o2
object user
user : name = "Dummy"
user : id = 123
Customer *-- "1..*" Order
```

A `diamond` node models an n-ary association:

```plantuml
object o1
object o2
diamond dia
o1 --> dia
o2 --> dia
dia --> o3
```

## Component diagrams

Components are `[Name]` or the `component` keyword; interfaces are `()` or the
`interface` keyword. Both are created implicitly on first reference.

```plantuml
[First component]
[Another component] as Comp2
component Comp3
() "First Interface"
interface Interf3
```

Connect with `..` (dotted), `--` (solid) and `-->` (dependency). A solid line
usually wires a component to an interface it provides.

```plantuml
DataAccess - [First Component]
[First Component] ..> HTTP : use
```

### Grouping

`package`, `node`, `folder`, `frame`, `cloud`, `database` — all nest freely.

```plantuml
package "Web Tier" {
  HTTP - [Frontend]
}
node "App Tier" {
  [Frontend] --> [Service]
}
```

### Other component features

| Feature | Syntax |
| ------- | ------ |
| Horizontal link | single dash: `[A] - [B]` |
| Steer direction | `[A] -left-> B`, also `right`, `up`, `down` |
| Plain rectangles | `skinparam componentStyle rectangle` |
| Background colour | `component [Web Server] #Yellow` |
| Drop unconnected | `hide @unlinked` or `remove @unlinked` |
| Boundary ports | `port p1`, `portin`, `portout` inside a component body |

## Deployment diagrams

A superset of the component notation, for runtime topology.

Shapes: `node`, `artifact`, `database`, `cloud`, `folder`, `frame`, `component`,
`queue`, `stack`, `storage`.

```plantuml
node Server
artifact app.jar
database Postgres
cloud Internet
queue MessageBus
```

Short forms: `[name]` component, `(name)` use case, `()` `"name"` interface,
`:name:` actor.

Line types: `--` plain, `..` dashed, `~~` dotted, `==` bold. Arrow heads
include `-->`, `--*`, `--o`, `--|>` and the socket `--(0`.

Bracketed link style sets line properties:

```plantuml
foo -[#red]-> bar1 : red
foo -[dashed]-> bar2 : dashed
foo -[#blue,thickness=4]-> bar3 : thick blue
```

## Use case diagrams

Use cases are `(Name)` or the `usecase` keyword; actors are `:Name:` or the
`actor` keyword. Any undefined name at the far end of an arrow becomes an actor.

```plantuml
left to right direction
actor "Food Critic" as fc
rectangle Restaurant {
  usecase "Eat Food" as UC1
  usecase "Pay for Food" as UC2
}
fc --> UC1
fc --> UC2
```

- `rectangle` around use cases is the usual way to draw the system boundary.
- `<|--` shows generalisation between actors or between use cases.
- A trailing `/` marks a **business** actor or use case: `actor/ Manager`.
- `left to right direction` almost always lays these out better.
- Inline style: `actor b #pink;line:red;line.bold;text:red`.
