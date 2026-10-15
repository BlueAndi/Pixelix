---
name: plantuml-reference
description: Reference for the PlantUML diagram language and for producing valid UML 2.5.1 with it. Use when writing, fixing or reviewing PlantUML source — a `.puml` file, an `@startuml` block in C/C++ source or Markdown, a PlantUML class/sequence/activity/state/component/deployment/use-case/object/timing diagram, `skinparam` or `<style>` styling, Creole text markup, a PlantUML diagram that renders wrongly or not at all, or a request for a UML-conformant diagram in PlantUML. Not for other diagram languages such as Mermaid, Graphviz DOT or D2.
---

Use this skill whenever **PlantUML** diagram source is written or changed — a
`.puml` file, or an `@startuml`/`@startgantt`/... block embedded in source code,
Markdown or reStructuredText. It does not cover other diagram languages such as
Mermaid, Graphviz DOT or D2. Follow these rules, then open the reference file for
the diagram type at hand.

## Rules

1. Diagrams are **UML 2.5.1** unless the user asked for one of the non-UML types
   (Gantt, MindMap, WBS, Salt, nwdiag, JSON, YAML, ArchiMate). Read
   [references/uml-conformance.md](references/uml-conformance.md) before writing
   one: PlantUML checks syntax only, so a diagram that renders perfectly can
   still be invalid UML.
2. Wrap every diagram in `@startuml` / `@enduml`, except the types that carry
   their own markers (see the table below). A missing or mismatched marker is the
   most common reason a diagram does not render.
3. Write one diagram per file or per fenced block. Give it a `title` when it is
   published in a document.
4. Prefer the **current** activity syntax (`:action;`), never the legacy `(*)`
   notation, for new diagrams.
5. Alias every element whose label contains spaces: `class "Order Line" as Line`.
   Then reference the alias, not the quoted label.
6. Reach for layout control only after the content is right — `left to right
   direction`, then direction hints inside arrows (`-up->`, `-left->`). Neither
   applies to sequence, activity or timing diagrams: `left to right direction`
   is a syntax error there. Set `skinparam linetype ortho` only if it actually
   improves the result; it often makes edge routing worse.
7. Keep styling minimal and at the top of the diagram. Use `<style>` blocks for
   visual styling (colours, fonts) in new work; `skinparam` is still supported but
   is being superseded. Layout parameters have **no** `<style>` equivalent and must
   stay `skinparam` — `nodesep`, `ranksep`, `packageStyle`, `linetype`. A `<style>`
   block silently ignores properties it does not know, so an unsupported setting
   fails without any error.
8. Verify the diagram renders before claiming it is done. Render it with the
   PlantUML jar (`java -jar plantuml.jar -tpng diagram.puml`) or a PlantUML
   server, and look at the result — a file that parses can still be unreadable.

## Diagram markers

Most diagram types use `@startuml`. These do not — and none of them is a UML
diagram, so none may be delivered where UML is required:

| Type | Markers | UML? |
| ---- | ------- | ---- |
| JSON data | `@startjson` / `@endjson` | no |
| YAML data | `@startyaml` / `@endyaml` | no |
| Gantt chart | `@startgantt` / `@endgantt` | no |
| MindMap | `@startmindmap` / `@endmindmap` | no |
| Work breakdown structure | `@startwbs` / `@endwbs` | no |
| Salt wireframe | `@startsalt` / `@endsalt`, or `@startuml` + a `salt` line | no |
| Network diagram | `@startnwdiag` / `@endnwdiag` | no |
| Standalone maths | `@startmath` / `@endmath`, `@startlatex` / `@endlatex` | no |

`@startuml` does not by itself make a diagram UML: ArchiMate, entity-relationship
crow's-foot notation and PlantUML's decorative shapes all live inside it. See
section 14 of [references/uml-conformance.md](references/uml-conformance.md).

## Commands that work in every diagram

| Purpose | Syntax |
| ------- | ------ |
| Line comment | `' comment` |
| Block comment | `/' ... '/` |
| Heading above the diagram | `title text`, or `title` ... `end title` |
| Caption below the diagram | `caption text` |
| Top / bottom stamp | `header text`, `footer text` (prefix `left`/`center`/`right`) |
| Explanatory box | `legend` ... `endlegend` (accepts `top left`, `right`, ...) |
| Outer UML frame | `mainframe text` |
| Resize | `scale 1.5`, `scale 200 width`, `scale max 800*600` |
| Hide empty compartments | `hide empty members` |

`left to right direction` is **not** one of them: it is a syntax error in
sequence, activity and timing diagrams, whose layout is fixed by the notation.
It works in class, object, package, component, deployment, use case and state
machine diagrams.

## Reference files

Read the one that matches the task; each covers syntax, relationships and
annotation for its diagram types.

| File | Covers |
| ---- | ------ |
| [references/uml-conformance.md](references/uml-conformance.md) | OMG UML 2.5.1 — diagram kinds, legal vocabulary and the constraints PlantUML will let you break |
| [references/structure-diagrams.md](references/structure-diagrams.md) | class, object, component, deployment, use case |
| [references/behaviour-diagrams.md](references/behaviour-diagrams.md) | sequence, activity, state, timing |
| [references/styling.md](references/styling.md) | `skinparam`, `<style>`, colors, Creole text markup, sprites |
| [references/preprocessing.md](references/preprocessing.md) | variables, conditions, loops, procedures, functions, `!include` |
| [references/other-diagrams.md](references/other-diagrams.md) | JSON, YAML, Gantt, MindMap, WBS, Salt, nwdiag, ArchiMate, entity-relationship, maths — **none of them UML** |

## Before claiming a diagram is done

- The diagram kind matches its primary symbols, and is one of the fourteen in
  section 1 of [references/uml-conformance.md](references/uml-conformance.md).
- Only UML-defined elements and relationships are used — nothing from section 14
  of that file.
- The constraint table for this diagram kind has been re-read against the
  diagram. Arrow directions are the usual casualty: «include», «extend»,
  «deploy» and «merge».
- It renders, and the result is readable.

## Embedding diagrams

- **Markdown** — a fenced block tagged `plantuml` containing the full
  `@startuml` ... `@enduml` source. GitLab renders these natively when its
  PlantUML integration is enabled.
- **C/C++ source** — inside a Doxygen comment block; see the
  `sw-detailed-design-doxygen` skill for where the diagrams belong in an SDD.
- **Sphinx / reStructuredText** — via the `sphinxcontrib-plantuml` extension or
  a project-local directive. Point it at a PlantUML server with the
  `PLANTUML_SERVER_URL` environment variable rather than hard-coding a URL.
- **Rendering locally** — projects here fetch the jar with a
  `tools/plantUML/get_plantuml.[sh|bat]` helper; call it before rendering in a
  script.

## Getting the full language documentation

This skill is a condensed reference. For the complete language, including the
diagram types not summarised here, consult the official documentation at
<https://plantuml.com/> or the self-hosted PlantUML documentation published by
the DevOps chapter.

The UML rules are distilled from **OMG Unified Modeling Language 2.5.1**
(formal/2017-12-05), <https://www.omg.org/spec/UML/2.5.1/>. Clause numbers cited
in `uml-conformance.md` refer to that document.
