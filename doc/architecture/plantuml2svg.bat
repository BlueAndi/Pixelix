@echo off

REM This batch file converts all PlantUML files in the ./uml directory to ./svg/ directory as .svg files using PlantUML.
REM If PlantUML is not installed, it will be downloaded and used from the local directory.

REM Check if PlantUML is installed, if not, download it
if not exist plantuml.jar (
    echo PlantUML not found. Downloading...
    powershell -Command "Invoke-WebRequest -Uri https://github.com/plantuml/plantuml/releases/download/v1.2026.2/plantuml-mit-1.2026.2.jar -OutFile plantuml.jar"
)

REM Create the output directory if it doesn't exist
if not exist svg (
    mkdir svg
)

REM Convert all .puml, .wsd, .plantuml files in the uml directory to .svg files in the svg directory
for %%f in (uml\*.puml uml\*.wsd uml\*.plantuml) do (
    echo Converting %%f to SVG...
    java -jar plantuml.jar -tsvg "%%f" -o ..\svg
)

echo Conversion complete. SVG files are located in the svg directory.
