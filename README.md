# MaSimViz - Malaria Simulation Visualization

**MaSimViz** is a cross-platform and powerful tool designed to visualize the output of Malaria simulations, providing the ability to playback event logs and display various simulation data. It automatically parses input files, displays raster maps, and supports interactive plots of simulation data for deeper analysis.

![](https://github.com/KienTTran/MaSimViz/blob/master/ABMGPU.gif)

## Features

1. **Auto Load and Parse Simulation Data**: 
   - Automatically loads input files, raster maps, and SQLite databases from the simulation folder.
   
2. **Input Raster Maps Display**: 
   - Visualizes all input raster maps used in the simulation, aiding geographical insights.

3. **Table and Column Selection**:
   - Allows users to select specific tables and columns from the database for visualization.
   
4. **IQR Calculation**:
   - Calculates and caches the Interquartile Range (IQR) data from all simulations for the selected columns.

5. **Location-based Plots**:
   - Plots data from different locations, helping users to compare and analyze the results spatially.

6. **Pan and Zoom Functionality**:
   - Interactive pan (right mouse button) and zoom (mouse wheel) support for detailed exploration of plots.

7. **Support for District and Pixel Reporters**:
   - Visualizes district-level and pixel-level data from the simulation.

## Installation

Clone the repository and follow the setup instructions provided in the project.

```bash
git clone https://github.com/your-repo/masimviz.git
cd masimviz
./vcpkg install
mkdir build && cd build
cmake ../ && make
./masimviz
```

or

Using QtCreator to open the project for easier development.

