# Styling and Text Markup

`skinparam`, `<style>` blocks, inline colour, Creole text markup and sprites.

## skinparam

One parameter per line, anywhere in the diagram. Values are colour names or RGB
codes. `skinparam` is gradually being superseded by `<style>`, but remains fully
supported and is what most existing diagrams use.

```plantuml
skinparam backgroundColor transparent
skinparam shadowing false
skinparam monochrome true
```

### Grouping by prefix

Parameters sharing a prefix can be collected in a block — exactly equivalent to
writing each on its own line.

```plantuml
skinparam sequence {
  ArrowColor DeepSkyBlue
  LifeLineBorderColor blue
  LifeLineBackgroundColor #A9DCDF
}
```

### Fonts

Each element family exposes `FontColor`, `FontSize` and `FontName`;
`defaultFontName` sets one font for the whole diagram. Font names are system
dependent — prefer widely available ones if the diagram must render everywhere.

```plantuml
skinparam classFontColor #CC0000
skinparam classFontSize 14
```

### Scoping to a stereotype

Append `<<stereotype>>` so a parameter applies only to elements carrying it.

```plantuml
skinparam component {
  BackgroundColor<<core>> Gold
  BackgroundColor<<external>> LightCoral
}
[Engine] <<core>>
[Payment API] <<external>>
```

### Frequently used parameters

| Parameter | Effect |
| --------- | ------ |
| `backgroundColor` | diagram background; `transparent` is valid |
| `shadowing false` | flat shapes, no drop shadow |
| `monochrome true` | strip all colour, for print |
| `linetype ortho` \| `polyline` | edge routing style |
| `nodesep` / `ranksep` | spacing between and across ranks |
| `componentStyle rectangle` | plain rectangles instead of UML component icons |
| `classAttributeIconSize 0` | show raw visibility symbols instead of icons |
| `sequenceMessageAlign` | `left`, `right`, `center`, `direction` |
| `packageStyle rectangle` | package drawn as a plain rectangle |
| `titleBorderThickness`, `titleBorderColor`, `titleBackgroundColor`, `titleBorderRoundCorner` | the title box |

### Discovering parameters

There are far more than any page can list. Render a diagram whose only content
is the `skinparameters` keyword, or run `java -jar plantuml.jar -language`.

## Style blocks

`<style>` gives finer control than individual skinparams. Each element takes its
own properties. Prefer this for visual styling in new work.

A `<style>` block silently ignores properties it does not recognise — no error is
raised. Layout parameters are **not** available here: `nodesep`, `ranksep`,
`packageStyle` and `linetype` only work as `skinparam`. Verified on PlantUML
1.2026.6: `skinparam nodesep 10` vs `200` changes the rendered width, while
`<style> root { NodeSep 10 }` vs `200` produces byte-identical output.

```plantuml
<style>
title {
  FontSize 22
  FontColor blue
}
legend {
  FontSize 16
  BackGroundColor yellow
}
</style>
```

## Inline element styling

Most elements accept a colour, and many accept a full inline style, directly
after the declaration.

```plantuml
component [Web Server] #Yellow
actor b #pink;line:red;line.bold;text:red
```

```plantuml
state Healthy #palegreen
state Degraded #gold ##[dashed]orange
Healthy --> Degraded : warning
```

Properties: a `#fill` colour, `line:<colour>`, `line.bold` / `line.dashed` /
`line.dotted`, and `text:<colour>`. Borders use the `##[style]colour` form.

## Creole text markup

Creole formats text **inside** diagram elements — notes, class bodies, labels,
titles and legends. A subset of HTML-like tags is also accepted.

### Emphasis

| Markup | Result |
| ------ | ------ |
| `**bold**` | bold |
| `//italics//` | italic |
| `""monospaced""` | monospaced |
| `--stricken--` | struck out |
| `__underlined__` | underlined |
| `~~wave-underlined~~` | wave underline |

Escape a marker with a leading tilde to print it literally: `~__not underlined__`.

### Lists

`*` starts a bulleted item, `#` a numbered one. Repeat the marker to nest:
`**` and `##` go one level deeper.

### Headings and separators

Leading `=` signs make headings — `=` largest through `====` smallest.

Separators inside an element: `----` plain line, `====` double, `____` strong,
and `..text..` a dotted line carrying a title.

```plantuml
note as N
This is **bold** and //italic//
----
* Bullet list
** Sub item
end note
```

## Sprites

A sprite is a small monochrome icon usable anywhere text appears. Define it from
a hex pixel grid (one digit per pixel, gray level `0`–`F`) and reference it with
`<$name>`.

```plantuml
sprite $foo1 {
FFFFFFFFFFFFFFF
F0123456789ABCF
FFFFFFFFFFFFFFF
}
Alice -> Bob : Testing <$foo1>
```

Sprites can be scaled and coloured at the point of use, and imported from
outside sources.

## Standard library

PlantUML bundles icon and model libraries — AWS, Azure, Kubernetes, the C4 model
and more — included by angle-bracket path:

```plantuml
!include <aws/common>
!include <aws/Storage/AmazonS3/AmazonS3>
AMAZONS3(s3_internal)
```

These depend on the library files being available to the renderer, which is not
guaranteed on every server. If a diagram must render anywhere, build it from
plain elements instead.
