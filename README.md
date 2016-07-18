# SFML Template for Windows, Mac, and Linux

### Requirements
* SFML files for your OS. Found [here](http://www.sfml-dev.org/download.php)

* CMake

#### Clone the Repo

```git clone https://github.com/MitchellHansen/SFML_CMake_Template```

### Windows

* Open the CMake GUI and under "Where is the source code" enter the path that you cloned the repo into. For example ```C:/Users/mrh/Desktop/SFML_CMake_Template```

* Under "Where to build the binaries" enter the destination you want you project to build to. Traditionally this is placed in /build. For example ```C:/Users/mrh/Desktop/SFML_CMake_Template/build```

* Hit the configure button and choose from the dropdown box your generator. As this is on Windows I'll assume you're using Visual Studio, select your version and the platform you want to build for. ```Visual Studio VV YYYY``` (32 bit) or ```Visual Studio VV YYYY 64``` (64 bit). Press Finish when done.

* If at any time you want to re-enter this infomation, hit file -> delete cache and then press configure again.

* CMake will now complain about an error in the configuration process as it can't find where you downloaded SFML to. I usually place libs in a seperate drive, so I would replace ```root``` in the ```SFML_ROOT``` row with ```Z:/cpp_libs/SFML-2.3.2-windows-vc14-32-bit```

* Press Generate and CMake will now generate and place all the files required by Visual Studio in your ```build``` directory

* Open up this directory in your file explorer and open  ```Game.sln```

* In your solution explorer set Game as the starup project and hit F5 to run the project!


